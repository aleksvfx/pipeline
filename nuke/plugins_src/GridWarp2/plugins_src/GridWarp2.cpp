/* GrideWarp2.cpp
 *
 * Copyright (c) 2008 The Foundry Visionmongers Ltd.  All Rights Reserved.
 * 
 * Merged from Digital Domain 11/11/08
 * 
 */

/*
 ** Mods 03-19-2008 by Raymond Yeung
 ** - src/dst has independent transforms
 ** - each transform is loaded to transform knob when selected (src default)
 ** - support to/from_script like SplineWarp2
 ** - use transform_from_script/extract_bracket_express a la SplineWarp2
 ** - HACK: reading from script synchronization (current version 4)
 ** - HACK: reading older version script (version <= 3)
 **         transform knob read after grid knobs read so that the code relies
 **         on the specific way Nuke calls knobs twice, once before and once
 **         after script is read to set up grid transforms
 **   !!!!! this will not work when Nuke's start up from script behavior is
 **   !!!!! changed
 ** - hack: best to re-save to version 4
 */

/*
 ** GridWarp provides two rectangular grids that can be deformed.
 **
 ** Still To Do:
 ** - CRASH: undo crashes on many different occasions
 ** - CRASH: make sure that we can't paste if there's nothing in the copy buffer
 ** - BUG: 'Render' does not work correctly in proxy mode
 ** - BUG: Transform jack scale and drag-center do not work in proxy mode
 ** - FEAT: RemoveCurve must recalculate the neighbor interpolations using the 
 **         removed curve!
 ** - FEAT: distortion rate and continuity
 ** - FEAT: more documentation
 ** - FEAT: distortion amount slider
 */

#include <DDImage/Render.h>
#include <DDImage/Knobs.h>
#include <DDImage/Knob.h>
#include <DDImage/Row.h>
#if DD_IMAGE_VERSION_MAJOR < 5
#include <DDImage/math.h>
#else
#if (DD_IMAGE_VERSION_MAJOR == 5 && DD_IMAGE_VERSION_MINOR < 1)
#include <DDImage/math.h>
#else
#include <DDImage/DDMath.h>
#endif
#endif
#include <DDImage/ViewerContext.h>
#include <DDImage/Thread.h>
#include <DDImage/LUT.h>

#ifdef _WIN32
#include <windows.h>
#ifdef min
#undef min
#endif
#ifdef max
#undef max
#endif
#endif
#include <DDImage/gl.h>


#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <map>
#include <iostream>
#include <sstream>
#include <string>


#include "TransformUtils.h"

static char *HELP = 
"@i;GridWarp@n; allows for transfering image information from on bezier grid "
"(source) onto another grid (destination).\n\n"
"Grids are deformed by selecting one "
"or more points which can be dragged around arbitrarily. Each point comes with "
"a number of handles that modify the curve that is connecting the points. Both "
"grids can be independently animated using key shapes.";

using namespace DD::Image;
using namespace std;

//==== classes implemented in this file =======================================

namespace DD {
    namespace GridWarp {
        struct MPoint;
        class MGrid;
        class MTri;
        class MRect;
        class MTriList;
        class Grid_Knob;
        class WarpIop;
    }
};

// Increment this to prevent compatibility issues when switching between
// old and new GridWarps:
#define GRID_VERSION   4
// marking for the last version for use by to/from_script
#define GRID_VERSION_3 3

//==== statics ================================================================

static const char* const CLASS = "GridWarp2";
static const char* const CLASSM = "GridWarpMath2";

// return the determinant of two coordinates
static float det(float ux, float uy, float vx, float vy) {
    return ux*vy-uy*vx;
}

// return a coordinate of a bezier curve at a certain fraction in the curve.
// this works for the x and for the y coordinate alike.
static float bezier_at(float t, float x0, float x1, float x2, float x3) {
    x1 += x0; x2 += x3;
    return x0 + 3*t*(x1-x0) + 3*t*t*(x0+x2-2*x1) + t*t*t*(x3-x0+3*x1-3*x2);
}

// gang two handles in rotation, but not in length
static void rot_gang(float x, float y, float &ox, float &oy) {
    float len = sqrtf(ox*ox+oy*oy);
    float gamma = atan2f(y, x);
    oy = sinf(gamma)*len;
    ox = cosf(gamma)*len;
}

// silly string reading shortcuts:
// skip white space
static void sws(const char*&s) { for (;*s;s++) { if (!isspace(*s)) break; } }
// skip non-white space
static void snws(const char*&s) { for (;*s;s++) { if (isspace(*s)) break; } }
// read and skip a float
static double getf(const char*&s) { sws(s); if (!*s) return 0; 
    double v = atof(s); snws(s); return v;
}
// read and skip an int
static int geti(const char*&s) { sws(s); if (!*s) return 0; 
    int v = atoi(s); snws(s); return v;
}

#ifdef DEBUG
#define DEBUG_TEST_KNOBS
#endif

namespace {

bool extract_bracket_expression( char const*& text, std::string& expression )
{
    if ( !text ) {
        expression = "";
        return true;
    }

    sws( text );
    if ( *text != '{' ) {
        std::cerr << "Unable to extract the next bracket expression from: " << text << std::endl;
        return false;
    }

    int brackets = 1;
    char const* text_end = text + 1;

    // find matching bracket skipping internal bracket sub-expressions
    while ( *text_end && brackets ) {

        char current = *text_end;

        if ( current == '{' ) {
            brackets++;
        } else if ( current == '}' ) {
            brackets--;
        }

        text_end++;

    }

    if ( !*text_end ) {
        std::cerr << "Incomplete bracket expression from: " << text << std::endl;
        return false;
    }

    int length = text_end - text;
    expression.assign( text, length );
    text = text_end;

    return true;
}


bool transform_from_script( char const*& text, GridTransform& transform )
{
    std::string expression;
    bool ok = extract_bracket_expression( text, expression );
    if ( !ok ) {
        return false;
    }
    return transform.from_script( expression );
}


} // namespace



namespace DD { namespace GridWarp {

//==== MPoint =================================================================

/** The MPoint class describes a manipulatable point (or node) in the grid.
 * Each point has its coordinates and four more handles (coordinates stored 
 * relative to point) that describe the bezier curves that creat the mesh.
 */
struct MPoint {
    float x, y; // point position
    float nx, ny, sx, sy, wx, wy, ex, ey; // relative handle coordinates

    // intialize point to some remotely useful settings
    void init(int ix, int iy, float xscale, float yscale, float xoff, float yoff) {
        x = xscale * ix + xoff;
        y = yscale * iy + yoff;
        nx = sx = wy = ey = 0.0;
        ny =  yscale/3.0;
        ex =  xscale/3.0;
        sy = -yscale/3.0;
        wx = -xscale/3.0;
    }

    void transform(const Matrix4& m) {
        MPoint p = *this;
        x = m.a00*p.x + m.a01*p.y + m.a03;
        y = m.a10*p.x + m.a11*p.y + m.a13;
        ex = m.a00*p.ex + m.a01*p.ey;
        ey = m.a10*p.ex + m.a11*p.ey;
        wx = m.a00*p.wx + m.a01*p.wy;
        wy = m.a10*p.wx + m.a11*p.wy;
        nx = m.a00*p.nx + m.a01*p.ny;
        ny = m.a10*p.nx + m.a11*p.ny;
        sx = m.a00*p.sx + m.a01*p.sy;
        sy = m.a10*p.sx + m.a11*p.sy;
    }
};

//==== MGrid ==================================================================

static float scalarLerp( float t, float a, float b ) {
  return a+t*(b-a);
}

/** The MGrid holds a number of MPoints to describe the Bezier mesh
 */
class MGrid {
    public:
        std::vector<MPoint> points;
        int width, height;
        unsigned int color;

        MGrid() : width(0), height(0), color(0xcccccc00)
            { points.reserve(20); }

        int     num_points() const { return points.size(); }
        MPoint* array() { return &(points[0]); }
        void    fitRect( int x, int y, int r, int t );
        void    init( int w, int h, float scale=50.0f, float xoff=100.0f,
                float yoff=100.0f);
        MGrid  *dup() const;
        void    lerp( MGrid *a, MGrid *b, float f );
        void    resize( int w, int h );
        void    copy_to( MGrid *d ) const;
        MPoint &at( int x, int y );
        void    getXY(int n, int& x, int& y) const;
        void    remove_curve( int ix );
        void    add_curve( int ix, float mul );

        void getbox(int& x,int& y,int& r,int& t) const;

        void transform(const Matrix4& m) {
            const unsigned n = points.size();
            for (unsigned i=0; i < n; i++)
                points[i].transform(m);
        }

//        void from_script(const Script_List& list);
        void to_script(std::ostream& o) const;
};

}} // namespace DD::GridWarp

using namespace DD::GridWarp;

void MGrid::fitRect(int x, int y, int r, int t)
{
    float xs = (r-x)/float(width-1.0f), ys = (t-y)/float(height-1.0);
    MPoint *dst = array();
    for (int iy=0; iy<height; iy++) 
        for (int ix=0; ix<width; ix++) 
            (dst++)->init(ix, iy, xs, ys, x, y);
}

void MGrid::init(int w, int h, float scale, float xoff, float yoff )
{
    resize(w, h);
    MPoint *dst = array();
    for (int y=0; y<height; y++) 
        for (int x=0; x<width; x++) 
            (dst++)->init(x, y, scale, scale, xoff, yoff);
}

void MGrid::getbox(int& ix, int& iy, int& ir, int& it) const {
    const unsigned n = points.size();
    if (!n) return;
    float x,y,r,t;
    const MPoint* p = &(points[0]);
    x = r = p->x;
    y = t = p->y;
    for (unsigned i=0; i < n; i++) {
        float v;
        v = p->x; x = MIN(x,v); r = MAX(r,v);
        v = p->x+p->nx; x = MIN(x,v); r = MAX(r,v);
        v = p->x+p->sx; x = MIN(x,v); r = MAX(r,v);
        v = p->x+p->wx; x = MIN(x,v); r = MAX(r,v);
        v = p->x+p->ex; x = MIN(x,v); r = MAX(r,v);
        v = p->y; y = MIN(y,v); t = MAX(t,v);
        v = p->y+p->ny; y = MIN(y,v); t = MAX(t,v);
        v = p->y+p->sy; y = MIN(y,v); t = MAX(t,v);
        v = p->y+p->wy; y = MIN(y,v); t = MAX(t,v);
        v = p->y+p->ey; y = MIN(y,v); t = MAX(t,v);
        p++;
    }
    ix = fast_floor(x);
    iy = fast_floor(y);
    ir = fast_floor(r)+1;
    it = fast_floor(t)+1;
}

MGrid *MGrid::dup() const
{
    MGrid *g = new MGrid();
    copy_to(g);
    return g;
}

void MGrid::lerp(MGrid *a, MGrid *b, float f)
{
    if (width!=a->width || width!=b->width) return; // ooops!
    float g = 1.0f-f;
    MPoint *pa = a->array(), *pb = b->array(), *pd = array();
    for (unsigned i=points.size(); i>0; i--) {
        pd->x  = pa->x*g  + pb->x*f;  pd->y  = pa->y*g  + pb->y*f; 
        pd->nx = pa->nx*g + pb->nx*f; pd->ny = pa->ny*g + pb->ny*f; 
        pd->sx = pa->sx*g + pb->sx*f; pd->sy = pa->sy*g + pb->sy*f; 
        pd->ex = pa->ex*g + pb->ex*f; pd->ey = pa->ey*g + pb->ey*f; 
        pd->wx = pa->wx*g + pb->wx*f; pd->wy = pa->wy*g + pb->wy*f; 
        pa++; pb++; pd++;
    }
}

void MGrid::resize(int w, int h)
{
    points.resize(w*h);
    width = w; height = h;
}

void MGrid::copy_to(MGrid* d) const
{
    if (this == d) return;
    d->resize(width, height);
    d->points = points;
}

MPoint &MGrid::at(int x, int y)
{
    x = clamp(x, 0, width-1);
    y = clamp(y, 0, height-1);
    return points[x+width*y];
}

void MGrid::getXY(int n, int& x, int& y) const
{
    n = clamp(n, 0, (int)points.size()-1);
    x = n%width;
    y = n/width;
}

void MGrid::remove_curve(int ix)
{
    MGrid dst;
    int del = ix>>1;
    if (ix&1) { // remove a y curve
        // Disallow reducing size to one dimension
        if ( height <= 2 ) {
            return;
        }
        dst.resize(width, height-1);
        MPoint *s = array(), *d = dst.array();
        for (int y=0; y<height; y++) {
            for (int x=0; x<width; x++) {
                if (y!=del) {
                    *d = *s;
                    if (y==del-1) { d->nx*=2.0f; d->ny*=2.0f; }
                    if (y==del+1) { d->sx*=2.0f; d->sy*=2.0f; }
                    d++;
                }
                s++;
            }
        }
    } else { // remove the x curve
        // Disallow reducing size to one dimension
        if ( width <= 2 ) {
            return;
        }
        dst.resize(width-1, height);
        MPoint *s = array(), *d = dst.array();
        for (int y=0; y<height; y++) {
            for (int x=0; x<width; x++) {
                if (x!=del) {
                    *d = *s;
                    if (x==del-1) { d->ex*=2.0f; d->ey*=2.0f; }
                    if (x==del+1) { d->wx*=2.0f; d->wy*=2.0f; }
                    d++;
                }
                s++;
            }
        }
    }
    dst.copy_to(this);
}

void MGrid::add_curve(int ix, float mul)
{
    int x, y;
    MGrid dst;
    float imul = 1.0f-mul;
    if (ix&1) { // add a y curve (horizontal)
        int add = (ix>>1)&0x7fff;
        dst.resize(width, height+1);
        MPoint *s = array(), *d = dst.array();
        for (y=0; y<height+1; y++) {
            for (x=0; x<width; x++) {
                if (y==add) {
                    MPoint &s1 = dst.at(x, y-1), &s2 = at(x, y);

                    // Use absolute coords for the tangents
                    float s1nx = s1.nx + s1.x;
                    float s1ny = s1.ny + s1.y;
                    float s2sx = s2.sx + s2.x;
                    float s2sy = s2.sy + s2.y;

                    float z;
                    z     = scalarLerp(mul, s1nx,  s2sx);
                    s1nx  = scalarLerp(mul, s1.x,  s1nx);
                    s2sx  = scalarLerp(mul, s2sx,  s2.x);
                    d->sx = scalarLerp(mul, s1nx,  z);
                    d->nx = scalarLerp(mul, z,     s2sx);
                    d->x  = scalarLerp(mul, d->sx, d->nx);

                    z     = scalarLerp(mul, s1ny,  s2sy);
                    s1ny  = scalarLerp(mul, s1.y,  s1ny);
                    s2sy  = scalarLerp(mul, s2sy,  s2.y);
                    d->sy = scalarLerp(mul, s1ny,  z);
                    d->ny = scalarLerp(mul, z,     s2sy);
                    d->y  = scalarLerp(mul, d->sy, d->ny);

                    // Put new point's tangents back to proper origin
                    d->nx -= d->x;
                    d->ny -= d->y;
                    d->sx -= d->x;
                    d->sy -= d->y;

                    // calculate the E/W handles by interpolating the neighbors E/W's
                    d->ex = bezier_at(mul, s1.x+s1.ex, s1.nx, s2.sx, s2.x+s2.ex) - d->x;
                    d->ey = bezier_at(mul, s1.y+s1.ey, s1.ny, s2.sy, s2.y+s2.ey) - d->y;
                    d->wx = bezier_at(mul, s1.x+s1.wx, s1.nx, s2.sx, s2.x+s2.wx) - d->x;
                    d->wy = bezier_at(mul, s1.y+s1.wy, s1.ny, s2.sy, s2.y+s2.wy) - d->y;

                } else {
                    *d = *s;
                    s++;
                }
                d++;
            }
        }
        dst.copy_to(this);
        // adjust the handles of the adjacent points
        if (add>0) for (x=0; x<width; x++) {
            MPoint &d = at(x, add-1); d.nx*=mul; d.ny*=mul; 
        }
        if (add<height-1) for (x=0; x<width; x++) {
            MPoint &d = at(x, add+1); d.sx*=imul; d.sy*=imul;
        }
    } else { // add the x curve (vertical)
        int add = (ix>>16)&0x7fff;
        dst.resize(width+1, height);
        MPoint *s = array(), *d = dst.array();
        for (y=0; y<height; y++) {
            for (x=0; x<width+1; x++) {
                if (x==add) {
                    MPoint &s1 = dst.at(x-1, y), &s2 = at(x, y);

                    // Use absolute coords for the tangents
                    float s1ex = s1.ex + s1.x;
                    float s1ey = s1.ey + s1.y;
                    float s2wx = s2.wx + s2.x;
                    float s2wy = s2.wy + s2.y;

                    float z;
                    z     = scalarLerp(mul, s1ex,  s2wx);
                    s1ex  = scalarLerp(mul, s1.x,  s1ex);
                    s2wx  = scalarLerp(mul, s2wx,  s2.x);
                    d->wx = scalarLerp(mul, s1ex,  z);
                    d->ex = scalarLerp(mul, z,     s2wx);
                    d->x  = scalarLerp(mul, d->wx, d->ex);

                    z     = scalarLerp(mul, s1ey,  s2wy);
                    s1ey  = scalarLerp(mul, s1.y,  s1ey);
                    s2wy  = scalarLerp(mul, s2wy,  s2.y);
                    d->wy = scalarLerp(mul, s1ey,  z);
                    d->ey = scalarLerp(mul, z,     s2wy);
                    d->y  = scalarLerp(mul, d->wy, d->ey);

                    // Put new point's tangents back to proper origin
                    d->ex -= d->x;
                    d->ey -= d->y;
                    d->wx -= d->x;
                    d->wy -= d->y;

                    // calculate the N/S handles by interpolating the neighbors N/S's
                    d->nx = bezier_at(mul, s1.x+s1.nx, s1.ex, s2.wx, s2.x+s2.nx) - d->x;
                    d->ny = bezier_at(mul, s1.y+s1.ny, s1.ey, s2.wy, s2.y+s2.ny) - d->y;
                    d->sx = bezier_at(mul, s1.x+s1.sx, s1.ex, s2.wx, s2.x+s2.sx) - d->x;
                    d->sy = bezier_at(mul, s1.y+s1.sy, s1.ey, s2.wy, s2.y+s2.sy) - d->y;

                } else {
                    *d = *s;
                    s++;
                }
                d++;
            }
        }
        dst.copy_to(this);
        // adjust the handles of the adjacent points
        if (add>0) for (y=0; y<height; y++) {
            MPoint &d = at(add-1, y); d.ex*=mul; d.ey*=mul;
        }
        if (add<width-1) for (y=0; y<height; y++) {
            MPoint &d = at(add+1, y); d.wx*=imul; d.wy*=imul;
        }
    }
}

#if 0
void MGrid::from_script(const Script_List& list) {
{
    // read each point into the grid:
    int j = 0;
    for (int y=0; y<height; y++) {
        for (int x=0; x<width; x++) {
            Script_List list2(list[j++]);
            if (list2.size() != 10) return true;
            MPoint& p = points[x+width*y];
            double d;
            if (!to_double(list2[0], d)) return true; p.x  = float(d);
            if (!to_double(list2[1], d)) return true; p.y  = float(d);
            if (!to_double(list2[2], d)) return true; p.nx = float(d);
            if (!to_double(list2[3], d)) return true; p.ny = float(d);
            if (!to_double(list2[4], d)) return true; p.sx = float(d);
            if (!to_double(list2[5], d)) return true; p.sy = float(d);
            if (!to_double(list2[6], d)) return true; p.wx = float(d);
            if (!to_double(list2[7], d)) return true; p.wy = float(d);
            if (!to_double(list2[8], d)) return true; p.ex = float(d);
            if (!to_double(list2[9], d)) return true; p.ey = float(d);
        }
    }
}
#endif

void MGrid::to_script(std::ostream& o) const
{
    for (int y=0; y<height; y++) {
        for (int x=0; x<width; x++) {
            const MPoint& p = points[x+width*y];
            o << "{";
            o << p.x << " " << p.y << " ";
            o << p.nx << " " << p.ny << " " << p.sx << " " << p.sy << " ";
            o << p.wx << " " << p.wy << " " << p.ex << " " << p.ey;
            o << "}" << endl;
        }
    }
}

//==== GridKnob ===============================================================

#include <DDImage/ShapeKnob.h>

// some constants for drawing the transformation jack
static const double radius = 30;
static const double handle_length = 90;
static const double skew_length = 50;
static bool control = false; // controlkey was held down on event-push
#define dscale() sqrt(ctx->zoom())

namespace DD { namespace GridWarp {

/** The grid knob is a data storage that provides
 * a bezier grid (patch) style user interface element
 */
class Grid_Knob : public ShapeKnob {
    private:
        std::vector<MGrid> gridlist;

        // Backlink to Iop for fast preview operations
        WarpIop *iop;
        // This is our grid of nodes including bezier handles
        MGrid cooked_grid, jack_grid;
        // Make sure that the resolution of the partner grid stays the same as in 
        // this grid
        Grid_Knob *partner_;
        // user wants to see the grid
        bool show_grid_;
        // User might also want to just lock the grid
        bool _lockGrid;
        // additinally to 'edit', we support an 'add' mode and 'remove' mode
        int mode;

        // TransformJack stuff
        int prevSelected;
        float jack_tx, jack_ty, jack_cx, jack_cy, jack_rotate, jack_irotate;
        float jack_skew, jack_scale_x, jack_scale_y;
        double pixel_aspect;

    protected:

        /*virtual*/ int get_key_list(std::set<int> &keylist) const;
        /*virtual*/ bool not_default() const { return true; }
        /*virtual*/ bool from_script(const char* text);
#if DD_IMAGE_VERSION_MAJOR < 5
        /*virtual*/ void to_script(std::ostream& o, ToScriptReason, bool quote) const;
#else
        // changed to OutputContext object for proxy access in 5.0
        /*virtual*/ void to_script(std::ostream& o, const OutputContext*, bool quote) const;
#endif

    public:
        enum { MODE_EDIT, MODE_ADD, MODE_REMOVE };

        bool transform_me, transform_dirty;
        Knob* track;
        Matrix4 matrix, inverse_matrix;
	bool need_first_xform;
	bool just_been_read;
	GridTransform xform;
        // !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
	//
	// Major HACK.
	//   When version 3 is loaded, both source and destination
	//   do not have any transforms. The transform is then loaded
	//   from the transform knob which got read after the grids.
	//     0. call knobs
	//     1. read grids
	//     2. read which src/dst transform knob controls
	//     3. read transform knob settings
	//     4. call knobs
	//     4a set transform for source or destination grid base
	//        on #2 in the second call to knobs
	//   If this is not done, the control panel will have to be
	//   launched first to have the grid transforms set up in
	//   knob_changed.
        //
        // !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
	//
	// This variable is set to true in Grid_Knob::from_script so
	//   we know that Nuke just read a version < 4
	bool loading_pre_v4;

#if DD_IMAGE_VERSION_MAJOR < 5
        Grid_Knob(MGrid* src, const char* n, const char* shapename);
#else
        Grid_Knob( Knob_Closure* cb,
                  MGrid* src, const char* n, const char* shapename );
#endif

	void transform_changed();
	void update_transform ( double frame );

        void partner(Grid_Knob *k) { partner_ = k; }
        void show_grid(bool v) { show_grid_ = v; }
        void lock_grid( bool v ) { _lockGrid = v; }
        void remove_mode() { mode = mode ? MODE_EDIT : MODE_REMOVE; }
        void add_mode() { mode = mode ? MODE_EDIT : MODE_ADD; }
        void remove_curve(int ix);
        void add_curve(int ix, float at);
#if DD_IMAGE_VERSION_MAJOR < 5
#else
        // changed to OutputContext object for proxy access in 5.0
	bool gotoContext(const OutputContext&);
#endif

//      /*virtual*/ void changed() { ShapeKnob::changed(); }
        /*virtual*/ bool keyable() const { return cooked_grid.num_points() != 0; }
        /*virtual*/ bool goto_frame(double frame);
#if DD_IMAGE_VERSION_MAJOR < 5
        /*virtual*/ bool goto_shape(float shape, double frame);
        /*virtual*/ void store(void* dest, double frame);
        /*virtual*/ void append(Hash&, double frame);
#else
        // changed to OutputContext object for proxy access in 5.0
        /*virtual*/ bool goto_shape(float shape, const OutputContext &context);
        /*virtual*/ void store(DD::Image::StoreType, void* dest, Hash&, const OutputContext&);
        /*virtual*/ void append(Hash&, const OutputContext*);
#endif
        /*virtual*/ bool build_handle(ViewerContext *ctx);
        /*virtual*/ void draw_handle(ViewerContext *ctx);

        bool do_menu(ViewerContext*, int n);
        void draw_handle_edit(ViewerContext *ctx);
        void jack_changed(ViewerContext* ctx);
        void paste( const MGrid &fromGrid );
        void smooth(int n);

        void draw_xform_jack(ViewerContext *ctx);
        void draw_handle_remove(ViewerContext *ctx);
        bool remove_handle(ViewerContext* ctx, int index);
        float find_fraction(int index, float mx, float my);
        void draw_handle_add(ViewerContext *ctx);
        bool add_handle(ViewerContext* ctx, int index);
        bool drop_handle(ViewerContext* ctx, int index);
        bool move_handle(ViewerContext* ctx, int index);

        /*virtual*/ void set_key(int key);
        /*virtual*/ void add_key(int key);
        /*virtual*/ void delete_keys(int first, int number);

        void fitRect(int xi, int yi, int ri, int ti);

        void set_track( Knob* track ) { this->track = track; }

        static bool drag_translate(ViewerContext* ctx, Knob* p, int);
        static bool drag_rotate(ViewerContext* ctx, Knob* p, int);
        static bool drag_skew(ViewerContext* ctx, Knob* p, int);
        static bool drag_scale(ViewerContext* ctx, Knob* p, int corner);
        static bool drag_scale_x(ViewerContext* ctx, Knob* p, int corner);
        static bool drag_scale_y(ViewerContext* ctx, Knob* p, int corner);
        static bool drag_center(ViewerContext* ctx, Knob* p, int);
        static bool remove_cb(ViewerContext* ctx, Knob* data, int index);
        static bool add_cb(ViewerContext* ctx, Knob* data, int index);
        static bool xy_cb(ViewerContext* ctx, Knob* data, int index);

#ifdef DEBUG_TEST_KNOBS
        friend std::ostream& operator<< ( std::ostream& out, Grid_Knob const& knob ) {
#if DD_IMAGE_VERSION_MAJOR < 5
	    knob.to_script( out, Knob::TO_SCRIPT, false );
#else
	    knob.to_script( out, &(knob.uiContext()), false );
#endif
	    return out;
	}
#endif

#if DD_IMAGE_VERSION_MAJOR < 5
#else
    const char* Class() const { return "Grid_Knob"; }
#endif
};

}} // namespace DD::GridWarp

// create a new data knob. Key reference can not be changed later.
#if DD_IMAGE_VERSION_MAJOR < 5
Grid_Knob::Grid_Knob(MGrid* src, const char* n, const char* shapename) :
    ShapeKnob(n, shapename),
#else
Grid_Knob::Grid_Knob( Knob_Closure* cb,
                     MGrid* src, const char* n, const char* shapename) :
    ShapeKnob( cb, n, shapename ),
#endif
    _lockGrid( false )
{
    if (src) {
        src->copy_to(&cooked_grid);
        cooked_grid.color = src->color;
    } else {
        cooked_grid.init(3, 3);
    }
    partner_ = 0;
    show_grid_ = true;
    mode = MODE_EDIT;
    iop = 0;
    prevSelected = 0;
    pixel_aspect = 1;
    matrix.makeIdentity();
    need_first_xform = true;
    just_been_read = false;
    xform.make_identity();
    inverse_matrix.makeIdentity();
    gridlist.reserve(3);
    transform_me = false;
    transform_dirty = true;
    // !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
    //
    // Major HACK.
    //   When version 3 is loaded, both source and destination
    //   do not have any transforms. The transform is then loaded
    //   from the transform knob which got read after the grids.
    //     0. call knobs
    //     1. read grids
    //     2. read which src/dst transform knob controls
    //     3. read transform knob settings
    //     4. call knobs
    //     4a set transform for source or destination grid base
    //        on #2 in the second call to knobs
    //   If this is not done, the control panel will have to be
    //   launched first to have the grid transforms set up in
    //   knob_changed.
    //
    // !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
    //
    // This variable is initialize to false on creation
    loading_pre_v4 = false;
}

void Grid_Knob::transform_changed()
{
#if DD_IMAGE_VERSION_MAJOR < 5
    xform.evaluate( track, track->frame() );
#else
    // changed to OutputContext object for proxy access in 5.0
    xform.evaluate( track, uiContext().frame() );
#endif
}

void Grid_Knob::update_transform( double frame )
{
    xform.goto_frame( track, frame );
}

// fill in a list with keyframe data
/*virtual*/
int Grid_Knob::get_key_list(std::set<int> &keylist) const
{
    if (!show_grid_) return 0;
    return ShapeKnob::get_key_list(keylist);
}

/*virtual*/
void Grid_Knob::set_key(int key)
{
    if (key == 0 && gridlist.empty()) gridlist.push_back(MGrid());
    new_undo();
    MGrid uncooked( cooked_grid );
    uncooked.transform( inverse_matrix );
    uncooked.copy_to( &gridlist[key] );
    changed();
}

/*virtual*/
void Grid_Knob::add_key(int key)
{
    gridlist.insert(gridlist.begin()+key, MGrid());
    MGrid uncooked( cooked_grid );
    uncooked.transform( inverse_matrix );
    uncooked.copy_to( &gridlist[key] );
}

/*virtual*/
void Grid_Knob::delete_keys(int first, int number)
{
    if (!number) return;
    gridlist.erase(gridlist.begin()+first, gridlist.begin()+first+number-1);
}

/*virtual*/
#if DD_IMAGE_VERSION_MAJOR < 5
bool Grid_Knob::goto_shape(float shape, double frame)
#else
// changed to OutputContext object for proxy access in 5.0
bool Grid_Knob::goto_shape(float shape, const OutputContext& context)
#endif
{
#if DD_IMAGE_VERSION_MAJOR < 5
#else
    // changed to OutputContext object for proxy access in 5.0
    double frame = context.frame();
#endif
    if (need_first_xform) {
        if (xform.is_identity())
	{
	    xform.from_knob( track );
	}
	need_first_xform = false;
    }
    if (transform_me) {
	if (!just_been_read)
	{
            xform.from_knob( track );
	}
    }
    update_transform( frame );
    bool dirty = false;
#if DD_IMAGE_VERSION_MAJOR < 5
    pixel_aspect = proxy_pixel_aspect();
#else
    // changed to OutputContext object for proxy access in 5.0
    context.to_proxy_pixel_aspect( pixel_aspect );
#endif
    Matrix4 m = xform.get_matrix();
    if (transform_dirty || m != matrix) {
        dirty = true;
        // Current points are already baked. Put them back to where they were
        // before and then apply the current tracking transformation.
        cooked_grid.transform( inverse_matrix );
        matrix = m;
        inverse_matrix = m.inverse();
        transform_dirty = false;
    }

    // no keyframes: return current shape:
    if (gridlist.empty()) {
        if ( dirty ) cooked_grid.transform( matrix );
        return true;
    }

    // Figure out the integer and 0-1 't' for the interpolation:
    int first_key = fast_floor(shape);
    if (first_key > keys()-2) first_key = keys()-2;
    if (first_key < 0) first_key = 0;

    float t = shape - (float)first_key;
    if (t == 0.0f) {
        // We are on a key, copy it:
        gridlist[first_key].copy_to( &cooked_grid );
    } else if (t == 1.0f) {
        // We are on a key, copy it:
        gridlist[first_key+1].copy_to( &cooked_grid );
    } else {
        // Get the interpolated grid between two shapes:
        cooked_grid.resize(gridlist[first_key].width, gridlist[first_key].height);
        cooked_grid.lerp(&gridlist[first_key], &gridlist[first_key+1], t);
    }
    cooked_grid.transform( matrix );

    return true;
}

void Grid_Knob::paste( const MGrid &fromGrid )
{
    new_undo();
    fromGrid.copy_to( &cooked_grid );
#if DD_IMAGE_VERSION_MAJOR < 5
    maybe_set_key(frame());
#else
    maybe_set_key();
#endif
}

// make the given grid fit in to the rectangle
void Grid_Knob::fitRect(int xi, int yi, int ri, int ti)
{
    float x = xi, y = yi, r = ri, t = ti;
    cooked_grid.fitRect((int)x, (int)y, (int)r, (int)t);
    cooked_grid.transform( matrix );
#if DD_IMAGE_VERSION_MAJOR < 5
    maybe_set_key(frame());
#else
    maybe_set_key();
#endif
}

/*virtual*/
#if DD_IMAGE_VERSION_MAJOR < 5
void Grid_Knob::to_script(std::ostream& o, ToScriptReason, bool quote) const
#else
void Grid_Knob::to_script(std::ostream& o, const OutputContext*, bool quote) const
#endif
{
    // header:
    if (quote) o << "{";
    o << GRID_VERSION << " " << cooked_grid.width << " " << cooked_grid.height << " "; 
    // transform
    o << xform;

    if (gridlist.empty()) {
        // only write out current grid
        o << " {{\n";
        MGrid uncooked( cooked_grid );
        uncooked.transform( inverse_matrix );
        uncooked.to_script(o);
        o << "}}";
    } else {
        // write keys:
        for (int i=0; i < keys(); i++) {
            if (i) o << "} {\n"; else o << " {{\n";
            gridlist[i].to_script(o);
        }
        o << "}}";
    }
    if (quote) o << "}";
}

// copy data from a text string
/*virtual*/
bool Grid_Knob::from_script(const char* text)
{
    Script_List list(text);
    if (list.error()) {
        error("grid script definition has improper formatting");
        return false;
    }
    // header:
    int version; to_int(list[0], version);
    if (version == GRID_VERSION) {
        // version 4
        /* new format:
            srcgrid {4 4 4 { {rotate {curve x1 0 x40 -53.97262878}} {center 320 240} {matrix {0.588172 0.808736 0 -62.3116} {-0.808736 0.588172 0 357.634} {0 0 1 0} {0 0 0 1}} {identity false} } {{
            {100 100 0 16.6667 0 -16.6667 -16.6667 0 16.6667 0 0}
            } {
            {100 100 0 16.6667 0 -16.6667 -16.6667 0 16.6667 0 0}
            } {
            {100 100 0 16.6667 0 -16.6667 -16.6667 0 16.6667 0 0}
            }}}
        */
        int width;  to_int(list[1], width);
        int height; to_int(list[2], height);

	// !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
	//
	// This little manoever here is needed due to parsing incompatibility
	//   SplineWarp2 { transform_form_script -> extract_bracket_expression }
	// here. The SplineWarp functions needed the outside " {"/"} " pair to
	// be part of the input string. The list[3] is without these delimiters.
	//
	// This temporary fix just added " {" and "} " to list[3] and it works.
	// This should probably be re-written to conform to not use Script_List
	// and conform to SplineWarp2 convention.
	//
	// !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
	//
	std::string compatible_string;
	compatible_string  = " {";
	compatible_string += list[3];
	compatible_string += "} ";
	const char *s = compatible_string.c_str();
	if (transform_from_script( s, xform ) == false) {
	    return false;
	}
	matrix = xform.get_matrix();
	inverse_matrix = matrix.inverse();
	just_been_read = true;
	need_first_xform = false;

        if (list.size() == 5) {
            Script_List list1(list[4]);
//printf("VERSION 4 grid has %d keys\n", list1.size());
            keys(list1.size());

            gridlist.clear();
            gridlist.reserve(keys());

            int num_points = width*height;
            for (int n=0; n < keys(); n++) {
                Script_List list2(list1[n]);
//printf("VERSION 4 key %d(%d)\n", n, num_points);
                if (list2.error()) return true;
                // make sure point count is same as grid definition:
                if (list2.size() != num_points) {
                    error("grid has %d points, key %d has %d points", num_points, n, list2.size());
                    return false;
                }

                gridlist.push_back(MGrid());
                MGrid& g = gridlist[n];
                g.resize(width, height);
                memset(g.array(), 0, g.num_points()*sizeof(MPoint));

                // read each point into the grid:
                int j = 0;
                for (int y=0; y<g.height; y++) {
                    for (int x=0; x<g.width; x++) {
                        Script_List list3(list2[j++]);
                        if (list3.size() != 10) return true;
                        MPoint& p = g.at(x, y);
                        double d;
                        if (!to_double(list3[0], d)) return true; p.x  = float(d);
                        if (!to_double(list3[1], d)) return true; p.y  = float(d);
                        if (!to_double(list3[2], d)) return true; p.nx = float(d);
                        if (!to_double(list3[3], d)) return true; p.ny = float(d);
                        if (!to_double(list3[4], d)) return true; p.sx = float(d);
                        if (!to_double(list3[5], d)) return true; p.sy = float(d);
                        if (!to_double(list3[6], d)) return true; p.wx = float(d);
                        if (!to_double(list3[7], d)) return true; p.wy = float(d);
                        if (!to_double(list3[8], d)) return true; p.ex = float(d);
                        if (!to_double(list3[9], d)) return true; p.ey = float(d);
                    }
                }
            }
            // update the transform jack
            prevSelected = -1;

            cooked_grid.transform( matrix );

        }
    } else if (version == GRID_VERSION_3) {
        // version 3
        /* format:
            srcgrid {3 4 4 {{
            {100 100 0 16.6667 0 -16.6667 -16.6667 0 16.6667 0 0}
            } {
            {100 100 0 16.6667 0 -16.6667 -16.6667 0 16.6667 0 0}
            } {
            {100 100 0 16.6667 0 -16.6667 -16.6667 0 16.6667 0 0}
            }}}
        */
        int width;  to_int(list[1], width);
        int height; to_int(list[2], height);

        if (list.size() == 4) {
            Script_List list1(list[3]);
//printf("VERSION 3 grid has %d keys\n", list1.size());
            keys(list1.size());

            gridlist.clear();
            gridlist.reserve(keys());

            int num_points = width*height;
            for (int n=0; n < keys(); n++) {
                Script_List list2(list1[n]);
//printf("VERSION 3 key %d(%d)\n", n, num_points);
                if (list2.error()) return true;
                // make sure point count is same as grid definition:
                if (list2.size() != num_points) {
                    error("grid has %d points, key %d has %d points", num_points, n, list2.size());
                    return false;
                }

                gridlist.push_back(MGrid());
                MGrid& g = gridlist[n];
                g.resize(width, height);
                memset(g.array(), 0, g.num_points()*sizeof(MPoint));

                // read each point into the grid:
                int j = 0;
                for (int y=0; y<g.height; y++) {
                    for (int x=0; x<g.width; x++) {
                        Script_List list3(list2[j++]);
                        if (list3.size() != 10) return true;
                        MPoint& p = g.at(x, y);
                        double d;
                        if (!to_double(list3[0], d)) return true; p.x  = float(d);
                        if (!to_double(list3[1], d)) return true; p.y  = float(d);
                        if (!to_double(list3[2], d)) return true; p.nx = float(d);
                        if (!to_double(list3[3], d)) return true; p.ny = float(d);
                        if (!to_double(list3[4], d)) return true; p.sx = float(d);
                        if (!to_double(list3[5], d)) return true; p.sy = float(d);
                        if (!to_double(list3[6], d)) return true; p.wx = float(d);
                        if (!to_double(list3[7], d)) return true; p.wy = float(d);
                        if (!to_double(list3[8], d)) return true; p.ex = float(d);
                        if (!to_double(list3[9], d)) return true; p.ey = float(d);
                    }
                }
            }
            // update the transform jack
            prevSelected = -1;

            // applying identity matrix?
            cooked_grid.transform( matrix );

            // !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
	    //
	    // Major HACK.
	    //   When version 3 is loaded, both source and destination
	    //   do not have any transforms. The transform is then loaded
	    //   from the transform knob which got read after the grids.
	    //     0. call knobs
	    //     1. read grids
	    //     2. read which src/dst transform knob controls
	    //     3. read transform knob settings
	    //     4. call knobs
	    //     4a set transform for source or destination grid base
	    //        on #2 in the second call to knobs
	    //   If this is not done, the control panel will have to be
	    //   launched first to have the grid transforms set up in
	    //   knob_changed.
            //
            // !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
	    //
	    // This variable is set to true when reading version 3
            loading_pre_v4 = true;
        }
    } else {
        fprintf(stderr, "Older grid format - attempting to read...\n");

        /* old format: 
               srcgrid  {
                 2 4 4 0  {
                   1 4 4 0
                   100 100 0 16.6667 0 -16.6667 -16.6667 0 16.6667 0 0
                 }  {
                   1 2 0 0 0 
                   1  {
                     1 4 4 0
                     100 100 0 16.6667 0 -16.6667 -16.6667 0 16.6667 0 0
                   }
                   37  {
                     1 4 4 0
                     100 100 0 16.6667 0 -16.6667 -16.6667 0 16.6667 0 0
                   }
                 }
               }
        */
        int width;    to_int(list[1], width);
        int height;   to_int(list[2], height);

        if (list.size() != 6) {
            error("grid has unrecognized structure");
            return false;
        } else {
            int list_index;
            bool have_keys = false;
            Script_List tmp(list[5]);
            int num_keys; to_int(tmp[1], num_keys);
            if (num_keys == 0) {
                list_index = 4;
            } else {
                list_index = 5;
                have_keys = true;
            }
            Script_List list1(list[list_index]);
            if (have_keys) {
                keys(num_keys);
                list_index = 5;
            } else {
                keys(1);
            }

            gridlist.clear();
            gridlist.reserve(keys());
            std::string curve_string;
            if (have_keys) {
                //format: {curve L x5 0 x13 9.454545455 x15 11.81818182 x27 26}
                curve_string.reserve(1024);
                curve_string.append("{curve L ");
            }

            char buf[128];
            for (int n=0; n < keys(); n++) {
                if (have_keys) {
                    sprintf(buf, "x%s %d ", list1[list_index++], n);
                    curve_string.append(buf);
                }
 
                gridlist.push_back(MGrid());
                MGrid& g = gridlist[n];
                g.resize(width, height);
                memset(g.array(), 0, g.num_points()*sizeof(MPoint));

                // read each point into the grid:
                const char* s;
                if (have_keys) s = list1[list_index++];
                else s = list[list_index];
                geti(s); geti(s); geti(s); geti(s); // skip header junk
                for (int y=0; y<g.height; y++) {
                    for (int x=0; x<g.width; x++) {
                        MPoint& p = g.at(x, y);
                        p.x = getf(s); p.y = getf(s);
                        p.nx = getf(s); p.ny = getf(s); p.sx = getf(s); p.sy = getf(s);
                        p.wx = getf(s); p.wy = getf(s); p.ex = getf(s); p.ey = getf(s);
                        geti(s);
                    }
                }
            }
            if (have_keys) {
                curve_string.append("}");
                // Now that we have all the keys, pass it to the shape knob:
                ShapeKnob::i->from_script(curve_string.c_str());
            }
        }

         // !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
	 //
	 // Major HACK.
	 //   When version 3 is loaded, both source and destination
	 //   do not have any transforms. The transform is then loaded
	 //   from the transform knob which got read after the grids.
	 //     0. call knobs
	 //     1. read grids
	 //     2. read which src/dst transform knob controls
	 //     3. read transform knob settings
	 //     4. call knobs
	 //     4a set transform for source or destination grid base
	 //        on #2 in the second call to knobs
	 //   If this is not done, the control panel will have to be
	 //   launched first to have the grid transforms set up in
	 //   knob_changed.
         //
         // !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
	 //
	 // This variable is set to true for version < 3
	 loading_pre_v4 = true;
    }

    need_first_xform = false;
    changed();
    return true;
}

// remove a curve from all grids
void Grid_Knob::remove_curve(int ix)
{
    cooked_grid.remove_curve(ix);
    for (unsigned i=0; i < gridlist.size(); i++)
        gridlist[i].remove_curve(ix);
    changed();
}

// add a curve to all grids
void Grid_Knob::add_curve(int ix, float at)
{
    cooked_grid.add_curve(ix, at);
    for (unsigned i=0; i < gridlist.size(); i++)
        gridlist[i].add_curve(ix, at);
    changed();
}

#if DD_IMAGE_VERSION_MAJOR < 5
#else
// set the grid data set to show a different frame number
bool Grid_Knob::gotoContext( const OutputContext& context) {
    return goto_shape( shape(context.frame()), context );
}
#endif

// set the grid data set to show a different frame number
/*virtual*/
bool Grid_Knob::goto_frame(double frame)
{
    return ShapeKnob::goto_frame(frame);
}

// make sure that Nuke knows if the key changed in any way
/*virtual*/
#if DD_IMAGE_VERSION_MAJOR < 5
void Grid_Knob::store(void* dest, double frame)
{
    goto_frame(frame);
    cooked_grid.copy_to((MGrid*)dest);
}
#else
// changed to OutputContext object for proxy access in 5.0
void Grid_Knob::store(StoreType, void* dst, Hash& hash, const OutputContext& context)
{
    gotoContext( context );
    cooked_grid.copy_to((MGrid*)dst);
    hash.append( cooked_grid.array(), sizeof(MPoint)*cooked_grid.num_points() );
}
#endif

/*virtual*/
#if DD_IMAGE_VERSION_MAJOR < 5
void Grid_Knob::append(Hash& hash, double frame) {
    goto_frame(frame);
    hash.append(cooked_grid.array(), sizeof(MPoint)*cooked_grid.num_points());
}
#else
// changed to OutputContext object for proxy access in 5.0
void Grid_Knob::append(Hash& hash, const OutputContext* context) {
    if (context) gotoContext( *context );
    // not right if context is zero, should use whole animation
    hash.append( cooked_grid.array(), sizeof(MPoint)*cooked_grid.num_points() );
}
#endif

/*virtual*/
bool Grid_Knob::build_handle(ViewerContext *ctx)
{
    return (ctx->transform_mode()==VIEWER_2D && isEnabled() && show_grid_);
}

/*virtual*/
void Grid_Knob::draw_handle(ViewerContext *ctx)
{
#if DD_IMAGE_VERSION_MAJOR < 5
    goto_frame(frame());
    pixel_aspect = proxy_pixel_aspect();
#else
    // changed to OutputContext object for proxy access in 5.0
    OutputContext uiCtx = uiContext();
    goto_frame( (float)uiCtx.frame() );
    uiCtx.to_proxy_pixel_aspect( pixel_aspect );
#endif

    // Turn off display when in add/remove mode:
    if (partner_->mode != MODE_EDIT) return;

    if (ctx->event()==PUSH) control = ctx->state(COMMAND);
    switch (mode) {
        case MODE_ADD: draw_handle_add(ctx); break;
        case MODE_REMOVE: draw_handle_remove(ctx); break;
        default: draw_handle_edit(ctx);
    }
}

void Grid_Knob::draw_handle_edit(ViewerContext *ctx)
{
    int x, y, i;
    if (!ctx->draw_knobs()) return;
    if (!this->name()) return;
#if DD_IMAGE_VERSION_MAJOR < 5
#else
    // changed to OutputContext object for proxy access in 5.0
    OutputContext uiCtx = uiContext();
#endif
    // draw colored lines into the mesh
    if (ctx->event()==DRAW_SHADOW) glColor(0);
    else glColor(cooked_grid.color);
    for (y=0; y<cooked_grid.height; y++) {
        glBegin(GL_LINE_STRIP);
#if DD_IMAGE_VERSION_MAJOR < 5
        const float x1 = to_proxy_x(cooked_grid.at(0, y).x);
        const float y1 = to_proxy_y(cooked_grid.at(0, y).y);
#else
        // changed to OutputContext object for proxy access in 5.0
	float x1 = cooked_grid.at( 0, y ).x;
	float y1 = cooked_grid.at( 0, y ).y;
        uiCtx.to_proxy_xy( x1, y1 );
#endif
        glVertex2f(x1, y1);
        for (x=0; x<cooked_grid.width-1; x++) {
#if DD_IMAGE_VERSION_MAJOR < 5
            const float x1 = to_proxy_x(cooked_grid.at(x, y).x);
            const float y1 = to_proxy_y(cooked_grid.at(x, y).y);
            const float x2 = x1+to_proxy_w(cooked_grid.at(x, y).ex);
            const float y2 = y1+to_proxy_h(cooked_grid.at(x, y).ey);
            const float x4 = to_proxy_x(cooked_grid.at(x+1, y).x);
            const float y4 = to_proxy_y(cooked_grid.at(x+1, y).y);
            const float x3 = x4+to_proxy_w(cooked_grid.at(x+1, y).wx);
            const float y3 = y4+to_proxy_h(cooked_grid.at(x+1, y).wy);
#else
            // changed to OutputContext object for proxy access in 5.0
            float x1 = cooked_grid.at( x, y ).x;
            float y1 = cooked_grid.at( x, y ).y;
	    uiCtx.to_proxy_xy( x1, y1 );
            float ex = cooked_grid.at( x, y ).ex;
            float ey = cooked_grid.at( x, y ).ey;
	    uiCtx.to_proxy_wh( ex, ey );
            float x2 = x1 + ex;
            float y2 = y1 + ey;
            float x4 = cooked_grid.at( x+1, y ).x;
            float y4 = cooked_grid.at( x+1, y ).y;
	    uiCtx.to_proxy_xy( x4, y4 );
            float wx = cooked_grid.at( x+1, y ).wx;
            float wy = cooked_grid.at( x+1, y ).wy;
	    uiCtx.to_proxy_wh( wx, wy );
            float x3 = x4 + wx;
            float y3 = y4 + wy;
#endif
            gl_bezierf(x1,y1, x2,y2, x3,y3, x4,y4, ctx->zoom());
        }
        glEnd();
    }
    for (x=0; x<cooked_grid.width; x++) {
        glBegin(GL_LINE_STRIP);
#if DD_IMAGE_VERSION_MAJOR < 5
        const float x1 = to_proxy_x(cooked_grid.at(x, 0).x);
        const float y1 = to_proxy_y(cooked_grid.at(x, 0).y);
#else
        // changed to OutputContext object for proxy access in 5.0
        float x1 = cooked_grid.at( x, 0 ).x;
        float y1 = cooked_grid.at( x, 0 ).y;
	uiCtx.to_proxy_xy( x1, y1 );
#endif
        glVertex2f(x1, y1);
        for (y=0; y<cooked_grid.height-1; y++) {
#if DD_IMAGE_VERSION_MAJOR < 5
            const float x1 = to_proxy_x(cooked_grid.at(x, y).x);
            const float y1 = to_proxy_y(cooked_grid.at(x, y).y);
            const float x2 = x1+to_proxy_w(cooked_grid.at(x, y).nx);
            const float y2 = y1+to_proxy_h(cooked_grid.at(x, y).ny);
            const float x4 = to_proxy_x(cooked_grid.at(x, y+1).x);
            const float y4 = to_proxy_y(cooked_grid.at(x, y+1).y);
            const float x3 = x4+to_proxy_w(cooked_grid.at(x, y+1).sx);
            const float y3 = y4+to_proxy_h(cooked_grid.at(x, y+1).sy);
#else
            // changed to OutputContext object for proxy access in 5.0
            float x1 = cooked_grid.at( x, y ).x;
            float y1 = cooked_grid.at( x, y ).y;
	    uiCtx.to_proxy_xy(  x1,  y1 );
            float nx = cooked_grid.at( x, y ).nx;
            float ny = cooked_grid.at( x, y ).ny;
	    uiCtx.to_proxy_wh( nx, ny );
            float x2 = x1 + nx;
            float y2 = y1 + ny;
            float x4 = cooked_grid.at( x, y+1 ).x;
            float y4 = cooked_grid.at( x, y+1 ).y;
	    uiCtx.to_proxy_xy( x4, y4 );
            float sx = cooked_grid.at( x, y+1 ).sx;
            float sy = cooked_grid.at( x, y+1 ).sy;
	    uiCtx.to_proxy_wh( sx, sy );
            float x3 = x4 + sx;
            float y3 = y4 + sy;
#endif
            gl_bezierf(x1,y1, x2,y2, x3,y3, x4,y4, ctx->zoom());
        }
        glEnd();
    }

    // If grid is locked, don't draw any handles.
    if ( !_lockGrid ) {

        // draw handles in grey (white if selected)
        // glColor(ctx->node_color());
        for (i=0; i<cooked_grid.num_points(); i++) {
            MPoint &p = cooked_grid.points[i];
#if DD_IMAGE_VERSION_MAJOR < 5
            const float x = to_proxy_x(p.x);
            const float y = to_proxy_y(p.y);
#else
            // changed to OutputContext object for proxy access in 5.0
            float x = p.x;
            float y = p.y;
	    uiCtx.to_proxy_xy( x, y );
#endif
            // Draw selected handle bright white
            if (ctx->event()==DRAW_LINES && ShapeKnob::is_selected(ctx, xy_cb, i<<3)) {
                glColor(0xffffff00);
            } else {
                glColor(0xc0c0c000);
            }
            make_handle(SELECTABLE, ctx, xy_cb, i<<3, x, y);
        }
        // draw bright white active handles
        //if (ctx->event()==DRAW_LINES) glColor(0xe0e0e000);
        glColor(ctx->node_color());
        i = 0;
        for (int iy=0; iy<cooked_grid.height; iy++) {
            for (int ix=0; ix<cooked_grid.width; ix++, i++) {
                if (!ShapeKnob::is_selected(ctx, xy_cb, i<<3)) continue;
                MPoint &p = cooked_grid.points[i];
#if DD_IMAGE_VERSION_MAJOR < 5
                const float x = to_proxy_x(p.x);
                const float y = to_proxy_y(p.y);
#else
                // changed to OutputContext object for proxy access in 5.0
                float x = p.x;
                float y = p.y;
	        uiCtx.to_proxy_xy( x, y );
#endif
                if (iy<cooked_grid.height-1) {
#if DD_IMAGE_VERSION_MAJOR < 5
                    const float dx = to_proxy_w(p.nx);
                    const float dy = to_proxy_h(p.ny);
#else
                    // changed to OutputContext object for proxy access in 5.0
                    float dx = p.nx;
                    float dy = p.ny;
		    uiCtx.to_proxy_wh( dx, dy );
#endif
                    make_handle(ctx, xy_cb, (i<<3)+1, x+dx, y+dy);
                    glBegin(GL_LINES); glVertex2d(x, y); glVertex2d(x+dx, y+dy); glEnd();
                }
                if (iy>0) {
#if DD_IMAGE_VERSION_MAJOR < 5
                    const float dx = to_proxy_w(p.sx);
                    const float dy = to_proxy_h(p.sy);
#else
                    // changed to OutputContext object for proxy access in 5.0
                    float dx = p.sx;
                    float dy = p.sy;
		    uiCtx.to_proxy_wh( dx, dy );
#endif
                    make_handle(ctx, xy_cb, (i<<3)+2, x+dx, y+dy);
                    glBegin(GL_LINES); glVertex2d(x, y); glVertex2d(x+dx, y+dy); glEnd();
                }
                if (ix<cooked_grid.width-1) {
#if DD_IMAGE_VERSION_MAJOR < 5
                    const float dx = to_proxy_w(p.ex);
                    const float dy = to_proxy_h(p.ey);
#else
                    // changed to OutputContext object for proxy access in 5.0
                    float dx = p.ex;
                    float dy = p.ey;
		    uiCtx.to_proxy_wh( dx, dy );
#endif
                    make_handle(ctx, xy_cb, (i<<3)+3, x+dx, y+dy);
                    glBegin(GL_LINES); glVertex2d(x, y); glVertex2d(x+dx, y+dy); glEnd();
                }
                if (ix>0) {
#if DD_IMAGE_VERSION_MAJOR < 5
                    const float dx = to_proxy_w(p.wx);
                    const float dy = to_proxy_h(p.wy);
#else
                    // changed to OutputContext object for proxy access in 5.0
                    float dx = p.wx;
                    float dy = p.wy;
		    uiCtx.to_proxy_wh( dx, dy );
#endif
                    make_handle(ctx, xy_cb, (i<<3)+4, x+dx, y+dy);
                    glBegin(GL_LINES); glVertex2d(x, y); glVertex2d(x+dx, y+dy); glEnd();
                }
            }
        }
        if (ctx->event()!=DRAG) {
            draw_xform_jack(ctx);
        }
    }
}

void Grid_Knob::jack_changed(ViewerContext* ctx)
{
    // calculate a transformation matrix:
    Matrix4 m;
    // translate origin to center + translate
    m.translation(jack_cx+jack_tx, jack_cy+jack_ty);
    // rotate, scale
    m.scale(1/pixel_aspect, 1, 1);
    m.rotate((jack_rotate+jack_irotate)*M_PI_2/90);
    m.skew(jack_skew);
    m.scale(jack_scale_x, jack_scale_y, 1.0);
    m.rotate(-jack_irotate*M_PI_2/90);
    m.scale(pixel_aspect, 1, 1);
    // translate center back to original location:
    m.translate(-jack_cx, -jack_cy);
    // transform all the original point positions by this matrix:
    for (int i=0; i<cooked_grid.num_points(); i++)
        if (ShapeKnob::is_selected(ctx, xy_cb, i<<3)) {
            MPoint &p = cooked_grid.points[i];	
            MPoint &o = jack_grid.points[i];	
            p.x = m.a00*o.x + m.a01*o.y + m.a03;
            p.y = m.a10*o.x + m.a11*o.y + m.a13;
            p.nx = m.a00*o.nx + m.a01*o.ny; p.ny = m.a10*o.nx + m.a11*o.ny;
            p.sx = m.a00*o.sx + m.a01*o.sy; p.sy = m.a10*o.sx + m.a11*o.sy;
            p.ex = m.a00*o.ex + m.a01*o.ey; p.ey = m.a10*o.ex + m.a11*o.ey;
            p.wx = m.a00*o.wx + m.a01*o.wy; p.wy = m.a10*o.wx + m.a11*o.wy;
        }
#if DD_IMAGE_VERSION_MAJOR < 5
    maybe_set_key(frame());
#else
    maybe_set_key();
#endif
}

////////////////////////////////////////////////////////////////

// events produced by the menu:
enum {
    SMOOTH = FIRST_MENU,
    CUSP,
    SELECTALL
};

// Creates the Menu object when it is first needed:
static Menu* make_menu() {
    static Menu* menu = 0;
    if (menu) return menu;
    menu = new Menu;
    menu->add("select all", COMMAND+'A', SELECTALL);
    menu->add("smooth", 'z', SMOOTH);
    menu->add("cusp", 'c', CUSP);
    menu->add("set key", 0, ShapeKnob::SET_KEY);
    menu->add("delete key", CTRL+'X', ShapeKnob::DELETE_KEY);
    menu->add("copy key", CTRL+'C', ShapeKnob::COPY_KEY);
    menu->add("paste key", CTRL+'V', ShapeKnob::PASTE_KEY);
    return menu;
}

// Does a menu item to one point:
bool Grid_Knob::do_menu(ViewerContext* ctx, int n) {
	const int nPoints = cooked_grid.num_points();
    // Call do_menu on all selected points:
    static bool recurse = false;
    if (!recurse && ShapeKnob::is_selected(ctx, xy_cb, n)) {
        recurse = true;
        for (int i = nPoints; i--;)
            if (ShapeKnob::is_selected(ctx, xy_cb, i<<3)) do_menu(ctx, i);
        recurse = false;
        return true;
    }
    assert(n>=0 && n < nPoints);

    // transform cooked grid back to source space:
    switch (ctx->event()) {
    case SMOOTH:
        smooth(n);
        break;
    case CUSP: {
        MPoint& p = cooked_grid.points[n];
        p.nx = p.sx = p.ex = p.wx = 0.0f;
        p.ny = p.sy = p.ey = p.wy = 0.0f;
        break;
        }
    case ShapeKnob::SET_KEY:
    case ShapeKnob::DELETE_KEY:
    case ShapeKnob::COPY_KEY:
    case ShapeKnob::PASTE_KEY:
#if DD_IMAGE_VERSION_MAJOR < 5
        return handle(ctx->event(), frame());
#else
#if (DD_IMAGE_VERSION_MAJOR == 5 && DD_IMAGE_VERSION_MINOR < 1)
        return handle(ctx->event());
#else
        return handle(ctx->event(), uiContext().frame());
#endif
#endif
    case SELECTALL: {
        int i;
        for (i=0; i < nPoints; i++)
            if (!ShapeKnob::is_selected(ctx, xy_cb, i)) break;
        if (i >= nPoints) return false; // all selected
        for (i = 0; i < nPoints; i++)
            make_handle(SELECTED_BY_THIS, ctx, xy_cb, i<<3,
                          cooked_grid.points[i].x, cooked_grid.points[i].y);
        return true;
        }
    default:
        return false;
    }
#if DD_IMAGE_VERSION_MAJOR < 5
    maybe_set_key(frame());
#else
    maybe_set_key();
#endif
    return true;
}

void Grid_Knob::smooth(int n) {
	int x, y;
    cooked_grid.getXY(n, x, y);
    MPoint& p =  cooked_grid.at(  x,   y);
    MPoint& pn = cooked_grid.at(  x, y+1);
    MPoint& ps = cooked_grid.at(  x, y-1);
    MPoint& pe = cooked_grid.at(x+1,   y);
    MPoint& pw = cooked_grid.at(x-1,   y);
    p.transform( inverse_matrix );
    if (&pn != &p) pn.transform( inverse_matrix );
    if (&ps != &p) ps.transform( inverse_matrix );
    if (&pe != &p) pe.transform( inverse_matrix );
    if (&pw != &p) pw.transform( inverse_matrix );

    float dx = pn.x - ps.x;
    float dy = pn.y - ps.y;
    float len = sqrtf(dx*dx + dy*dy);
    dx /= len;
    dy /= len;
    if (&pn != &p) {
        float lx = pn.x - p.x;
        float ly = pn.y - p.y;
        len = sqrtf(lx*lx + ly*ly)/3.0f;
        p.nx = dx*len;
        p.ny = dy*len;
        pn.transform( matrix );
    } else {
        p.nx = p.ny = 0.0f;
    }
    if (&ps != &p) {
        float lx = ps.x - p.x;
        float ly = ps.y - p.y;
        len = sqrtf(lx*lx + ly*ly)/3.0f;
        p.sx = -dx*len;
        p.sy = -dy*len;
        ps.transform( matrix );
    } else {
        p.sx = p.sy = 0.0f;
    }

    dx = pe.x - pw.x;
    dy = pe.y - pw.y;
    len = sqrtf(dx*dx + dy*dy);
    dx /= len;
    dy /= len;
    if (&pe != &p) {
        float lx = pe.x - p.x;
        float ly = pe.y - p.y;
        len = sqrtf(lx*lx + ly*ly)/3.0f;
        p.ex = dx*len;
        p.ey = dy*len;
        pe.transform( matrix );
    } else {
        p.ex = p.ey = 0.0f;
    }
    if (&pw != &p) {
        float lx = pw.x - p.x;
        float ly = pw.y - p.y;
        len = sqrtf(lx*lx + ly*ly)/3.0f;
        p.wx = -dx*len;
        p.wy = -dy*len;
        pw.transform( matrix );
    } else {
        p.wx = p.wy = 0.0f;
    }

    p.transform( matrix );
}

////////////////////////////////////////////////////////////////

//  static
bool Grid_Knob::drag_translate(ViewerContext* ctx, Knob* p, int)
{
    Grid_Knob &k = *(Grid_Knob*)p;
    if (ctx->event() != DRAG) return false;
#if DD_IMAGE_VERSION_MAJOR < 5
    k.jack_tx = k.from_proxy_x(ctx->x());
    k.jack_ty = k.from_proxy_y(ctx->y());
#else
    // changed to OutputContext object for proxy access in 5.0
    OutputContext kUiCtx = k.uiContext();
    k.jack_tx = ctx->x();
    k.jack_ty = ctx->y();
    kUiCtx.from_proxy_xy( k.jack_tx, k.jack_ty );
#endif
    k.jack_changed(ctx);
    return true;
}

//  static
bool Grid_Knob::drag_rotate(ViewerContext* ctx, Knob* p, int)
{
    static double irotate;
    Grid_Knob &k = *(Grid_Knob*)p;
    if (ctx->event() == PUSH) {irotate = k.jack_irotate; return true;}
    if (ctx->event() != DRAG) return false;
#if DD_IMAGE_VERSION_MAJOR < 5
    float mx = k.from_proxy_x(ctx->x());
    float my = k.from_proxy_y(ctx->y());
#else
    // changed to OutputContext object for proxy access in 5.0
    OutputContext kUiCtx = k.uiContext();
    float mx = ctx->x();
    float my = ctx->y();
    kUiCtx.from_proxy_xy( mx, my );
#endif
    if (k.jack_scale_x < 0) {mx = -mx; my = -my;}
    float a = atan2(my,mx)*90/M_PI_2;
    // prefer right angles if possible:
    if (!ctx->state(CTRL|SHIFT|ALT|META)) {
        if (fabsf(a) < 5) a = 0;
        if (fabsf(fabsf(a)-90) < 5) a = (a<0) ? -90 : 90;
        if (fabsf(fabsf(a)-180) < 5) a = 180;
    }
    // find the closest angle to the current one, so animation does not spin:
    while (k.jack_rotate > a+180.1f) a += 360;
    while (k.jack_rotate < a-180.1f) a -= 360;
    if (control) {
        while (a > 180) a -= 360;
        while (a <-180) a += 360;
        k.jack_irotate = a+irotate-k.jack_rotate;
        if (k.jack_scale_x == k.jack_scale_y && !k.jack_skew) return true;
    } else {
        k.jack_rotate = a;
    }
    k.jack_changed(ctx);
    return true;
}

//  static
bool Grid_Knob::drag_skew(ViewerContext* ctx, Knob* p, int)
{
    if (ctx->event() != DRAG) return false;
    Grid_Knob &k = *(Grid_Knob*)p;
    if (!ctx->y()) return true;
#if DD_IMAGE_VERSION_MAJOR < 5
    float x = k.from_proxy_x(ctx->x());
    float y = k.from_proxy_y(ctx->y());
#else
    // changed to OutputContext object for proxy access in 5.0
    OutputContext kUiCtx = k.uiContext();
    float x = ctx->x();
    float y = ctx->y();
    kUiCtx.from_proxy_xy( x, y );
#endif
    double v = clamp(x/y, -4, 4);
    if (!ctx->state(CTRL|SHIFT|ALT|META) && fabs(v)<.1) v = 0;
    k.jack_skew = v;
    k.jack_changed(ctx);
    return true;
}

//  static
bool Grid_Knob::drag_scale(ViewerContext* ctx, Knob* p, int corner)
{
    static double scale_ratio; // scale.y/scale.x
    Grid_Knob &k = *(Grid_Knob*)p;
    if (ctx->event() == PUSH) scale_ratio = k.jack_scale_y/k.jack_scale_x;
    if (ctx->event() != DRAG) return false;
#if DD_IMAGE_VERSION_MAJOR < 5
    float mx = p->from_proxy_x(ctx->x());
    float my = p->from_proxy_y(ctx->y());
#else
    // changed to OutputContext object for proxy access in 5.0
    OutputContext kUiCtx = k.uiContext();
    float mx = ctx->x();
    float my = ctx->y();
    kUiCtx.from_proxy_xy( mx, my );
#endif
    double scale = sqrt(mx*mx+my*my/(scale_ratio*scale_ratio))
        / (radius/dscale());
    scale = rint(scale*1000)/1000;
    if (scale <= 0) return true;
    if (!ctx->state(CTRL|SHIFT|ALT|META)
            && scale > .9 && scale < 1.1) scale = 1;
    double sx = ((corner&1)!=0) ? -scale : scale;
    double sy = sx * scale_ratio;
    k.jack_scale_x = sx;
    k.jack_scale_y = sy;
    k.jack_changed(ctx);
    return true;
}  

//  static
bool Grid_Knob::drag_scale_x(ViewerContext* ctx, Knob* p, int corner)
{
    if (ctx->event() != DRAG) return false;
    Grid_Knob &k = *(Grid_Knob*)p;
#if DD_IMAGE_VERSION_MAJOR < 5
    float x = p->from_proxy_x(ctx->x());
#else
    // changed to OutputContext object for proxy access in 5.0
    OutputContext kUiCtx = k.uiContext();
    float x = ctx->x();
    float y = ctx->y();
    kUiCtx.from_proxy_xy( x, y );
#endif
    double scale = x/(radius/dscale());
    scale = rint(scale*1000)/1000;
    if (corner) scale = -scale;
    double r = fabs(scale/k.jack_scale_y);
    if (!ctx->state(CTRL|SHIFT|ALT|META) && r > .9 && r < 1.1)
        scale = copysign(k.jack_scale_y, scale);
    k.jack_scale_x = scale;
    k.jack_changed(ctx);
    return true;
}	

//  static
bool Grid_Knob::drag_scale_y(ViewerContext* ctx, Knob* p, int corner)
{
    if (ctx->event() != DRAG) return false;
    Grid_Knob &k = *(Grid_Knob*)p;
    double scale = ctx->y()/(radius/dscale());
    scale = rint(scale*1000)/1000;
    if (corner) scale = -scale;
    double r = fabs(scale/k.jack_scale_x);
    if (!ctx->state(CTRL|SHIFT|ALT|META) && r > .9 && r < 1.1)
        scale = copysign(k.jack_scale_x, scale);
    k.jack_scale_y = scale;
    k.jack_changed(ctx);
    return true;
}

//  static
bool Grid_Knob::drag_center(ViewerContext* ctx, Knob* p, int)
{
    Grid_Knob &k = *(Grid_Knob*)p;
#if DD_IMAGE_VERSION_MAJOR < 5
#else
    // changed to OutputContext object for proxy access in 5.0
    OutputContext kUiCtx = k.uiContext();
#endif
    if (ctx->event() == DROP) {
        if (strncmp(ctx->key_text(), ":nuke:values:", 13)==0) {
            float x = k.jack_tx+k.jack_cx, y = k.jack_ty+k.jack_cy;
            istringstream in(ctx->key_text()+13);
            int n = 0; in >> n;
            if (n>0) in >> x;
            if (n>1) in >> y;
            float dx = x-(k.jack_tx+k.jack_cx), dy = y-(k.jack_ty+k.jack_cy);
            for (int i=0; i<k.cooked_grid.num_points(); i++) {
                if (k.is_selected(ctx, xy_cb, i<<3)) {
                    MPoint &p = k.cooked_grid.points[i];	
                    p.x += dx;
                    p.y += dy;
                }
            }
            k.jack_tx += dx; k.jack_ty += dy; 
#if DD_IMAGE_VERSION_MAJOR < 5
            k.maybe_set_key(k.frame());
#else
            k.maybe_set_key();
#endif
        } else if (strncmp(ctx->key_text(), ":nuke:keyframes:", 16)==0) {
#if DD_IMAGE_VERSION_MAJOR < 5
            float currentframe = k.frame();
#else
            // changed to OutputContext object for proxy access in 5.0
            float currentframe = kUiCtx.frame();
#endif
            istringstream in(ctx->key_text()+16);
            int n = 0; in >> n;
            float dxi = 0, dyi = 0;
            for (int i=0; i<n; i++) {
                int ix, nkey; in >> ix >> nkey;
                int readpos = in.tellg();
                int ki; for (ki=0; ki<nkey; ki++) {
                    double t, v; in >> t >> v;
                    if (ix==0 || ix==1) {
                        k.goto_frame(t);
                        k.set_key_at(t);
                    }
                }
                in.seekg(readpos);
                for (ki=0; ki<nkey; ki++) {
                    double t, v; in >> t >> v;
                    if (ix==0 || ix==1) {
                        k.goto_frame(t);
                        float x = k.jack_tx+k.jack_cx, y = k.jack_ty+k.jack_cy;
                        if (ix==0) x = v; 
                        if (ix==1) y = v;
                        float dx = x-(k.jack_tx+k.jack_cx), dy = y-(k.jack_ty+k.jack_cy);
                        if (t==currentframe && ix==0) dxi = dx; 
                        if (t==currentframe && ix==1) dyi = dy;
                        for (int i=0; i<k.cooked_grid.num_points(); i++) {
                            if (k.is_selected(ctx, xy_cb, i<<3)) {
                                MPoint &p = k.cooked_grid.points[i];	
                                p.x += dx;
                                p.y += dy;
                            }
                        }
                        k.set_key_at(t);
                    }
                }
            }
            k.jack_tx += dxi; k.jack_ty += dyi; 
        } else return false;
        k.changed();
        return true;
    }
    if (ctx->event() != DRAG) return false;
#if DD_IMAGE_VERSION_MAJOR < 5
    float x = p->from_proxy_x(ctx->x());
    float y = p->from_proxy_y(ctx->y());
#else
    // changed to OutputContext object for proxy access in 5.0
    float x = ctx->x();
    float y = ctx->y();
    kUiCtx.from_proxy_xy( x, y );
#endif
    double dx = x-k.jack_cx;
    double dy = y-k.jack_cy;
    if (!dx && !dy) return false;
    k.jack_cx = x;
    k.jack_cy = y;
    Matrix4 m;
    m.scaling(1/k.pixel_aspect, 1, 1);
    m.rotate((k.jack_rotate+k.jack_irotate)*M_PI_2/90);
    m.skew(k.jack_skew);
    m.scale(k.jack_scale_x, k.jack_scale_y, 1.0);
    m.rotate(-k.jack_irotate*M_PI_2/90);
    m.scale(k.pixel_aspect, 1, 1);
    m.translate(dx, dy);
    k.jack_tx += m.a03-dx;
    k.jack_ty += m.a13-dy;
    return true;
}

void Grid_Knob::draw_xform_jack(ViewerContext *ctx)
{
    double prx, pry;
    // count the selected points in the grid
    int i, nSelected = 0;
    bool mmfound = false;
    float cx = 0.0f, cy = 0.0f, maxx=0.0f, maxy = 0.0f;
#if DD_IMAGE_VERSION_MAJOR < 5
#else
    // changed to OutputContext object for proxy access in 5.0
    OutputContext uiCtx = uiContext();
#endif
    for (i=0; i<cooked_grid.num_points(); i++) {
        if (is_selected(ctx, xy_cb, i<<3)) {
            MPoint &p = cooked_grid.points[i];
            if (mmfound) {
                if (p.x<cx) cx = p.x;  if (p.x>maxx) maxx = p.x;
                if (p.y<cy) cy = p.y;  if (p.y>maxy) maxy = p.y;
            } else {
                cx = maxx = p.x; cy = maxy = p.y;
                mmfound = true;
            }
            nSelected++;
        }
    }
    if (nSelected<2) { prevSelected = 0; return; }
    cx = (cx+maxx)*0.5f; cy = (cy+maxy)*0.5f;
    // reset the jack
    if (nSelected!=prevSelected) {
        jack_cx = cx; jack_cy = cy;
        jack_tx = 0.0f; jack_ty = 0.0f; jack_irotate = jack_rotate = 0;
        jack_skew = 0.0f; jack_scale_x = jack_scale_y = 1.0;
        cooked_grid.copy_to(&jack_grid);
        prevSelected = nSelected;
    }
    // draw the jack
    Matrix4 saved_matrix(ctx->modelmatrix);
    double x = jack_tx, y = jack_ty;
#if DD_IMAGE_VERSION_MAJOR < 5
    prx = to_proxy_x(jack_cx); pry = to_proxy_y(jack_cy);
#else
    prx = jack_cx;
    pry = jack_cy;
    uiCtx.to_proxy_xy( prx, pry );
#endif
    ctx->modelmatrix.translate(prx, pry);
    // translation handle
#if DD_IMAGE_VERSION_MAJOR < 5
    prx = to_proxy_x(x); pry = to_proxy_y(y);
#else
    // changed to OutputContext object for proxy access in 5.0
    prx = x;
    pry = y;
    uiCtx.to_proxy_xy( prx, pry );
#endif
    if (!(ctx->event()==PUSH && control)) {
        glLoadMatrixf(ctx->modelmatrix.array());
        make_handle(ctx, drag_translate, 0, prx, pry);
    }
    ctx->modelmatrix.translate(prx, pry);
#if DD_IMAGE_VERSION_MAJOR < 5
    prx = to_proxy_x(1/pixel_aspect); pry = to_proxy_y(1);
#else
    // changed to OutputContext object for proxy access in 5.0
    prx = 1 / pixel_aspect;
    pry = 1;
    uiCtx.to_proxy_xy( prx, pry );
#endif
    ctx->modelmatrix.scale(prx, pry);
    ctx->modelmatrix.rotate(jack_irotate*M_PI_2/90);
    glLoadMatrixf(ctx->modelmatrix.array());
    // draw rotation handle:
    float r = handle_length*jack_scale_x/dscale();
    x = r*cos(jack_rotate*M_PI_2/90.0);
    y = r*sin(jack_rotate*M_PI_2/90.0);
    begin_handle(DISTANCE_FROM_POINT, ctx, drag_rotate, 0);
    glBegin(GL_LINES); glVertex2d(0,0); glVertex2d(x,y); glEnd();
    end_handle(ctx);
    // rotate:
    ctx->modelmatrix.rotate(jack_rotate*M_PI_2/90);
    glLoadMatrixf(ctx->modelmatrix.array());
    // draw skew handle:
    r = skew_length*jack_scale_y/dscale();
    if (fabs(r*ctx->zoom()) > 3) {
        x = r*jack_skew;
        y = r;
        begin_handle(DISTANCE_FROM_POINT, ctx, drag_skew, 0);
        glBegin(GL_LINES); glVertex2d(-x,-y); glVertex2d(x,y); glEnd();
        end_handle(ctx);
    }
    // skew:
    ctx->modelmatrix.skew(jack_skew);
    glLoadMatrixf(ctx->modelmatrix.array());
    // draw scaling circle:
    r = radius/dscale();
    begin_handle(DISTANCE_FROM_POINT, ctx, drag_scale,
            (jack_scale_x<0?1:0)+(jack_scale_y<0?2:0));
    glScalef(jack_scale_x, jack_scale_y, 1);
    gl_circlef(0, 0, 0, 2*r, XY, false);
    end_handle(ctx);
    glLoadMatrixf(ctx->modelmatrix.array());
    // draw non-uniform scaling boxes:
    if (fabs(r*jack_scale_y*ctx->zoom()) > 3) {
        make_handle(ctx, drag_scale_y,0,0,r*jack_scale_y);
        make_handle(ctx, drag_scale_y,1,0,-r*jack_scale_y);
    }
    if (fabs(r*jack_scale_x*ctx->zoom()) > 3) {
        make_handle(ctx, drag_scale_x,0,r*jack_scale_x,0);
        make_handle(ctx, drag_scale_x,1,-r*jack_scale_x,0);
    }
    // draw the draggable central point:
    if (ctx->event()==PUSH && control) {
        ctx->modelmatrix.scale(jack_scale_x, jack_scale_y, 1);
        ctx->modelmatrix.rotate(-jack_irotate*M_PI_2/90);
        ctx->modelmatrix.scale(pixel_aspect, 1, 1);
        ctx->modelmatrix.translate(-jack_cx, -jack_cy, 0);
        glLoadMatrixf(ctx->modelmatrix.array());
        make_handle(ctx, drag_center, 0, jack_cx, jack_cy);
    }
    ctx->modelmatrix = saved_matrix;
    glLoadMatrixf(ctx->modelmatrix.array());
}

void Grid_Knob::draw_handle_remove(ViewerContext *ctx)
{
    int x, y;
    if (!ctx->draw_knobs()) return;
    if (!this->name()) return;
#if DD_IMAGE_VERSION_MAJOR < 5
#else
    // changed to OutputContext object for proxy access in 5.0
    OutputContext uiCtx = uiContext();
#endif
    // draw colored lines into the mesh
    if (ctx->event()==DRAW_SHADOW) glColor(0);
    else glColor(0xff607000); // bright red for delete mode

    for (y=0; y<cooked_grid.height; y++) {
        begin_handle(ctx, remove_cb, (y<<1)|1, 0, 0, 0);
        glBegin(GL_LINE_STRIP);
#if DD_IMAGE_VERSION_MAJOR < 5
        const float x1 = to_proxy_x(cooked_grid.at(0, y).x);
        const float y1 = to_proxy_y(cooked_grid.at(0, y).y);
#else
        // changed to OutputContext object for proxy access in 5.0
        float x1 = cooked_grid.at( 0, y ).x;
        float y1 = cooked_grid.at( 0, y ).y;
	uiCtx.to_proxy_xy( x1, y1 );
#endif
        glVertex2f(x1, y1);
        for (x=0; x<cooked_grid.width-1; x++) {
#if DD_IMAGE_VERSION_MAJOR < 5
            const float x1 = to_proxy_x(cooked_grid.at(x, y).x);
            const float y1 = to_proxy_y(cooked_grid.at(x, y).y);
            const float x2 = x1+to_proxy_w(cooked_grid.at(x, y).ex);
            const float y2 = y1+to_proxy_h(cooked_grid.at(x, y).ey);
            const float x4 = to_proxy_x(cooked_grid.at(x+1, y).x);
            const float y4 = to_proxy_y(cooked_grid.at(x+1, y).y);
            const float x3 = x4+to_proxy_w(cooked_grid.at(x+1, y).wx);
            const float y3 = y4+to_proxy_h(cooked_grid.at(x+1, y).wy);
#else
            // changed to OutputContext object for proxy access in 5.0
            float x1 = cooked_grid.at( x, y ).x;
            float y1 = cooked_grid.at( x, y ).y;
	    uiCtx.to_proxy_xy( x1, y1 );
            float ex = cooked_grid.at( x, y ).ex;
            float ey = cooked_grid.at( x, y ).ey;
	    uiCtx.to_proxy_wh( ex, ey );
            float x2 = x1 + ex;
            float y2 = y1 + ey;
            float x4 = cooked_grid.at( x+1, y ).x;
            float y4 = cooked_grid.at( x+1, y ).y;
	    uiCtx.to_proxy_xy( x4, y4 );
            float wx = cooked_grid.at( x+1, y ).wx;
            float wy = cooked_grid.at( x+1, y ).wy;
	    uiCtx.to_proxy_wh( wx, wy );
            float x3 = x4 + wx;
            float y3 = y4 + wy;
#endif
            gl_bezierf(x1,y1, x2,y2, x3,y3, x4,y4, ctx->zoom());
        }
        glEnd();
        end_handle(ctx);
    }
    for (x=0; x<cooked_grid.width; x++) {
        begin_handle(ctx, remove_cb, (x<<1), 0, 0, 0);
        glBegin(GL_LINE_STRIP);
#if DD_IMAGE_VERSION_MAJOR < 5
        const float x1 = to_proxy_x(cooked_grid.at(x, 0).x);
        const float y1 = to_proxy_y(cooked_grid.at(x, 0).y);
#else
        // changed to OutputContext object for proxy access in 5.0
        float x1 = cooked_grid.at( x, 0 ).x;
        float y1 = cooked_grid.at( x, 0 ).y;
	uiCtx.to_proxy_xy( x1, y1 );
#endif
        glVertex2f(x1, y1);
        for (y=0; y<cooked_grid.height-1; y++) {
#if DD_IMAGE_VERSION_MAJOR < 5
            const float x1 = to_proxy_x(cooked_grid.at(x, y).x);
            const float y1 = to_proxy_y(cooked_grid.at(x, y).y);
            const float x2 = x1+to_proxy_w(cooked_grid.at(x, y).nx);
            const float y2 = y1+to_proxy_h(cooked_grid.at(x, y).ny);
            const float x4 = to_proxy_x(cooked_grid.at(x, y+1).x);
            const float y4 = to_proxy_y(cooked_grid.at(x, y+1).y);
            const float x3 = x4+to_proxy_w(cooked_grid.at(x, y+1).sx);
            const float y3 = y4+to_proxy_h(cooked_grid.at(x, y+1).sy);
#else
            // changed to OutputContext object for proxy access in 5.0
            float x1 = cooked_grid.at( x, y).x;
            float y1 = cooked_grid.at( x, y).y;
	    uiCtx.to_proxy_xy( x1, y1 );
            float nx = cooked_grid.at( x, y ).nx;
            float ny = cooked_grid.at( x, y ).ny;
	    uiCtx.to_proxy_wh( nx, ny );
            float x2 = x1 + nx;
            float y2 = y1 + ny;
            float x4 = cooked_grid.at( x, y+1 ).x;
            float y4 = cooked_grid.at( x, y+1 ).y;
	    uiCtx.to_proxy_xy( x4, y4 );
            float sx = cooked_grid.at( x, y+1 ).sx;

            float sy = cooked_grid.at( x, y+1 ).sy;
	    uiCtx.to_proxy_wh( sx, sy );
            float x3 = x4 + sx;
            float y3 = y4 + sy;
#endif
            gl_bezierf(x1,y1, x2,y2, x3,y3, x4,y4, ctx->zoom());
        }
        glEnd();
        end_handle(ctx);
    }
}

//  static
bool Grid_Knob::remove_cb(ViewerContext* ctx, Knob* data, int index)
{
    if (ctx->event()!=PUSH) return true;
    return ((Grid_Knob*)data)->remove_handle(ctx, index);
}

bool Grid_Knob::remove_handle(ViewerContext* ctx, int index)
{
    remove_curve(index);
    if (partner_)
        partner_->remove_curve(index);
    mode = MODE_EDIT;
    changed();
    return true;
}

float Grid_Knob::find_fraction(int index, float mx, float my)
{
    int  x = ((index>>16)&0x7fff)-1, y = ((index>>1)&0x7fff)-1;
    float x1, y1, x2, y2, x3, y3, x4, y4;
    if (index&1) { // add a y curve (horizontal line)
        x1 = cooked_grid.at(x, y).x, y1 = cooked_grid.at(x, y).y; 
        x2 = cooked_grid.at(x, y).nx, y2 = cooked_grid.at(x, y).ny; 
        x4 = cooked_grid.at(x, y+1).x, y4 = cooked_grid.at(x, y+1).y; 
        x3 = cooked_grid.at(x, y+1).sx, y3 = cooked_grid.at(x, y+1).sy; 
    } else { // add an x curve (vertical line)
        x1 = cooked_grid.at(x, y).x, y1 = cooked_grid.at(x, y).y; 
        x2 = cooked_grid.at(x, y).ex, y2 = cooked_grid.at(x, y).ey; 
        x4 = cooked_grid.at(x+1, y).x, y4 = cooked_grid.at(x+1, y).y; 
        x3 = cooked_grid.at(x+1, y).wx, y3 = cooked_grid.at(x+1, y).wy; 
    }
    float dm = 0.01f, dd = 10000.0f, mm = 0.5f;
    for (float m=0.0f+dm; m<1.0f; m+=dm) {
        float px = mx-bezier_at(m, x1, x2, x3, x4), 
              py = my-bezier_at(m, y1, y2, y3, y4);
        float d = sqrt(px*px+py*py);
        if (d<dd) { dd = d; mm = m; }
    }
    return mm;
}

void Grid_Knob::draw_handle_add(ViewerContext *ctx)
{
    int x, y;
    if (!ctx->draw_knobs()) return;
    if (!this->name()) return;
#if DD_IMAGE_VERSION_MAJOR < 5
#else
    // changed to OutputContext object for proxy access in 5.0
    OutputContext uiCtx = uiContext();
#endif
    // draw colored lines into the mesh
    if (ctx->event()==DRAW_SHADOW) glColor(0);
    else glColor(0xffffff00); // bright white for adding mode

    for (y=0; y<cooked_grid.height; y++) {
        for (x=0; x<cooked_grid.width-1; x++) {
            begin_handle(Knob::DISTANCE_FROM_POINT, ctx, add_cb, 
                    ((x+1)<<16)|((y+1)<<1), 0, 0, 0);
            glBegin(GL_LINE_STRIP);
#if DD_IMAGE_VERSION_MAJOR < 5
            const float x1 = to_proxy_x(cooked_grid.at(x, y).x);
            const float y1 = to_proxy_y(cooked_grid.at(x, y).y);
            const float x2 = x1+to_proxy_w(cooked_grid.at(x, y).ex);
            const float y2 = y1+to_proxy_h(cooked_grid.at(x, y).ey);
            const float x4 = to_proxy_x(cooked_grid.at(x+1, y).x);
            const float y4 = to_proxy_y(cooked_grid.at(x+1, y).y);
            const float x3 = x4+to_proxy_w(cooked_grid.at(x+1, y).wx);
            const float y3 = y4+to_proxy_h(cooked_grid.at(x+1, y).wy);
#else
            // changed to OutputContext object for proxy access in 5.0
            float x1 = cooked_grid.at( x, y ).x;
            float y1 = cooked_grid.at( x, y ).y;
	    uiCtx.to_proxy_xy( x1, y1 );
            float ex = cooked_grid.at( x, y ).ex;
            float ey = cooked_grid.at( x, y ).ey;
	    uiCtx.to_proxy_wh( ex, ey );
            float x2 = x1 + ex;
            float y2 = y1 + ey;
            float x4 = cooked_grid.at( x+1, y ).x;
            float y4 = cooked_grid.at( x+1, y ).y;
	    uiCtx.to_proxy_xy( x4, y4 );
            float wx = cooked_grid.at( x+1, y ).wx;
            float wy = cooked_grid.at( x+1, y ).wy;
	    uiCtx.to_proxy_wh( wx, wy );
            float x3 = x4 + wx;
            float y3 = y4 + wy;
#endif
            glVertex2f(x1, y1);
            gl_bezierf(x1,y1, x2,y2, x3,y3, x4,y4, ctx->zoom());
            glEnd();
            end_handle(ctx);
        }
    }
    for (x=0; x<cooked_grid.width; x++) {
        for (y=0; y<cooked_grid.height-1; y++) {
            begin_handle(Knob::DISTANCE_FROM_POINT, ctx, add_cb, 
                    ((x+1)<<16)|((y+1)<<1)|1, 0, 0, 0);
            glBegin(GL_LINE_STRIP);
#if DD_IMAGE_VERSION_MAJOR < 5
            const float x1 = to_proxy_x(cooked_grid.at(x, y).x);
            const float y1 = to_proxy_y(cooked_grid.at(x, y).y);
            const float x2 = x1+to_proxy_w(cooked_grid.at(x, y).nx);
            const float y2 = y1+to_proxy_h(cooked_grid.at(x, y).ny);
            const float x4 = to_proxy_x(cooked_grid.at(x, y+1).x);
            const float y4 = to_proxy_y(cooked_grid.at(x, y+1).y);
            const float x3 = x4+to_proxy_w(cooked_grid.at(x, y+1).sx);
            const float y3 = y4+to_proxy_h(cooked_grid.at(x, y+1).sy);
#else
            // changed to OutputContext object for proxy access in 5.0
            float x1 = cooked_grid.at( x, y ).x;
            float y1 = cooked_grid.at( x, y ).y;
	    uiCtx.to_proxy_xy( x1, y1 );
            float nx = cooked_grid.at( x, y ).nx;
            float ny = cooked_grid.at( x, y ).ny;
	    uiCtx.to_proxy_wh( nx, ny );
            float x2 = x1 + nx;
            float y2 = y1 + ny;
            float x4 = cooked_grid.at( x, y+1 ).x;
            float y4 = cooked_grid.at( x, y+1 ).y;
	    uiCtx.to_proxy_xy( x4, y4 );
            float sx = cooked_grid.at( x, y+1 ).sx;
            float sy = cooked_grid.at( x, y+1 ).sy;
	    uiCtx.to_proxy_wh( sx, sy );
            float x3 = x4 + sx;
            float y3 = y4 + sy;
#endif
            glVertex2f(x1, y1);
            gl_bezierf(x1,y1, x2,y2, x3,y3, x4,y4, ctx->zoom());
            glEnd();
            end_handle(ctx);
        }
    }
}

//  static
bool Grid_Knob::add_cb(ViewerContext* ctx, Knob* data, int index)
{
    if (ctx->event()!=PUSH) return true;
    return ((Grid_Knob*)data)->add_handle(ctx, index);
}

bool Grid_Knob::add_handle(ViewerContext* ctx, int index)
{
#if DD_IMAGE_VERSION_MAJOR < 5
    float mx = from_proxy_x(ctx->x());
    float my = from_proxy_y(ctx->y());
#else
    // changed to OutputContext object for proxy access in 5.0
    float mx = ctx->x();
    float my = ctx->y();
    uiContext().from_proxy_xy( mx, my );
#endif
    float f = find_fraction(index, mx, my);
    add_curve(index, f);
    if (partner_)
        partner_->add_curve(index, f);
    mode = MODE_EDIT;
#if DD_IMAGE_VERSION_MAJOR < 5
    maybe_set_key(frame());
#else
    // changed to OutputContext object for proxy access in 5.0
    maybe_set_key();
#endif
    return true;
}

//  static
bool Grid_Knob::xy_cb(ViewerContext* ctx, Knob* data, int index)
{
    Grid_Knob& k = *(Grid_Knob*)data;

    switch (ctx->event()) {
    case PUSH:
	   if (ctx->button() > 2) goto DO_MENU;
# if __APPLE__
       if (ctx->button() == 1 && ctx->state(CTRL)) goto DO_MENU;
# endif
	    return false;
    case DRAG:
    case KEY:
        return k.move_handle(ctx, index);
    case DROP:
        return k.drop_handle(ctx, index);
    DO_MENU:
    default:
        ctx->menu(make_menu());
        return k.do_menu(ctx, index);
    }
}

bool Grid_Knob::drop_handle(ViewerContext* ctx, int index)
{
    int sub = index&7;
    if (sub!=0) return false; // allow only drag'n'drop onto main handles
    MPoint &p = cooked_grid.points[index>>3];
    if (strncmp(ctx->key_text(), ":nuke:values:", 13)==0) {
        istringstream in(ctx->key_text()+13);
        double v; int n = 0; in >> n;
        if (n>0) { in >> v; p.x = v; }
        if (n>1) { in >> v; p.y = v; }
#if DD_IMAGE_VERSION_MAJOR < 5
        maybe_set_key(frame());
#else
        // changed to OutputContext object for proxy access in 5.0
        maybe_set_key();
#endif
    } else if (strncmp(ctx->key_text(), ":nuke:keyframes:", 16)==0) {
        istringstream in(ctx->key_text()+16);
        int n = 0; in >> n;
        for (int i=0; i<n; i++) {
            int ix, nkey; in >> ix >> nkey;
            for (int ki=0; ki<nkey; ki++) {
                double t, v; in >> t >> v;
                if (ix==0 || ix==1) {
                    goto_frame(t);
                    if (ix==0) p.x = v; 
                    if (ix==1) p.y = v;
                    set_key_at(t);
                }
            }
        }
    } else return false;
#if DD_IMAGE_VERSION_MAJOR < 5
    maybe_set_key(frame());
#else
    // changed to OutputContext object for proxy access in 5.0
    maybe_set_key();
#endif
    return true;	
}

bool Grid_Knob::move_handle(ViewerContext* ctx, int index)
{
    MPoint &p = cooked_grid.points[index>>3];
    int sub = index&7;
    double x, y;
    switch (sub) {
        case 0: x = p.x; y = p.y; break;
        case 1: x = p.x+p.nx; y = p.y+p.ny; break;
        case 2: x = p.x+p.sx; y = p.y+p.sy; break;
        case 3: x = p.x+p.ex; y = p.y+p.ey; break;
        case 4: x = p.x+p.wx; y = p.y+p.wy; break;
    }

    if (ctx->event()==KEY) {
        if (!ctx->nudgeXY(x, y)) {
            return false;
        }
        new_nudge_undo();
    } else if (ctx->event() == DRAG) {
        x = ctx->x(); y = ctx->y();
    } else if ( ctx->event() == PUSH ) {
        new_undo();
        return false;
    } else if ( ctx->event() == RELEASE ) {
        return false;
    } else {
        return false;
    }
#if DD_IMAGE_VERSION_MAJOR < 5
    x = from_proxy_x(x);
    y = from_proxy_y(y);
#else
    // changed to OutputContext object for proxy access in 5.0
    uiContext().from_proxy_xy( x, y );
#endif

    bool  gang = (ctx->state() & SHIFT);
    bool rgang = !gang && !(ctx->state() & COMMAND);
    switch (sub) {
        case 0: 
            p.x = x; p.y = y; break;
        case 1: 
            p.nx = x-p.x; p.ny = y-p.y; 
            if (gang) { p.sx = -p.nx; p.sy = -p.ny; } 
            if (rgang) rot_gang(-p.nx, -p.ny, p.sx, p.sy);
            break;
        case 2: 
            p.sx = x-p.x; p.sy = y-p.y; 
            if (gang) { p.nx = -p.sx; p.ny = -p.sy; } 
            if (rgang) rot_gang(-p.sx, -p.sy, p.nx, p.ny);
            break;
        case 3: 
            p.ex = x-p.x; p.ey = y-p.y; 
            if (gang) { p.wx = -p.ex; p.wy = -p.ey; } 
            if (rgang) rot_gang(-p.ex, -p.ey, p.wx, p.wy);
            break;
        case 4: 
            p.wx = x-p.x; p.wy = y-p.y; 
            if (gang) { p.ex = -p.wx; p.ey = -p.wy; } 
            if (rgang) rot_gang(-p.wx, -p.wy, p.ex, p.ey);
            break;
    }
#if DD_IMAGE_VERSION_MAJOR < 5
    maybe_set_key(frame());
#else
    // changed to OutputContext object for proxy access in 5.0
    maybe_set_key();
#endif
    return true;
}

/*! This may be called from a knobs() function in an Iop to make a
  control that updates a Bezier. */
Grid_Knob* Grid_knob(Knob_Callback f, MGrid* p, const char* name, const char* shapename) {
#if DD_IMAGE_VERSION_MAJOR < 5
    if (Knob::Make_Knobs) return new Grid_Knob(p, name, shapename);
    return (Grid_Knob*)f(INT_KNOB, p, name, 0, 0);
#else
    if (f.makeKnobs()) {
        return new Grid_Knob( &f, p, name, shapename );
    }
    return (Grid_Knob*)f( (int)INT_KNOB, Custom, p, name, 0, 0 );
#endif
}

namespace DD { namespace GridWarp {

//==== MTri ===================================================================

/** Before rendering individual rows, GridWarp creates a precalculated list of 
 * triangles that describe the source and the destination mesh for quick access
 */
class MTri {
    public:
        float p0x, p0y, p1x, p1y, p2x, p2y;
        float q0x, q0y, q1x, q1y, q2x, q2y;
        float detq01, detq02, detq12;
        void set_src(float x0, float y0, float x1, float y1, float x2, float y2) {
            p0x = x0; p0y = y0; p1x = x1-x0; p1y = y1-y0; p2x = x2-x0; p2y = y2-y0; 
        }
        void set_dst(float x0, float y0, float x1, float y1, float x2, float y2) {
            q0x = x0; q0y = y0; q1x = x1-x0; q1y = y1-y0; q2x = x2-x0; q2y = y2-y0; 
            detq02 = det(q0x,q0y,q2x,q2y);
            detq01 = det(q0x,q0y,q1x,q1y);
            detq12 = det(q1x,q1y,q2x,q2y);
        }
};


//==== MRect ==================================================================

/** This class helps to convert a rectangular segment of the grid into 
 * many smaller ractangles that conform to the bezier outlines of the big rect.
 */
class MRect {

    public:

        MRect(int xr, int yr);
        virtual ~MRect();

        void set(int ix, int iy, float xv, float yv);
        void set(MPoint &p0, MPoint &p1, MPoint &p2, MPoint &p3);

        void get(int ix, int iy, float &rx, float &ry);

    private:

        int xres, yres;
        float *x, *y;

};

}} // namespace DD::GridWarp

MRect::MRect(int xr, int yr)
{
    xres = xr, yres = yr;
    x = new float[(xres+1)*(yres+1)*2];
    y = x + (xres+1)*(yres+1);
}

MRect::~MRect()
{
    delete[] x;
}

void MRect::set(int ix, int iy, float xv, float yv)
{
    int i = ix+(xres+1)*iy;
    x[i] = xv; y[i] = yv;
}

void MRect::set(MPoint &p0, MPoint &p1, MPoint &p2, MPoint &p3)
{
    int i, j;
    // set the corners
    set(0, 0, p0.x, p0.y);    set(xres, 0, p1.x, p1.y);
    set(0, yres, p2.x, p2.y); set(xres, yres, p3.x, p3.y);
    // now calculate all bezier edges
    for (i=1; i<xres; i++) {
        float f = i/(float)(xres);
        set(i,      0, 
                bezier_at(f, p0.x, p0.ex, p1.wx, p1.x),
                bezier_at(f, p0.y, p0.ey, p1.wy, p1.y) );
        set(i, yres, 
                bezier_at(f, p2.x, p2.ex, p3.wx, p3.x),
                bezier_at(f, p2.y, p2.ey, p3.wy, p3.y) );
    }
    // finally interpolate the internal coordinates
    for (i=1; i<yres; i++) {
        float f = i/(float)(yres);
        float p0x = bezier_at(f, p0.x, p0.nx, p2.sx, p2.x);
        float p0y = bezier_at(f, p0.y, p0.ny, p2.sy, p2.y);
        float p1x = bezier_at(f, p1.x, p1.nx, p3.sx, p3.x);
        float p1y = bezier_at(f, p1.y, p1.ny, p3.sy, p3.y);
        set(0, i, p0x, p0y);
        set(xres, i, p1x, p1y);
        float p0ex = bezier_at(f, p0.x+p0.ex, p0.nx, p2.sx, p2.x+p2.ex) - p0x;
        float p0ey = bezier_at(f, p0.y+p0.ey, p0.ny, p2.sy, p2.y+p2.ey) - p0y;
        float p1wx = bezier_at(f, p1.x+p1.wx, p1.nx, p3.sx, p3.x+p3.wx) - p1x;
        float p1wy = bezier_at(f, p1.y+p1.wy, p1.ny, p3.sy, p3.y+p3.wy) - p1y;
        for (j=1; j<xres; j++) {
            float g = j/(float)xres;
            set(j, i, 
                    bezier_at(g, p0x, p0ex, p1wx, p1x),
                    bezier_at(g, p0y, p0ey, p1wy, p1y) );
        }
    }
}

void MRect::get(int ix, int iy, float &rx, float &ry)
{
    int i = ix+(xres+1)*iy;
    rx = x[i]; ry = y[i];
}

//==== WarpGeo ===============================================================

#include <DDImage/NullGeo.h>
#include <DDImage/Mesh.h>

static const int maxRenderRes = 200;

class WarpGeo2 : public NullGeo {
    Iop* parent;
    Mesh* mmesh;

    public:
    MGrid *src, *dst; 	// storage for the source and destination grids
    MGrid* clipboard;   // storage for copy/past key
    int renderres;		// grids will be converted to a mesh with this number of subsections
    int mxres, myres; 	// total resolution of the mesh in x and y
    double distort[2];	// multiply the amount of distortion by this value
    bool reverse; 		// reverse the deformation

#if DD_IMAGE_VERSION_MAJOR < 5
    WarpGeo2(Iop* p) : NullGeo(), parent(p) {
#else
    WarpGeo2(Node* node, Iop* p) : NullGeo( node ), parent( p ) {
#endif
        mmesh = 0;
        src = dst = 0;
        clipboard = 0;
        renderres = 10;
        mxres = myres = 0;
        distort[0] = distort[1] = 1.0;
        reverse = false;
    }

    void copy_settings(WarpGeo2 *a) {
        src = a->src;
        dst = a->dst;
        renderres = a->renderres;
        mxres = a->mxres;
        myres = a->myres;
        distort[0] = 1.0 - a->distort[1];
        reverse = !a->reverse;
    }

    /* Let the geometry know if we need to create new primitives: */
    void get_geometry_hash() {
        geo_hash[Group_Primitives] = geo_hash[Group_Points] = Op::hash();
    }

    void geometry_engine(Scene& scene, GeometryList& out) {
        if (rebuild(Mask_Primitives)) {
            out.delete_objects();
            out.add_object(0);

            // Add a simple Mesh:
            int xres = MIN(renderres,maxRenderRes);
            int yres = xres;
            mxres = (src->width-1)*xres+1;
            myres = (src->height-1)*yres+1;
            mmesh = new Mesh(mxres, myres, false, false, QUADS);

            // Let the mesh generate its points:
            PointList* points = out.writable_points(0);
            mmesh->add_points(points, PLANE_XY);
            Attribute* uv = out.writable_attribute(0, Group_Points, "uv", VECTOR4_ATTRIB);
            mmesh->create_point_uvs(uv, 1.0f, 1.0f);

            // Add it to the cache:
            out.add_primitive(0, mmesh);

            // Force points to update:
            set_rebuild(Mask_Points);
        }

        if (rebuild(Mask_Points)) {
            PointList* points = out.writable_points(0);
            Attribute* uv = out.writable_attribute(0, Group_Points, "uv", VECTOR4_ATTRIB);
            assert(uv);

            static int c = 30;
            if (c-- == 0) {
                c = 1;
            }

            float distortion = distort[0];
            const Format& f = parent->full_size_format();
            float xs = 1.0f / f.width();
            float ys = 1.0f / f.height();
            int xres = MIN(renderres,maxRenderRes);
            int yres = xres;
            MRect srect(xres, yres);
            MRect drect(xres, yres);

            MGrid* src, *dst;
            if (reverse) {
                src = this->dst;
                dst = this->src;
            } else {
                src = this->src;
                dst = this->dst;
            }

            int xext=0, yext=0;
            for (int y=0; y<src->height-1; y++) {
                yext = (y==src->height-2);
                for (int x=0; x<src->width-1; x++) {
                    xext = (x==src->width-2);
                    srect.set(src->at(x, y), src->at(x+1, y),
                            src->at(x, y+1), src->at(x+1, y+1));
                    drect.set(dst->at(x, y), dst->at(x+1, y),
                            dst->at(x, y+1), dst->at(x+1, y+1));
                    for (int yr=0; yr<(yres+yext); yr++) {
                        int gx = x*xres;
                        int gy = y*yres + yr;
                        int ix = mmesh->vertex_index(gx, gy);
                        for (int xr=0; xr<(xres+xext); xr++, ix++) {
                            Vector3& p = (*points)[ix];
                            Vector4& t = uv->vector4(ix);
                            float px, py, tx, ty;
                            drect.get(xr, yr, px, py);
                            srect.get(xr, yr, tx, ty);
                            tx = (tx-px)*distortion+px;
                            ty = (ty-py)*distortion+py;
#if DD_IMAGE_VERSION_MAJOR < 5
                            p.x = parent->to_proxy_x(px);
                            p.y = parent->to_proxy_y(py);
#else
                            // changed to OutputContext object for proxy access in 5.0
                            p.x = px;
                            p.y = py;
			    parent->outputContext().to_proxy_xy( p.x, p.y );
#endif
                            t.x = tx*xs;
                            t.y = ty*ys;
                        }
                    }
                }
            }
        }

        if (out.size()) {
            GeoInfo& info = out[0];
            info.matrix.makeIdentity();
            info.material   = parent;
            info.display3d  = DISPLAY_TEXTURED;
            info.render_mode = RENDER_TEXTURED;
            info.selectable = true;
            info.selected   = false;
            info.source_geo = info.final_geo = info.select_geo = this;
        }
    }

    const char* Class() const {return "InternalWarpGeo";}
    const char* node_help() const {return "internal";}
};

//==== WarpMath =============================================================

/**
 * Grid based image warping operator (base class)
 */
class WarpMath : public Render {
    protected:
        WarpGeo2 warpgeo;			//!< Warp math

    public:
#if DD_IMAGE_VERSION_MAJOR < 5
        WarpMath() : Render(), warpgeo(this) {
#else
        WarpMath( Node* node ) : Render( node ), warpgeo( node, this ) {
#endif
            //mixA = 0; background = 0;
            warpgeo.src = new MGrid();
            warpgeo.src->init(4, 4);
            warpgeo.src->color = 0xffaaaa00;
            warpgeo.dst = new MGrid();
            warpgeo.dst->init(4, 4, 50.0f, 400.0f);
            warpgeo.dst->color = 0xaaaaff00;
        }

#if DD_IMAGE_VERSION_MAJOR < 5
        WarpMath(WarpMath *a) : Render(), warpgeo(this) {
#else
        WarpMath(WarpMath *a) : Render( 0 ), warpgeo( 0, this ) {
#endif
            parent(a);
            copy_settings(a);
            //mixA = 0; background = 0;
        } 
        virtual ~WarpMath() {}

        GeoOp* render_geo(int sample) {return &warpgeo;}

        void copy_settings(WarpMath *a) {
            warpgeo.copy_settings(&a->warpgeo);
            //renderBox = a->renderBox;
        }

        void _validate(bool for_real) {
            // Copy the base info from input0:
            copy_info();

            warpgeo.invalidate();

            Render::_validate(for_real);
        }

        void _request(int x, int y, int r, int t, ChannelMask m, int s) {
            Render::_request(x, y, r, t, m, s);
        }

        void engine(int y, int x, int r, ChannelMask channels, Row& out_row, Row* bg) {
            Render::initialize();
            Render::draw_primitives(y, x, r, channels, out_row, bg);
        }

        const char* Class() const {return CLASSM;}
        const char* node_help() const {return "internal";}
};


//==== WarpIop ================================================================

/**
 * Grid based image warping operator
 */
namespace DD {
    namespace GridWarp {

        class WarpIop : public WarpMath {

            private:

                WarpMath *dst_iop;
                float blend;
                /** storage for the source and destination grid */
                MGrid copypaste;
                bool _copyPasteValid;
                /** user interface representation of animated grids */
                Grid_Knob *kSrcGrid, *kDstGrid;
                /** various user settings */
                bool src_show, dst_show;
                bool src_lock, dst_lock;
                /** mix the warped image onto a background */
                float mixbg;
                /** select a background (black, src, dst, bg) */
                int background;
                Knob* track;
                int transform_grid, prev_transform_grid;
                // !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
	        //
	        // Major HACK.
	        //   When version 3 is loaded, both source and destination
	        //   do not have any transforms. The transform is then loaded
	        //   from the transform knob which got read after the grids.
	        //     0. call knobs
	        //     1. read grids
	        //     2. read which src/dst transform knob controls
	        //     3. read transform knob settings
	        //     4. call knobs
	        //     4a set transform for source or destination grid base
	        //        on #2 in the second call to knobs
	        //   If this is not done, the control panel will have to be
	        //   launched first to have the grid transforms set up in
	        //   knob_changed.
                //
                // !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
	        //
		// This variable helps us keep track of which knobs call it was.
		int knobs_call_count;

                void set_transform_grid(int n);

                virtual int knob_changed(Knob* k);
                virtual void knobs(Knob_Callback);
                virtual void _open();

                virtual void _validate(bool for_real);
                virtual void _request(int x, int y, int r, int t, ChannelMask m, int count);
                virtual void engine(int y, int x, int r, ChannelMask channels, Row& out);

            public:

#if DD_IMAGE_VERSION_MAJOR < 5
                WarpIop();
#else
                WarpIop( Node* node );
#endif
                virtual ~WarpIop() {}
                const char *input_label(int i, char *buffer) const;
                const char* node_help() const {return HELP;}
                const char* Class() const {return CLASS;}

                static const Iop::Description d;
        };
    }
}

void WarpIop::_validate(bool for_real)
{
    copy_info();
    merge_info(1);

    if (blend<1.0) {
        WarpMath::_validate(for_real);
    }
    if (blend>0.0) {
        if (!dst_iop) dst_iop = new WarpMath(this);
        dst_iop->set_input(0, input(1));
        dst_iop->copy_settings(this);
        dst_iop->invalidate();
        dst_iop->validate(for_real);
        if (blend<1.0) {
            info_.merge(dst_iop->info()); // merge changes by the dst_iop on top
        } else {
            info_.set(dst_iop->info()); // set to dst_iop
        }
        info_.turn_on(dst_iop->channels());
    }
    if (background) {
        merge_info(background-1);
    }

}

void WarpIop::_request(int x, int y, int r, int t, ChannelMask m, int count)
{
    if (blend<1.0) {
        WarpMath::_request(x, y, r, t, m, count);
    }
    if (blend>0.0) {
        dst_iop->_request(x, y, r, t, m, count);
    }
    if (background) {
        input(background-1)->request(x, y, r, t, m, count);
    }
}

void WarpIop::engine(int y, int x, int r, ChannelMask channels, Row& out)
{
    if (mixbg >= 1) {
        if (background)
            input(background-1)->get(y, x, r, channels, out);
        else
            out.erase(channels);
        return;
    }
    // first, read in the background image
    Row bg(x, r);
    if (background) {
        input(background-1)->get(y, x, r, channels, bg);
    }
    // now process the first distortion
    if (blend >= 1.0) {
        dst_iop->engine(y, x, r, channels, out, background ? &bg : 0);
    } else {
        WarpMath::engine(y, x, r, channels, out, background ? &bg : 0);
        if (blend > 0) {
            // we may need to blend both inputs. Src is already in 'out'
            Row dr(x, r);
            dr.copy(bg, channels, x, r);
            dst_iop->engine(y, x, r, channels, dr, background ? &bg : 0);
            float b1 = 1.0f-blend, b2 = blend;
            foreach(z, channels) {
                const float *s1 = out[z]+x, *e = s1+r-x, *s2 = dr[z]+x;
                float *dst = out.writable(z)+x;
                while (s1<e) {
                    *dst++ = b1 * *s1++ + b2 * *s2++;
                }
            }
        }
    }
    // now blend the bg over the whole deformation
    if (mixbg > 0) {
        float b1 = 1.0f-mixbg, b2 = mixbg;
        if (background) {
            foreach(z, channels) {
                const float *s1 = out[z]+x, *e = s1+r-x, *s2 = bg[z]+x;
                float *dst = out.writable(z)+x;
                while (s1<e)
                    *dst++ = b1 * *s1++ + b2 * *s2++;
            }
        } else {
            foreach(z, channels) {
                const float *s1 = out[z]+x, *e = s1+r-x;
                float *dst = out.writable(z)+x;
                while (s1<e)
                    *dst++ = b1 * *s1++;
            }
        }
    }
}

#if DD_IMAGE_VERSION_MAJOR < 5
WarpIop::WarpIop()
#else
WarpIop::WarpIop( Node* node ) : WarpMath( node )
#endif
{
    dst_iop = 0;
    blend = 0.0f;
    inputs(3);
    kSrcGrid = kDstGrid = 0;
    src_show = dst_show = true;
    dst_lock = src_lock = false;
    background = 0;
    mixbg = 0.0f;
    track = 0;
    transform_grid = 1;//source
    prev_transform_grid = -1;
    knobs_call_count = 0;
}

const char *WarpIop::input_label(int i, char *buffer) const
{ 
    static const char *lu[] = { "src", "dst", "bg" };
    if (i<0 || i>2) return 0;
    return lu[i]; 
}

void WarpIop::_open()
{
    Render::_open();
}

void WarpIop::set_transform_grid(int n) {
    if (n == 1) { //source
        kSrcGrid->transform_me = true;
        kDstGrid->transform_me = false;
	if (kSrcGrid->loading_pre_v4) {
	    kSrcGrid->xform.from_knob( kSrcGrid->track );
            kSrcGrid->loading_pre_v4 = false;
            kDstGrid->loading_pre_v4 = false;
	}
	else {
            kSrcGrid->xform.to_knob( track );
	}
    } else if (n == 2) {//dest
        kSrcGrid->transform_me = false;
        kDstGrid->transform_me = true;
        // !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
	//
	// Major HACK.
	//   When version 3 is loaded, both source and destination
	//   do not have any transforms. The transform is then loaded
	//   from the transform knob which got read after the grids.
	//     0. call knobs
	//     1. read grids
	//     2. read which src/dst transform knob controls
	//     3. read transform knob settings
	//     4. call knobs
	//     4a set transform for source or destination grid base
	//        on #2 in the second call to knobs
	//   If this is not done, the control panel will have to be
	//   launched first to have the grid transforms set up in
	//   knob_changed.
        //
        // !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
	//
	// This part actually load up the destination transform after
	//   from_script. It was defaulted to source
	if (kDstGrid->loading_pre_v4 && prev_transform_grid == -1) {
	    // recover dst grid transform from knob
	    kDstGrid->xform.from_knob( kDstGrid->track );
	    // set src grid transform to identity
	    kSrcGrid->xform.make_identity();
            kSrcGrid->loading_pre_v4 = false;
            kDstGrid->loading_pre_v4 = false;
	}
        else {
            kDstGrid->xform.to_knob( kDstGrid->track );
        }
    } else if (n == 3) {//both
        kSrcGrid->transform_me = true;
        kDstGrid->transform_me = true;
	kSrcGrid->loading_pre_v4 = false;
	kDstGrid->loading_pre_v4 = false;
    } else {
        kSrcGrid->transform_me = false;
        kDstGrid->transform_me = false;
	kSrcGrid->loading_pre_v4 = false;
	kDstGrid->loading_pre_v4 = false;
    }
    kSrcGrid->transform_dirty = true;
    kDstGrid->transform_dirty = true;
    prev_transform_grid = transform_grid;
}


static const char* transform_grid_names[] = {"none", "source", "destination", "both", 0};

static char *backgroundList[] = { 
    "on black", "on src", "on dst", "on bg", 0 
};

void WarpIop::knobs(Knob_Callback f) {
    Divider(f, "@b;Source Grid  ");
    Bool_knob(f, &src_show, "src_show", "show");
    SetFlags(f, Knob::NO_ANIMATION|Knob::NO_RERENDER);
    Tooltip(f, "If checked, the source grid lines and handles will not be displayed.");  
    Bool_knob(f, &src_lock, "src_lock", "lock");
    SetFlags(f, Knob::NO_ANIMATION|Knob::NO_RERENDER);
    Tooltip(f, "If checked, the source grid handles cannot be moved (and will not be displayed).");  
    kSrcGrid = Grid_knob(f, warpgeo.src, "srcgrid", "srcshape");
    Newline(f, "clipboard");
    Script_knob(f, " ", "src_copy", " copy  ");
    Tooltip(f, "Copy the current source grid into a temporary buffer. "
                "The source and destination grids share the same temporary buffer.");
    Script_knob(f, " ", "src_paste", " paste ");
    Tooltip(f, "Copy the temporary buffer grid into the destination grid.");
    Newline(f, "grid curve");
    Script_knob(f, " ", "src_add_c", "  add   ");
    Tooltip(f, "Add a Bezier curve to both grids by clicking on a line in the "
                "source grid.");  
    Script_knob(f, " ", "src_remove_c", " remove ");
    Tooltip(f, "Remove a Bezier curve from both grids by clicking on a line in "
                "the source grid.");  
    Newline(f, "presets");
    Script_knob(f, " ", "src_preset_input", "image size");
    Tooltip(f, "Initialize the source grid to the size of the input image");  


    Divider(f, "@b;Destination Grid  ");
    Bool_knob(f, &dst_show, "dst_show", "show");
    SetFlags(f, Knob::NO_ANIMATION|Knob::NO_RERENDER);
    Tooltip(f, "If checked, the destination grid lines and handles will not be displayed.");
    Bool_knob(f, &dst_lock, "dst_lock", "lock");
    SetFlags(f, Knob::NO_ANIMATION|Knob::NO_RERENDER);
    Tooltip(f, "If checked, the destination grid handles cannot be moved (and will not be displayed).");  
    kDstGrid = Grid_knob(f, warpgeo.dst, "dstgrid", "dstshape");
    Newline(f, "clipboard");
    Script_knob(f, " ", "dst_copy", " copy  ");
    Tooltip(f, "Copy the current destination grid into a temporary buffer. "
                "The source and destination grids share the same temporary buffer.");
    Script_knob(f, " ", "dst_paste", " paste ");
    Tooltip(f, "Copy the temporary buffer grid into the destination grid.");
    Newline(f, "grid curve");
    Script_knob(f, " ", "dst_add_c", "  add   ");
    Tooltip(f, "Add a Bezier curve to both grids by clicking on a line in the "
                "destination grid.");  
    Script_knob(f, " ", "dst_remove_c", " remove ");
    Tooltip(f, "Remove a Bezier curve from both grids by clicking on a line in "
                "the destination grid.");  
    Newline(f, "presets");
    Script_knob(f, " ", "dst_preset_input", "image size");
    Tooltip(f, "Initialize the destination grid to the size of the input image");  

    kSrcGrid->partner(kDstGrid);
    kDstGrid->partner(kSrcGrid);

    Divider(f, "@b;Settings  ");
    Obsolete_knob(f, "autokey", 0);

    Bool_knob(f, &warpgeo.reverse, "reverse", "reverse");
    Int_knob(f, &warpgeo.renderres, "renderres", "submesh resolution");
    Tooltip(f, "Sets the number of subdivisions that are created between "
                "Bezier curves in the grid.\n@i;tcl: renderres");

    Divider(f, "@b;Output  ");
    Scale_knob(f, warpgeo.distort, IRange(0.0, 1.0), "distortion");
    Tooltip(f, "Multiply the overall distortion by this amount. "
        		  "Source and destination distortion can be set seperately.");
    Float_knob(f, &blend, IRange(0.0, 1.0), "blend");
    Tooltip(f, "Blend between the source and destination images.");
    Enumeration_knob(f, &background, backgroundList, "background");
    Tooltip(f, "Render the warped image on top of one of these unwarped backgrounds");
    Float_knob(f, &mixbg, "mixBG", "background blend");
    Tooltip(f, "Mix the warped input with the unwarped background");			 

    //Divider(f, "@b;Preview  "); // tab deleted because display_knobs is blank!
    display_knobs(f);

    Tab_knob(f, 0, "Render");
    render_knobs(f);

    Tab_knob( f, "Tracking" );
	Enumeration_knob(f, &transform_grid, transform_grid_names, "transform_grid", "grid");
    track = Transform2d_knob( f, 0, "track", TO_PROXY );
    kSrcGrid->set_track( track );
    kDstGrid->set_track( track );

    // Since we can only have a single transform knob per node, the
    // following hidden knob will be used to evaluate per-point expressions
    // (see Transform::evaluate) It gets very dangerously close to a hack
    // but I made it as elegant as possible.
#if DD_IMAGE_VERSION_MAJOR < 5
    XY_knob( f, 0, "evaluate" );
#else
    XY_knob( f, (double*)0, "evaluate" );
#endif
    SetFlags( f,
        Knob::NO_RERENDER | Knob::NO_HANDLES | Knob::NO_KNOB_CHANGED
        | Knob::NO_UNDO | Knob::DO_NOT_WRITE | Knob::DISABLED
        | Knob::INVISIBLE );

#ifdef DEBUG_TEST_KNOBS
    // Testing and debugging knobs {{
    int debug_flags =
        Knob::NO_RERENDER | Knob::NO_HANDLES
        | Knob::NO_UNDO | Knob::DO_NOT_WRITE
        ;
    BeginGroup( f, "Developer debugging and testing" );
        Newline( f );
            Script_knob( f, " ", "print_transform", " print transform ");
            SetFlags( f, debug_flags );
            Script_knob( f, " ", "print_src_xform", " print src transform ");
            SetFlags( f, debug_flags );
            Script_knob( f, " ", "print_dst_xform", " print dst transform ");
            SetFlags( f, debug_flags );
        Newline( f );
            Script_knob( f, " ", "print_src_knob", " source script " );
            Script_knob( f, " ", "print_dst_knob", " destination script " );
    EndGroup( f );
    // Testing and debugging knobs }}
#endif

    // !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
    //
    // Major HACK.
    //   When version 3 is loaded, both source and destination
    //   do not have any transforms. The transform is then loaded
    //   from the transform knob which got read after the grids.
    //     0. call knobs
    //     1. read grids
    //     2. read which src/dst transform knob controls
    //     3. read transform knob settings
    //     4. call knobs
    //     4a set transform for source or destination grid base
    //        on #2 in the second call to knobs
    //   If this is not done, the control panel will have to be
    //   launched first to have the grid transforms set up in
    //   knob_changed.
    //
    // !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
    //
    // This is where the deed is done. It is very specific as for the first
    //   conditional with the second call to knobs and from_script has just
    //   been called by source and destination grids
    if (knobs_call_count == 1 &&
        kSrcGrid->loading_pre_v4 && kDstGrid->loading_pre_v4) {
        set_transform_grid(transform_grid);
        kSrcGrid->loading_pre_v4 = false;
        kDstGrid->loading_pre_v4 = false;
	// we're done here with version 3 from_script problem
	knobs_call_count = -1;
    }
    // The above HACK is turned off once from_script is over and handled
    if (knobs_call_count >= 0) {
        knobs_call_count++;
    }
}

int WarpIop::knob_changed(Knob* k)
{ 
#if DD_IMAGE_VERSION_MAJOR < 5 || \
    (DD_IMAGE_VERSION_MAJOR == 5 && DD_IMAGE_VERSION_MINOR < 1)
    const char *name = k->name();
#else
    const char *name = k->name().data();
#endif

    // !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
    //
    // Major HACK.
    //   For animation curves, Grid_Knob::goto_shape is called many time
    //   before src/dst are set up right causing scripted transforms to
    //   be overloaded by the transform knob.
    //   This is the only safe place to do this, ie no new animation
    //   before show panel is called anyway.
    //
    // !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
    //
    if (strcmp(name, "INT_showPanel"))
    {
        kSrcGrid->just_been_read = false;
        kDstGrid->just_been_read = false;
    }
    // !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
    //
    // End of hack
    //
    // !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!

    if ( !kSrcGrid || !kDstGrid ) return 1;

    if (transform_grid != prev_transform_grid) {
        set_transform_grid( transform_grid );
    }

    kSrcGrid->show_grid(src_show);
    kDstGrid->show_grid(dst_show);
    kSrcGrid->lock_grid(src_lock);
    kDstGrid->lock_grid(dst_lock);
    if (!strcmp("src_show", name) ||
        !strcmp("dst_show", name) ||
        !strcmp("src_lock", name) ||
        !strcmp("dst_lock", name)) return 1;
    if (strcmp("src_copy", name)==0) {
        warpgeo.src->copy_to( &copypaste );
        _copyPasteValid = true;
    } else if (strcmp("dst_copy", name)==0) {
        warpgeo.dst->copy_to( &copypaste );
        _copyPasteValid = true;
    } else if (strcmp("src_paste", name)==0) {
        if ( _copyPasteValid &&
                copypaste.width == warpgeo.src->width &&
                copypaste.height == warpgeo.src->height ) {
            kSrcGrid->paste( copypaste );
        }
    } else if (strcmp("dst_paste", name)==0) {
        if ( _copyPasteValid &&
                copypaste.width == warpgeo.dst->width &&
                copypaste.height == warpgeo.dst->height ) {
            kDstGrid->paste( copypaste );
        }
    } else if (strcmp("src_add_c", name)==0) { 
        kSrcGrid->add_mode();
    } else if (strcmp("dst_add_c", name)==0) {
        kDstGrid->add_mode();
    } else if (strcmp("src_remove_c", name)==0) {
        kSrcGrid->remove_mode();
    } else if (strcmp("dst_remove_c", name)==0) {
        kDstGrid->remove_mode();
    } else if (strcmp("src_preset_input", name)==0) {
        const Format &fmt = input0().full_size_format();
        kSrcGrid->fitRect(fmt.x(), fmt.y(), fmt.r(), fmt.t());
    } else if (strcmp("dst_preset_input", name)==0) {
        const Format &fmt = input0().full_size_format();
        kDstGrid->fitRect(fmt.x(), fmt.y(), fmt.r(), fmt.t());
    } else if (strcmp( "translate", name ) == 0 ||
               strcmp( "rotate"   , name ) == 0 ||
	       strcmp( "scale"    , name ) == 0 ||
	       strcmp( "skew"     , name ) == 0 ||
	       strcmp( "center"   , name ) == 0) {
	if (transform_grid == 1 ||	// source
	    transform_grid == 3) {	// both
	    kSrcGrid->transform_changed();
	}
	if (transform_grid == 2 ||	// dest
	    transform_grid == 3) {	// both
	    kDstGrid->transform_changed();
	}
#ifdef DEBUG_TEST_KNOBS
    } else if (strcmp( "print_transform", name ) == 0) {
        GridTransform transform;
#if DD_IMAGE_VERSION_MAJOR < 5
	transform.evaluate( track, k->frame() );
#else
        // changed to OutputContext object for proxy access in 5.0
	transform.evaluate( track, k->uiContext().frame() );
#endif
	std::cout << "track knob| " << transform << std::endl;
    } else if (strcmp( "print_src_xform", name ) == 0) {
        std::cout << "src transform| " << kSrcGrid->xform << std::endl;
    } else if (strcmp( "print_dst_xform", name ) == 0) {
        std::cout << "dst transform| " << kDstGrid->xform << std::endl;
    } else if (strcmp( "print_src_knob", name ) == 0) {
        std::cout << name << "| " << *kSrcGrid << std::endl;
    } else if (strcmp( "print_dst_knob", name ) == 0) {
        std::cout << name << "| " << *kDstGrid << std::endl;
#endif
    } else {
        return 0;
    }

    return 1;
}

#if DD_IMAGE_VERSION_MAJOR < 5
static Iop* build() {return new WarpIop();}
#else
static Iop* build( Node* node ) {return new WarpIop( node );}
#endif

const Iop::Description WarpIop::d(CLASS, "Transform/GridWarp", build);
