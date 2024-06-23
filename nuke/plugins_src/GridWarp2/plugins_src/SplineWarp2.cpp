/* SplineWarp2.cpp
 *
 * Copyright (c) 2008 The Foundry Visionmongers Ltd.  All Rights Reserved.
 * 
 * Merged from Digital Domain 11/11/08
 * 
 */

/*
 ** Mods 03-19-2008 by Raymond Yeung
 ** - ShapeKnob works on per curve basis
 ** - Tracked points can be reset
 */

/*
 ** TODO: GUI
 ** - swap w/ show both reverting/loosing wiered
 **   * work around by not keeping/letting undo,
 **   *   i.e. NO new_undo()/changed() calls in Grid_Knob::swap_stroke(i)
 **   * looks like Nuke SDK problem, must address with Foundry
 **   * still work properly after both src and dst curve has been selected
 **   *   at least once for at least one curve
 ** - proper swap with jack up
 **   * currently disable swapping with jack up
 **   * single curve should be OK
 **   * multiple curve definitely NO
 */

/*
 ** TODO: SplineWarp
 ** - allow uv output or grid rendering mode
 ** - use close spline 1 to define layout
 ** - add one mode: edit vs. freehand draw
 ** - deleting a point does not interpolate the neighbors
 ** - A/B mix slider
 ** - mask out the outer area
 ** - tracking data onto every point
 ** - allow users to close spline curves
 ** - rename everything
 ** - documentation
 ** - keyframes per grid or per line
 ** - removing the last point in a spline seems to leave a ghost point and spline
 */

#include <DDImage/Render.h>
#include <DDImage/Knobs.h>
#include <DDImage/Knob.h>
#include <DDImage/Row.h>
#if DD_IMAGE_VERSION_MAJOR < 5
#include <DDImage/math.h>
#else
#if DD_IMAGE_VERSION_MINOR < 1
#include <DDImage/math.h>
#else
#include <DDImage/DDMath.h>
#endif
#endif
#include <DDImage/ViewerContext.h>
#include <DDImage/Thread.h>
#include <DDImage/LUT.h>
#include <DDImage/Vector3.h>
#include <DDImage/ShapeKnob.h>

//#include <DDMath2/ThinPlateSpline.H>
#include "ThinPlateSpline.h"

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
#include <string.h>
#include <assert.h>

#include <map>
#include <iostream>
#include <sstream>

#include "TransformUtils.h"

static char *HELP = 
"@i;SplineWarp2@n; transforms a source image from its original shape into a deformed "
"image.\n\n"
"Deformations can be defined by creating multiple independent bezier curves. "
"All bezier curves curved into a 'source' curve that describes outlines on "
"the original image, and 'destination' curves."
"The SplineWarp operator will then deform the image by moving the source curves "
"'on' the destination curves while bending the image as if it was "
"a thin metal plate.\n\n"
"The controls work similar to the Bezier opertor. Use Ctrl-Alt-click to "
"generate more points. If no points are selected, Ctrl-Alt-click will start "
"a new curve.\n\n"
"SplineWarp2 is based from Nuke 4.8v1b8 SplineWarp with the additional features:\n"
"- Cusp control per point.\n"
"- Tracking per shape, per curve and per point.\n"
"- ShapeKnob for both source and destination for more flexible and convenient shape interpolation.\n"
"- Visibility control per curve.\n"
"- Ability to swap source and destination per curve.\n"
"- Improved control panel layout."
;

using namespace DD::Image;
using namespace std;

//==== classes implemented in this file ========================================

class MPoint;
class MGrid;
class MTri;
class MRect;
class MTriList;
class MGridList;
class Grid_Knob;
class SplineWarpIop;

//==== statics =================================================================

static const char* const CLASS = "SplineWarp2";
static const char* const CLASSM = "SplineWarpMath2";

// events produced by the menu:
enum {
    SMOOTH = FIRST_MENU,
    CUSP,
    SELECTALL,
    DELETE_POINT,
    DELETE_CURVE,
    OPEN_CLOSE_CURVE,
    HIDE_CURVE,
    SWAP_CURVES
};

// Creates the Menu object when it is first needed:
static Menu* make_menu() {
    static Menu* menu = 0;
    if (menu) return menu;
    menu = new Menu;
    menu->add("select all", COMMAND+'A', SELECTALL);
    menu->add("smooth", 'z', SMOOTH);
    menu->add("cusp", 'c', CUSP);
    menu->add("delete point",       DeleteKey,  DELETE_POINT);
    menu->add("delete curve",       0,          DELETE_CURVE);
#if DD_IMAGE_VERSION_MAJOR < 5
    menu->add("open\\/close curve", 0,          OPEN_CLOSE_CURVE);
#else
    menu->add("open/close curve"  , 0,          OPEN_CLOSE_CURVE);
#endif
    menu->add("hide curve"  ,       0,          HIDE_CURVE);
#if DD_IMAGE_VERSION_MAJOR < 5
    menu->add("swap src\\/dst curves", 0,       SWAP_CURVES);
#else
    menu->add("swap src/dst curves"  , 0,       SWAP_CURVES);
#endif
    menu->add("set key", 0, ShapeKnob::SET_KEY);
    menu->add("delete key", CTRL+'X', ShapeKnob::DELETE_KEY);
    menu->add("copy key", CTRL+'C', ShapeKnob::COPY_KEY);
    menu->add("paste key", CTRL+'V', ShapeKnob::PASTE_KEY);
    return menu;
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
static double getf(const char*&s) {
    sws(s); if (!*s) return 0; 
    double v = atof(s); snws(s); return v;
}
// read and skip an int
static int geti(const char*&s) {
    sws(s); if (!*s) return 0; 
    int v = atoi(s); snws(s); return v;
}



#ifdef DEBUG
#define DEBUG_TEST_KNOBS
#endif


namespace {


enum {
    TRACK_ALL = 0,
    TRACK_CURVE,
    TRACK_POINT,
    TRACK_TYPE_COUNT
};

char const*const track_type_labels[ TRACK_TYPE_COUNT + 1 ] = { "all", "curve", "point", 0};

typedef std::set< int > SelectionSet;


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


bool transform_from_script( char const*& text, Transform& transform )
{
    std::string expression;
    bool ok = extract_bracket_expression( text, expression );
    if ( !ok ) {
        return false;
    }
    return transform.from_script( expression );
}


} // namespace



//==== MPoint ==================================================================

/** The MPoint class describes a manipulatable point (or node) in the grid.
 * Each point has its coordinates and four more handles (coordinates stored 
 * relative to point) that describe the bezier curves that creat the mesh.
 */
class MPoint {
public:
    float x, y; // point position
    float wx, wy, ex, ey; // relative handle coordinates
    int flags; 

    Transform transform;

    MPoint() {
        flags = 0;
    }

    // intialize point to some remotely useful settings
    void init(int ix, int iy, float xscale, float yscale, float xoff, float yoff) 
    {
        x = xscale * ix + xoff;
        y = yscale * iy + yoff;
        wy = ey = 0.0;
        ex =  xscale/3.0;
        wx = -xscale/3.0;
        flags = 0;
    }

    void set_transform( Transform const& transform ) {
        if ( !transform.is_identity() ) {
            Matrix4 const& matrix = transform.get_matrix();
            if ( !is_equal( matrix, Matrix4::identity() ) ) {
                x = 0;
                y = 0;
            }
        }
        this->transform = transform;
    }

    Transform const& get_transform() const {
        return transform;
    }

    void apply_transform( Matrix4 const& parent_matrix ) {
        Matrix4 matrix = parent_matrix;
        if ( !transform.is_identity() ) {
            matrix = transform.get_matrix();
        }
        do_transform( matrix );
    }

    void revert_transform( Matrix4 const& parent_matrix ) {
        Matrix4 matrix = parent_matrix;
        if ( !transform.is_identity() ) {
            matrix = transform.get_matrix();
        }
        do_transform( matrix.inverse() );
    }

    void update_transform( Knob* knob, double frame ) {
        transform.goto_frame( knob, frame );
    }

    void do_transform( Matrix4 const& m )
    {
        MPoint p = *this;
        x = m.a00*p.x + m.a01*p.y + m.a03;
        y = m.a10*p.x + m.a11*p.y + m.a13;
        ex = m.a00*p.ex + m.a01*p.ey;
        ey = m.a10*p.ex + m.a11*p.ey;
        wx = m.a00*p.wx + m.a01*p.wy;
        wy = m.a10*p.wx + m.a11*p.wy;
    }

};

typedef vector<MPoint> MPointList;
typedef vector<MPoint>::iterator MPointListIt;

//==== MStroke ===================================================================

#define STROKE_CLOSED 1

class MStroke {
    int flags;
    int hide;

public:
    Transform transform;

public:
    MPointList point;
    MStroke() {
        flags = 0;
        hide = 0;
        point.reserve(30);
    }
    void set_open() { flags |= STROKE_CLOSED; }
    void set_closed() { flags &= ~STROKE_CLOSED; }
    void flip_closed() { flags ^= STROKE_CLOSED; }
    bool is_closed() const { return (flags&STROKE_CLOSED != 0); }
    void set_visible( bool visible = true ) { hide = !visible; }
    bool is_hide() const { return ((hide)?true:false); }
    void init(int w) {
        point.resize(w);
    }
    int width() const { 
        return point.size(); 
    }
    void copy_to(MStroke &d) const {
        d.point = point;
        d.flags = flags;
        d.hide = hide;
        d.transform = transform;
    }
    void append(Hash& hash) const {
        hash.append(flags);
        if (width()>0) {
            hash.append(&point[0], sizeof(MPoint)*width());
        }
    }
    void merge_bbox(Box& box) const {
        if (!point.size()) return;
        float x,y,r,t,px,py;
        x = r = point[0].x;
        y = t = point[0].y;
        for (unsigned i=0; i<point.size(); i++) {
            px = point[i].x;
            x = MIN(x, px);
            r = MAX(r, px);
            py = point[i].y;
            y = MIN(y, py);
            t = MAX(t, py);
            if (i || is_closed()) {
                px = point[i].x+point[i].wx;
                x = MIN(x, px);
                r = MAX(r, px);
                py = point[i].y+point[i].wy;
                y = MIN(y, py);
                t = MAX(t, py);
            }
            if (is_closed() || i < point.size()-1) {
                px = point[i].x+point[i].ex;
                x = MIN(x, px);
                r = MAX(r, px);
                py = point[i].y+point[i].ey;
                y = MIN(y, py);
                t = MAX(t, py);
            }
        }
        Box b1(fast_floor(x), fast_floor(y),
                fast_floor(r)+1, fast_floor(t)+1);
        box.merge(b1);
    }

    // --- smooth a point out (always point 0) ---
    void smooth(int ix=-1) {
        if (ix == -1) {
        switch (width()) {
            case 0: case 1: break ;
            case 2: { // create a straight line
                        MPoint &p0 = point[0], &p1 = point[1];
                        float dx = p1.x-p0.x, dy = p1.y-p0.y;
                        float len = sqrtf(dx*dx+dy*dy)/3.0f, angle = atan2f(dx, dy);
                        p1.ex = p0.ex = len*sin(angle); p1.wx = p0.wx = -p0.ex; 
                        p1.ey = p0.ey = len*cos(angle); p1.wy = p0.wy = -p0.ey; 
                        break;
                    }
            default: { // create a smooth curve
                         MPoint &p0 = point[0], &p1 = point[1], &p2 = point[2];
                         float dx = p2.x-p0.x, dy = p2.y-p0.y;
                         dx = p1.x-p0.x, dy = p1.y-p0.y;
                         float len01 = sqrtf(dx*dx+dy*dy)/3.0f, a01 = atan2f(dx, dy);
                         dx = p1.x-p2.x, dy = p1.y-p2.y;
                         float len21 = sqrtf(dx*dx+dy*dy)/3.0f, a21 = atan2f(dx, dy);
                         float a02;
                         if (a01>a21) a02 = 0.5f*(a01+a21+M_PI); else a02 = 0.5f*(a01+a21-M_PI);
                         p0.ex = len01*sin(a01+a01-a02); p0.wx = -p0.ex; 
                         p0.ey = len01*cos(a01+a01-a02); p0.wy = -p0.ey; 
                         p1.ex = len21*sin(a02); p1.wx = -len01*sin(a02); 
                         p1.ey = len21*cos(a02); p1.wy = -len01*cos(a02); 
                         //++++ p2.wx = len21*sin(a21-da); p2.ex = -p2.wx; 
                         //++++ p2.wy = len21*cos(a21-da); p2.ey = -p2.wy; 
                         break;
                     }
        }
        } else {
            // create a smooth curve
            MPoint &p0 = point[clamp(ix-1, 0, width()-1)];
            MPoint &p1 = point[ix];
            MPoint &p2 = point[clamp(ix+1, 0, width()-1)];
            float dx = p2.x-p0.x, dy = p2.y-p0.y;
            dx = p1.x-p0.x, dy = p1.y-p0.y;
            float len01 = sqrtf(dx*dx+dy*dy)/3.0f, a01 = atan2f(dx, dy);
            dx = p1.x-p2.x, dy = p1.y-p2.y;
            float len21 = sqrtf(dx*dx+dy*dy)/3.0f, a21 = atan2f(dx, dy);
            float a02;
            if (a01>a21) a02 = 0.5f*(a01+a21+M_PI); else a02 = 0.5f*(a01+a21-M_PI);
            p1.ex = len21*sin(a02); p1.wx = -len01*sin(a02); 
            p1.ey = len21*cos(a02); p1.wy = -len01*cos(a02); 
        }
    }
    // --- cusp a point ---
    void cusp(int ix) {
        if (ix<0 || ix>=(int)point.size()) return;
        MPoint &p = point[ix];
        p.ex = p.wx = 0.0f; 
        p.ey = p.wy = 0.0f; 
    }
    // --- delete a point in the list ---
    void delete_point(int ix) {
        if (ix<0 || ix>=(int)point.size()) return;
        point.erase(point.begin()+ix);
    }
    // --- add a point add the fractional position between two other points ---
    void add_point(int ix, float mul) {
        if (ix<0||ix>=(int)point.size()) return;
        bool wrap = (ix==(int)point.size()-1);
        MPointListIt p = point.begin()+ix+1;
        point.insert(p, point[ix]);
        int ix2 = wrap ? 0 : ix+2;
        float imul = 1-mul;
        MPoint &s1 = point[ix], &d = point[ix+1], &s2 = point[ix2];
        d.x = bezier_at(mul, s1.x, s1.ex, s2.wx, s2.x);
        d.y = bezier_at(mul, s1.y, s1.ey, s2.wy, s2.y);
        float xx = s1.x+0.5f*s1.ex-s2.x-0.5f*s2.wx, 
              yy = s1.y+0.5f*s1.ey-s2.y-0.5f*s2.wy;
        float len = sqrtf(xx*xx+yy*yy)*0.5f;
        float dx = bezier_at(mul*1.02f, s1.x, s1.ex, s2.wx, s2.x) 
            - bezier_at(mul*0.98f, s1.x, s1.ex, s2.wx, s2.x);
        float dy = bezier_at(mul*1.02f, s1.y, s1.ey, s2.wy, s2.y) 
            - bezier_at(mul*0.98f, s1.y, s1.ey, s2.wy, s2.y);
        float a = atan2f(dy, dx);
        d.ey = sinf(a)*len*imul; d.wy = -sinf(a)*len*mul;
        d.ex = cosf(a)*len*imul; d.wx = -cosf(a)*len*mul;
        s1.ex *= mul; s1.ey *= mul;
        s2.wx *= imul; s2.wy *= imul;
    }
    // --- add a point at a new position ---
    void add_point(int ix, float x, float y) {
        if (ix<0) { // add in front of all points
            MPoint p;
            p.x = x; p.y = y;
            p.ex = 50; p.ey = 0;
            p.wx = -50; p.wy = 0;
            point.insert(point.begin(), p);
        }
    }
    // --- set the point ---
    void set_point(int ix, float x, float y) {
        MPoint p = point[ix];
        p.x = x; p.y = y;
    }
    bool point_inside(float x, float y, int res) {
        int w = width(), i1 = 1;
        bool odd = false;
        for (int i0=0; i0<w; i0++, i1++) {
            if (i1==w) i1 = 0;
            MPoint &p0 = point[i0], &p1 = point[i1];

            float p0x = p0.x, p0y = p0.y;
            if (res>1) for (int i=1; i<=res; i++) {
                float m = ((float)i)/((float)res);
                float p1x = bezier_at(m, p0.x, p0.ex, p1.wx, p1.x);
                float p1y = bezier_at(m, p0.y, p0.ey, p1.wy, p1.y);
                if (p0y<y && p1y>=y || p1y<y && p0y>=y)
                    if (p0x+(y-p0y)/(p1y-p0y)*(p1x-p0x)<x)
                        odd = !odd;
                p0x = p1x; p0y = p1y;
            } else {
                float p1x = p1.x, p1y = p1.y;      
                if (p0y<y && p1y>=y || p1y<y && p0y>=y)
                    if (p0x+(y-p0y)/(p1y-p0y)*(p1x-p0x)<x)
                        odd = !odd;
            }

        }
        return odd;
    }

    bool from_script(const char *&text) {
        const char *s = text;
        sws(s); snws(s); // skip the leading '{'
        int version = geti(s);
        if (version>2) 
            fprintf(stderr, "Unsupported Stroke data set. "
                    "Attempting to read anyway.\n");
        int w = geti(s);
        flags = geti(s); 
        hide = geti(s);
        geti(s); // spare
        init(w);

        if ( version > 1 ) {
            // A serialized transform must follow in the expected format written
            // by Transform::to_script.
            bool ok = transform_from_script( s, transform );
            if ( !ok ) {
                return false;
            }
        }

        for (int x=0; x<w; x++) {
            MPoint &p = point[x];
            p.x = getf(s); p.y = getf(s);
            p.wx = getf(s); p.wy = getf(s); p.ex = getf(s); p.ey = getf(s); 
            p.flags = geti(s);
            sws(s);
            if ( version > 1 ) {
                // A serialized transform must follow in the expected format written
                // by Transform::to_script.
                bool ok = transform_from_script( s, p.transform );
                if ( !ok ) {
                    return false;
                }
            }
        }
        sws(s); snws(s); // skip the trailing '}'
        text = s;
        return (version<=2);
    }
    // copy data into a text stream
    void to_script(std::ostream& o, int indent=0) const {
        o << " {" << endl;
        // version number and size
        if (indent) o << "  ";
        o << "        2 " << width() << " " << flags << " " << hide << " 0 " << transform << endl; 
        for (int x=0; x<width(); x++) {
            const MPoint &p = point[x];
            if (indent) o << "  ";
            o << "        " << p.x << " " << p.y << " ";
            o << p.wx << " " << p.wy << " " << p.ex << " " << p.ey << " "; 
            o << p.flags << " " << p.transform << endl;
        }
        if (indent) o << "  ";
        o << "      } ";
    }

    void set_transform( Transform const& transform ) {
        this->transform = transform;
    }

    Transform const& get_transform() const {
        return transform;
    }

    void update_transform( Knob* knob, double frame ) {
        transform.goto_frame( knob, frame );
        for ( int x = 0; x < width(); x++ ) {
            MPoint& p = point[x];
            p.update_transform( knob, frame );
        }
    }

    void apply_transform( Matrix4 const& parent_matrix ) {
        Matrix4 matrix = parent_matrix;
        if ( !transform.is_identity() ) {
            matrix = transform.get_matrix();
        }
        for ( int x = 0; x < width(); x++ ) {
            MPoint& p = point[x];
            p.apply_transform( matrix );
        }
    }

    void revert_transform( Matrix4 const& parent_matrix ) {
        Matrix4 matrix = parent_matrix;
        if ( !transform.is_identity() ) {
            matrix = transform.get_matrix();
        }
        for ( int x = 0; x < width(); x++ ) {
            MPoint& p = point[x];
            p.revert_transform( matrix );
        }
    }

};

typedef vector<MStroke> MStrokeList;
typedef vector<MStroke>::iterator MStrokeListIt;

//==== MGrid ===================================================================

/** The MGrid holds a number of MPoints to describe the Bezier mesh
 */
class MGrid {
public:
    Transform transform;

    MStrokeList stroke;
    MPoint *point(int n) { return &stroke[n].point[0]; }
    MPoint *point(int n, int m) { return &stroke[n].point[m]; }
    MPoint *point(int n, int m) const { return &((MGrid*)this)->stroke[n].point[m]; }
    int width(int n) const { return (n<int(stroke.size()))?stroke[n].width():0; }
    int height() const { return stroke.size(); }
    bool is_closed(int n) const { return stroke[n].is_closed(); }
    bool is_hide(int n) const { return stroke[n].is_hide(); }
    int size() const {
        int i, h = height(), n = 0;
        for (i=0; i<h; i++) n += width(i);
        return n;
    }
    unsigned int color;
    MGrid() {
        stroke.reserve(20);
        color = 0xcccccc00;
    }
    ~MGrid() {
    }
    void append(Hash& hash) const {
        for (int i=0; i<height(); i++) stroke[i].append(hash);
    }
    void fitRect(int x, int y, int r, int t) {
        float xs = (r-x)/10.0f, ys = (t-y)/float(height()-1.0);
        for (int iy=0; iy<height(); iy++) {
            MPoint *dst = point(iy);
            for (int ix=0; ix<width(iy); ix++) 
                (dst++)->init(ix, iy, xs, ys, x, y);
        }
    }
    void init(int w, int h, float scale=50.0f, float xoff=100.0f, float yoff=100.0f) {
        stroke.resize(h);
        for (int y=0; y<h; y++) {
            if (w) {
                stroke[y].init(w);
                MPoint *dst = point(y);
                for (int x=0; x<width(y); x++) 
                    (dst++)->init(x, y, scale, scale, xoff, yoff);
            }
        }
    }
    MGrid *dup() const {
        MGrid *g = new MGrid();
        copy_to(g);
        return g;
    }

    void lerp(MGrid *a, MGrid *b, float f) {
        float g = 1.0f-f;
        for (int y=0; y<height(); y++) {
            for (int x=0; x<width(y); x++) {
                MPoint *pa = a->point(y,x), *pb = b->point(y,x), *pd = point(y,x);
                Transform const& point_transform = pa->get_transform();
                if ( !point_transform.is_identity() ) {
                    continue;
                }
                pd->x  = pa->x*g  + pb->x*f;  pd->y  = pa->y*g  + pb->y*f; 
                pd->ex = pa->ex*g + pb->ex*f; pd->ey = pa->ey*g + pb->ey*f; 
                pd->wx = pa->wx*g + pb->wx*f; pd->wy = pa->wy*g + pb->wy*f; 
                pa++; pb++; pd++;
            }
        }
    }

    void resize(int h) {
        stroke.resize(h);
    }
    void copy_to(MGrid *d) const {
        d->resize(height());
        for (int y=0; y<height(); y++) {
            stroke[y].copy_to(d->stroke[y]);
        }
        d->transform = transform;
    }
    void copy_selection_to(MGrid *d, SelectionSet& selects) const {
        d->resize( selects.size() );
	int d_index = 0;
	for ( SelectionSet::const_iterator it = selects.begin(); it != selects.end(); it++ ) {
	    SelectionSet::value_type const& entry = *it;
	    int index = entry >> 16;
	    stroke[index].copy_to( d->stroke[d_index++] );
	}
    }
    MPoint &at(int x, int y) {
        return *point(y, x);
    }
    MPoint &at(int x, int y) const {
        return *point(y, x);
    }
    // --- delete a point ---
    void delete_point(int ix, int iy) {
        if (iy<0 || iy>=height()) return;
        if (width(ix)==1)
            delete_stroke(iy);
        else
            stroke[iy].delete_point(ix);
    }
    // --- smooth a point ---
    void smooth(int ix, int iy) {
        if (iy<0 || iy>=height()) return;
        stroke[iy].smooth(ix);
    }
    // --- delete a stroke ---
    void delete_stroke(int iy) {
        if (iy<0 || iy>=height()) return;
        stroke.erase(stroke.begin()+iy);
    }
    // --- toggle the 'closed' flag ---
    void flip_closed(int iy) {
        if (iy<0 || iy>=height()) return;
        stroke[iy].flip_closed();
    }
    // --- set the 'hide' flag ---
    void set_stroke_visible(int iy, bool visible = true) {
        if (iy<0 || iy>=height()) return;
        stroke[iy].set_visible( visible );
    }
    void set_all_stroke_visible(bool visible = true) {
        for ( int i = 0; i < height(); i++ ) {
            stroke[i].set_visible( visible );
        }
    }
    // --- add a new stroke to the grid ---
    int add_curve() {
        stroke.push_back(MStroke());
        return stroke.size()-1;
    }
    // --- add a new point: mul=position between ix and ix+1 ---
    void add_point(int ix, int iy, float mul) {
        if (iy<0 || iy>=height()) return;
        stroke[iy].add_point(ix, mul);
    }
    // --- add a new point ---
    void add_point(int ix, int iy, float x, float y) {
        if (iy<0 || iy>=height()) return;
        stroke[iy].add_point(ix, x, y);
    }
    // --- set the point position ---
    void set_point(int ix, int iy, float x, float y) {
        if (iy<0 || iy>=height()) return;
        stroke[iy].set_point(ix, x, y);
    }
    bool from_script(const char *&text) {
        const char *s = text;
        int version = geti(s);
        if (version>2) 
            fprintf(stderr, "Unsupported Grid data set. "
                    "Attempting to read anyway.\n");
        int h = geti(s);
        geti(s); geti(s); // spare
        init(0, h);

        if ( version > 1 ) {
            // A serialized transform must follow in the expected format written
            // by Transform::to_script.
            bool ok = transform_from_script( s, transform );
            if ( !ok ) {
                return false;
            }
        }

        for (int y=0; y<h; y++) {
            stroke[y].from_script(s);
        }
        text = s;
        return (version<=2);
    }
    // copy data into a text stream
    void to_script(std::ostream& o, int indent=0) const {
        o << " {" << endl;
        // version number and size
        if (indent) o << "  ";
        o << "      2 " << height() << " 0 0 " << transform;
        for (int y=0; y<height(); y++) {
            stroke[y].to_script(o, indent);
        }
        if (indent) o << "  ";
        o << endl << "    } ";
    }

    void set_transform( Transform const& transform, int track_type, SelectionSet const& selection ) {
        switch ( track_type ) {
        case TRACK_ALL:
            this->transform = transform;
            break;
        case TRACK_CURVE:
            for ( SelectionSet::const_iterator it = selection.begin(); it != selection.end(); it++ ) {
                SelectionSet::value_type const& entry = *it;
                int index = entry >> 16;
                MStroke& s = stroke[ index ];
                s.set_transform( transform );
            }
            break;
        case TRACK_POINT:
            for ( SelectionSet::const_iterator it = selection.begin(); it != selection.end(); it++ ) {
                SelectionSet::value_type const& entry = *it;
                int x = entry & 0xff;
                int y = entry >> 16;
                MPoint& p = at( x, y );
                p.set_transform( transform );
            }
            break;
        }
    }

    Transform get_transform( int track_type, SelectionSet const& selection ) const {

        if ( selection.empty() ) {
            return Transform();
        }

        SelectionSet::const_iterator it = selection.begin();
        SelectionSet::value_type const& entry = *it;

        switch ( track_type ) {

        case TRACK_ALL:
            return this->transform;

        case TRACK_CURVE:
            {
                int index = entry >> 16;
                MStroke const& s = stroke[ index ];
                return s.get_transform();
            }

        case TRACK_POINT:
            {
                int x = entry & 0xff;
                int y = entry >> 16;
                MPoint const& p = at( x, y );
                return p.get_transform();
            }

        }

        return Transform();
        
    }

    void update_transform( Knob* knob, double frame ) {
        transform.goto_frame( knob, frame );
        for ( int y = 0; y < height(); y++ ) {
            MStroke& s = stroke[y];
            s.update_transform( knob, frame );
        }
    }

    void apply_transform() {
        Matrix4 matrix = transform.get_matrix();
        for ( int y = 0; y < height(); y++ ) {
            MStroke& s = stroke[y];
            s.apply_transform( matrix );
        }
    }

    void revert_transform() {
        Matrix4 matrix = transform.get_matrix();
        for ( int y = 0; y < height(); y++ ) {
            MStroke& s = stroke[y];
            s.revert_transform( matrix );
        }
    }

#ifdef DEBUG_TEST_KNOBS
    void print_transform() {
        std::cout << "grid| " << transform << std::endl;
        for (int y=0; y<height(); y++) {
            std::cout << "stroke| " << stroke[y].transform << std::endl;
            for (int x=0; x<width(y); x++) {
                MPoint& p = at(x,y);
                std::cout << "point| " << p.x << " " << p.y << " " << p.transform << std::endl;
            }
        }
    }
#endif // DEBUG_TEST_KNOBS


};

//==== GridKnob ================================================================

// some constants for drawing the transformation jack
const double radius = 30;
const double handle_length = 90;
const double skew_length = 50;
static bool mod_ctrl  = false;
static bool mod_alt   = false;
static bool mod_shift = false;
#define dscale() sqrt(ctx->zoom())


/** The grid knob is a data storage that provides
 * a bezier grid (patch) style user interface element
 */
class Grid_Knob : public ShapeKnob {
    // Backlink to Iop for fast preview operations
    SplineWarpIop *iop;
    // This is our grid of nodes including bezier handles
    MGrid grid, jack_grid;
    // This is our list of other grids along the animation
    std::vector<MGrid> gridlist;
    // Make sure that the resolition of the partner grid stays the same as in 
    // this grid
    Grid_Knob *partner_;
    // user may want to hide the grid
    bool hide_grid_;
    int first_is_mask_;
    // additinally to 'edit', we support an 'add' mode (1) and 'remove' mode (2)
    bool start_new_curve;
    // keep track of curve selection changes
    int selected_curve_good;
    // last curve in list with selected points in them
    int selected_curve_;

    // TransformJack stuff
    int prevSelected;
    float jack_tx, jack_ty, jack_cx, jack_cy, jack_rotate, jack_irotate;
    float jack_skew, jack_scale_x, jack_scale_y;
    double pixel_aspect;

    Knob* track;
    Transform transforms[ TRACK_TYPE_COUNT ];
    SelectionSet selection;


protected:
    int edit_mode;
    // there is no default value for this data set
    bool not_default() const {return true;}
    // nothing will show in the panel, however the knob will know about UI changes
    bool hidden() const {return false;}
    /// the knob has keys, so can't be invisible
    bool invisible() const { return false; }
    // copy data from a text string into the Primatte control structure
    bool from_script(const char* text) {
        const char *s = text;
        int version = geti(s);
        if (version!=2) {
            fprintf(stderr, "Unsupported Grid data set.\n");
            return false;
        }
        geti(s); geti(s); // skip the spare
        first_is_mask_ = geti(s); 
        sws(s);
        if (*s=='{') {
            s++;
            if (grid.from_script(s)==false)
                return false;
        }
        sws(s);
        if (*s=='}') s++;
        sws(s);
        if (*s=='{') {
            s++;
            gridlist.clear();
            if (geti(s) > 1) {
                fprintf(stderr, "Unsupported Grid data set. Skipping.\n");
                return false;
            }
            int num_keys = geti(s);
            geti(s); geti(s); geti(s); // skip unused fields
            keys(num_keys);
            gridlist.reserve(keys());
            for (int n=0; n < num_keys; n++) {
                /*double frame = */getf(s);
                sws(s); 
                if (*s=='{') {
                    s++;
                    gridlist.push_back(MGrid());
                    MGrid* g = &gridlist[n];
                    if (g->from_script(s)==false)
                        return false;
                    sws(s); 
                    if (*s=='}') s++;
                }
            }
        }

        changed();
        return true;
    }

#if DD_IMAGE_VERSION_MAJOR < 5
    void to_script(std::ostream& o, ToScriptReason, bool quote) const {
#else
    // changed to OutputContext object for proxy access in 5.0
    void to_script(std::ostream& o, const OutputContext*, bool quote) const {
#endif
        if (quote) o << " {" << endl;
        // version number and spare:
        o << "    2 " << grid.width(0) << " " << grid.height() 
            << " " << first_is_mask_ << " "; 
        grid.to_script(o);           // write the current grid
        // version number, number of keys and spare
        o << " {" << endl << "      1 " << gridlist.size() << " 0 0 0 "; 
        for (unsigned i=0; i < gridlist.size(); i++) {
            o << endl << "      " << time(i) << " ";
            gridlist[i].to_script(o, 1);
        }
        o << endl << "    } ";
        if (quote) o << endl << "  } ";
    }

public:
    // create a new data knob. Key reference can not be changed later.
#if DD_IMAGE_VERSION_MAJOR < 5
    Grid_Knob(MGrid *src, const char* n, const char* shapename) :
        ShapeKnob(n, shapename)
#else
    Grid_Knob( Knob_Closure* cb,
              MGrid *src, const char* n, const char* shapename ) :
        ShapeKnob( cb, n, shapename )
#endif
    {
        if (src) {
            src->copy_to(&grid);
            grid.color = src->color;
        } else {
            grid.init(3, 3);
        }
        partner_ = 0;
        hide_grid_ = false;
        iop = 0;
        prevSelected = 0;
        selected_curve_good = 0;
        jack_tx = 0.0; jack_ty = 0.0;
        jack_cx = 0.0; jack_cy = 0.0;
        jack_rotate = 0.0; jack_irotate = 0.0; jack_skew = 0.0;
        jack_scale_x = 1.0; jack_scale_y = 1.0;
        pixel_aspect = 1.0;
        selected_curve_ = 0;
        first_is_mask_ = 0;

        track = 0;
        for ( int i = 0; i < TRACK_TYPE_COUNT; i++ ) {
            transforms[ i ].make_identity();
        }
    }
    // Selection
    const SelectionSet* get_selection() { return &selection; }
    // The partner grid, if existing, must maintain the same resolution
    void partner(Grid_Knob *k) { partner_ = k; }
    bool is_hide (int index) { return grid.is_hide(index); }
    // user wants to hide grid. Unrelated to the knob->hidden() above
    void hide_grid(bool v) { hide_grid_ = v; }
    bool is_grid_visible() const { return !hide_grid_; }
    // if this is 1, the first bezier is closed and drawn in a different color
    // if this is 2, the first bezier is not drawn
    void first_is_mask(int v) { first_is_mask_ = v; }

#if DD_IMAGE_VERSION_MAJOR < 5
#else
    // changed to OutputContext object for proxy access in 5.0
    bool gotoContext( const OutputContext& context ) {
        return goto_shape( shape(context.frame()), context );
    }
#endif

    /*virtual*/
    void set_key(int key) {
        if (key == 0 && gridlist.empty()) gridlist.push_back(MGrid());
        new_undo();
        grid.copy_to( &gridlist[key] );
        changed();
    }

    /*virtual*/
    void add_key(int key) {
        gridlist.insert(gridlist.begin()+key, MGrid());
        grid.copy_to( &gridlist[key] );
    }

    /*virtual*/
    void delete_keys(int first, int number) {
        if (!number) return;
        gridlist.erase(gridlist.begin()+first, gridlist.begin()+first+number);
    }

    /*virtual*/
    bool keyable() const {
//        return grid.num_points() != 0;
        return true;
    }

    bool goto_frame(double frame) {
        return ShapeKnob::goto_frame(frame);
    }

    /*virtual*/
#if DD_IMAGE_VERSION_MAJOR < 5
    bool goto_shape(float shape, double frame) {
#else
    // changed to OutputContext object for proxy access in 5.0
    bool goto_shape(float shape, const OutputContext& context) {
        double frame = context.frame();
#endif
        update_transforms( frame );

        // no keyframes: return current shape:
        if (gridlist.empty()) return true;

        // Figure out the integer and 0-1 't' for the interpolation:
        int first_key = fast_floor(shape);
        if (first_key > keys()-2) first_key = keys()-2;
        if (first_key < 0) first_key = 0;

        float t = shape - (float)first_key;
        if ( keys() < 2 ) {
            gridlist[first_key].copy_to( &grid );
        } else {
            // Get the interpolated grid between two shapes:
            grid.lerp(&gridlist[first_key], &gridlist[first_key+1], t);
        }

        return true;
    }

    // make sure that Nuke knows if the key changed in any way
    /*virtual*/
#if DD_IMAGE_VERSION_MAJOR < 5
    void store(void *dst, double frame) {
        goto_frame(frame);
        grid.copy_to((MGrid*)dst);
    }
#else
    // changed to OutputContext object for proxy access in 5.0
    void store( StoreType, void *dst, Hash& hash, const OutputContext& context ) {
        gotoContext( context );
        grid.copy_to((MGrid*)dst);
        grid.append( hash );
    }
#endif
    /*virtual*/
#if DD_IMAGE_VERSION_MAJOR < 5
    void append(Hash& hash, double frame) {
        goto_frame(frame);
        grid.append(hash);
    }
#else
    // changed to OutputContext object for proxy access in 5.0
    void append( Hash& hash, const OutputContext* context ) {
        if (context) gotoContext( *context );
        // not right if context is zero, should use whole animation
	grid.append( hash );
    }
#endif

    void update_selection() {
        selection.clear();
        for ( int y = 0; y < grid.height(); y++ ) {
            for ( int x = 0; x < grid.width(y); x++ ) {
                int i = ( y << 16 ) + x;
                if ( is_selected( 0, xy_cb, i<<3 ) ) {
                    selection.insert( i );
                }
            }
        }
    }

    bool has_selection() const {
        return !selection.empty();
    }

    Transform get_selection_transform( int track_type ) const {
        if ( has_selection() ) {
            return grid.get_transform( track_type, selection );
        } else {
            return transforms[ track_type ];
        }
    }

    // -- access to transform parameters
    // ** a bit strangely cumbersome, should be a better data structure later
    void get_xform_info(float& tx, float& ty, float& cx, float& cy,
                        float& rotate, float& irotate, float& skew,
			float& scale_x, float& scale_y, double& aspect) {
        tx = jack_tx; ty = jack_ty;
	    cx = jack_cx; cy = jack_cy;
	    rotate = jack_rotate;
	    irotate = jack_irotate;
	    skew = jack_skew;
	    scale_x = jack_scale_x;
	    scale_y = jack_scale_y;
	    aspect = pixel_aspect;
    }
    void set_xform_info(float tx, float ty, float cx, float cy,
                        float rotate, float irotate, float skew,
			float scale_x, float scale_y, double aspect) {
        jack_tx = tx; jack_ty = ty;
	    jack_cx = cx; jack_cy = cy;
	    jack_rotate = rotate;
	    jack_irotate = irotate;
	    jack_skew = skew;
	    jack_scale_x = scale_x;
	    jack_scale_y = scale_y;
	    pixel_aspect = aspect;
    }

    void paste( const MGrid &fromGrid ) {
        new_undo();
        fromGrid.copy_to( &grid );
#if DD_IMAGE_VERSION_MAJOR < 5
        maybe_set_key(frame());
#else
        maybe_set_key();
#endif
    }

    void paste_selection_from( const MGrid &fromGrid, const SelectionSet &selects) {
// ????? cheezy check for selection persistence
	if (fromGrid.height() != (int)selects.size()) {
// ????? should add some warning or note or something when it refuses
	    return;
	}
        new_undo();
	int from_index = 0;
	for ( SelectionSet::const_iterator it = selects.begin(); it != selects.end(); it++ ) {
	    SelectionSet::value_type const& entry = *it;
	    int index = entry >> 16;
	    fromGrid.stroke[from_index++].copy_to( grid.stroke[index] );
	}
#if DD_IMAGE_VERSION_MAJOR < 5
        maybe_set_key(frame());
#else
        maybe_set_key();
#endif
        changed();
    }

    // remove a curve from all grids
    void delete_stroke(int ix) {
        grid.delete_stroke(ix);
        for (unsigned i=0; i < gridlist.size(); i++)
            gridlist[i].delete_stroke(ix);
        changed();
    }

    // toggle the 'closed' flag on this stroke to generate a closed loop
    void flip_closed(int ix) {
        grid.flip_closed(ix);
        for (unsigned i=0; i < gridlist.size(); i++)
            gridlist[i].flip_closed(ix);
        changed();
    }
    void set_stroke_visible(int ix, bool visible = true) {
        grid.set_stroke_visible(ix, visible);
        for (unsigned i=0; i < gridlist.size(); i++)
            gridlist[i].set_stroke_visible(ix, visible);
        changed();
    }
    void set_all_stroke_visible(bool visible = true) {
        grid.set_all_stroke_visible(visible);
        for (unsigned i=0; i < gridlist.size(); i++)
            gridlist[i].set_all_stroke_visible(visible);
        changed();
    }
    void swap_stroke(int ix, bool script_knob_flag = false) {
        // don't do it if jack is up
	if (selected_curve_good > 1) return;
	// the other grid/animation missing
	if (!partner_) return;
	// ? #points: src stroke != dst stroke
	if (grid.stroke[ix].width() != partner_->grid.stroke[ix].width()) return;
// *********************************************************************
// This undo/change pair is not used due to Nuke SDK undo tracking issue
// * To be resolved with Foundry
	// knob needs a new undo
#if DD_IMAGE_VERSION_MAJOR < 5
        // new_undo();
#else
	if (script_knob_flag) {
            new_undo();
        }
#endif
	//
	// the current grid
	//   temporary stroke for grid
	MStroke g_stroke;
	grid.stroke[ix].copy_to(g_stroke);
	//   temporary stroke for partner
	MStroke p_stroke;
	partner_->grid.stroke[ix].copy_to(p_stroke);
	//   copy temporary stroke from grid to partner
	g_stroke.copy_to(partner_->grid.stroke[ix]);
	//   copy temporary stroke from partner to grid
	p_stroke.copy_to(grid.stroke[ix]);
        //   ????? attempt to relate jack grids
        //   jack grid next
	//   ????? just a copy from main grid
	grid.copy_to(&jack_grid);
	partner_->grid.copy_to(&(partner_->jack_grid));
	//   ????? attempt to relate transform parameters
	//   jack_... next
        float  g_tx, g_ty, g_cx, g_cy, g_rotate, g_irotate, g_skew,
	       g_scale_x, g_scale_y;
        double g_aspect;
        float  p_tx, p_ty, p_cx, p_cy, p_rotate, p_irotate, p_skew,
	       p_scale_x, p_scale_y;
        double p_aspect;
	//   get main grid
        get_xform_info
	    (g_tx, g_ty, g_cx, g_cy, g_rotate, g_irotate, g_skew,
	     g_scale_x, g_scale_y, g_aspect);
	//   get partner grid
        partner_->get_xform_info
	    (p_tx, p_ty, p_cx, p_cy, p_rotate, p_irotate, p_skew,
	     p_scale_x, p_scale_y, p_aspect);
	//   set main grid
        set_xform_info
	    (p_tx, p_ty, p_cx, p_cy, p_rotate, p_irotate, p_skew,
	     p_scale_x, p_scale_y, p_aspect);
        //   set partner grid
        partner_->set_xform_info
	    (g_tx, g_ty, g_cx, g_cy, g_rotate, g_irotate, g_skew,
	     g_scale_x, g_scale_y, g_aspect);
	// check for gridlist
	if (gridlist.empty())
        {
// *********************************************************************
// This undo/change pair is not used due to Nuke SDK undo tracking issue
// * To be resolved with Foundry
#if DD_IMAGE_VERSION_MAJOR < 5
	    //changed();
#else
	    changed();
#endif
	    return;
	}
	// have to do this for all grids in animation keyframes
        unsigned n = 0;
        unsigned m = 0;
        for (; (m < gridlist.size()) && (n < partner_->gridlist.size()); n++, m++) {
            MGrid& m_grid = gridlist[m];
            MGrid& p_grid = partner_->gridlist[n];
	    // temporary stroke for grid
	    m_grid.stroke[ix].copy_to(g_stroke);
	    // temporary stroke for partner
	    p_grid.stroke[ix].copy_to(p_stroke);
	    // copy temporary stroke from partner to grid
	    p_stroke.copy_to(m_grid.stroke[ix]);
	    // copy temporary stroke from grid to partner
	    g_stroke.copy_to(p_grid.stroke[ix]);
        }
        // main grid changed
// *********************************************************************
// This undo/change pair is not used due to Nuke SDK undo tracking issue
// * To be resolved with Foundry
#if DD_IMAGE_VERSION_MAJOR < 5
	//changed();
#else
	changed();
#endif
    }
    // --- delete a single point from a curve ---
    int delete_point(int ix, int iy) {
        grid.delete_point(ix, iy);
        for (unsigned i=0; i < gridlist.size(); i++)
            gridlist[i].delete_point(ix, iy);
        changed();
        return iy;
    }
    // --- smoothe out a point and its neighbors ---
    int smooth(int ix, int iy) {
        grid.smooth(ix, iy);
        for (unsigned i=0; i < gridlist.size(); i++)
            gridlist[i].smooth(ix, iy);
        changed();
        return iy;
    }
    // --- add a new curve to the list ---
    int add_curve() {
        int iy = grid.add_curve();
        for (unsigned i=0; i < gridlist.size(); i++)
            gridlist[i].add_curve();
        changed();
        return iy;
    }
    // --- create a new point on an existiong curve ---
    void add_point(int ix, int iy, float at) {
        grid.add_point(ix, iy, at);
        for (unsigned i=0; i < gridlist.size(); i++)
            gridlist[i].add_point(ix, iy, at);
        changed();
    }
    // --- create a new point at this position ---
    void add_point(int ix, int iy, float x, float y) {
        grid.add_point(ix, iy, x, y);
        for (unsigned i=0; i < gridlist.size(); i++)
            gridlist[i].add_point(ix, iy, x, y);
        changed();
    }
    // --- set a point on all keyframes ---
    void set_point(int ix, int iy, float x, float y) {
        grid.set_point(ix, iy, x, y);
        for (unsigned i=0; i < gridlist.size(); i++)
            gridlist[i].set_point(ix, iy, x, y);
        changed();
    }

    bool build_handle(DD::Image::ViewerContext *ctx) {
        return (ctx->transform_mode()==VIEWER_2D && isEnabled() && !hide_grid_);
    }
    void draw_handle(DD::Image::ViewerContext *ctx);
    void draw_handle_edit(DD::Image::ViewerContext *ctx) {
        int x, y;
        if (!ctx->draw_knobs()) return;
        if (!this->name()) return;

#if DD_IMAGE_VERSION_MAJOR < 5
#else
        // changed to OutputContext object for proxy access in 5.0
	OutputContext uiCtx = uiContext();
#endif

        unsigned int color1 = grid.color, color2 = 0xaaffaa00;
        if (ctx->event()==DRAW_SHADOW) color1 = color2 = 0;      
        int start = (first_is_mask_==2) ? 1 : 0;

        // draw the index number of this stroke //++ make this conditional
        char buf[16];
        float off = 4.0f/ctx->zoom();
        for (y=0; y<grid.height(); y++) if (grid.width(y)>0) {
            if (grid.is_hide(y)) {
  	        continue;
  	    }
#if DD_IMAGE_VERSION_MAJOR < 5
            const float xx = to_proxy_x(grid.at(0, y).x);
            const float yy = to_proxy_y(grid.at(0, y).y);
#else
            // changed to OutputContext object for proxy access in 5.0
            float xx = grid.at( 0, y ).x;
            float yy = grid.at( 0, y ).y;
	    uiCtx.to_proxy_xy( xx, yy );
#endif
            if (y==0&&first_is_mask_) {
                if (first_is_mask_==1) {
                    glColor(color2);
                    gl_text("mask", xx+off, yy+off);
                }
            } else {
                glColor(color1);
                sprintf(buf, "%d", y+1);
                gl_text(buf, xx+off, yy+off);
            }
        }
        // draw colored lines into the mesh
        for (y=start; y<grid.height(); y++) {
            if (grid.is_hide(y)) {
  	        continue;
  	    }
            glColor((y==0&&first_is_mask_)?color2:color1);
            int nn = grid.width(y)-1;
            int n = (grid.is_closed(y)) ? nn+1 : nn;
            for (x=0; x<n; x++) {
                begin_handle(Knob::DISTANCE_FROM_POINT, ctx,
                        handle_on_curve_cb, ((y<<16)+x), 0, 0, 0);
                glBegin(GL_LINE_STRIP);
#if DD_IMAGE_VERSION_MAJOR < 5
                const float x1 = to_proxy_x(grid.at(x, y).x);
                const float y1 = to_proxy_y(grid.at(x, y).y);
                const float x2 = x1+to_proxy_w(grid.at(x, y).ex);
                const float y2 = y1+to_proxy_h(grid.at(x, y).ey);
#else
                // changed to OutputContext object for proxy access in 5.0
                float x1 = grid.at( x, y ).x;
                float y1 = grid.at( x, y ).y;
		uiCtx.to_proxy_xy( x1, y1 );
                float ex = grid.at( x, y ).ex;
                float ey = grid.at( x, y ).ey;
		uiCtx.to_proxy_wh( ex, ey );
                float x2 = x1 + ex;
                float y2 = y1 + ey;
#endif
                glVertex2f(x1, y1);
                const int xx = (x==nn) ? 0 : x+1;
#if DD_IMAGE_VERSION_MAJOR < 5
                const float x4 = to_proxy_x(grid.at(xx, y).x);
                const float y4 = to_proxy_y(grid.at(xx, y).y);
                const float x3 = x4+to_proxy_w(grid.at(xx, y).wx);
                const float y3 = y4+to_proxy_h(grid.at(xx, y).wy);
#else
        	// changed to OutputContext object for proxy access in 5.0
                float x4 = grid.at( xx, y ).x;
                float y4 = grid.at( xx, y ).y;
		uiCtx.to_proxy_xy( x4, y4 );
                float wx = grid.at( xx, y ).wx;
                float wy = grid.at( xx, y ).wy;
		uiCtx.to_proxy_wh( wx, wy );
                float x3 = x4 + wx;
                float y3 = y4 + wy;
#endif
                gl_bezierf(x1,y1, x2,y2, x3,y3, x4,y4, ctx->zoom());
                glEnd();
                end_handle(ctx);
            }
        }
        // draw white positioners
        for (y=start; y<grid.height(); y++) {
            if (grid.is_hide(y)) {
  	        continue;
  	    }
            glColor((y==0&&first_is_mask_)?color2:color1);
            for (x=0; x<grid.width(y); x++) {
                int i = (y<<16)+x;
                MPoint &p = grid.at(x, y);
#if DD_IMAGE_VERSION_MAJOR < 5
                float px = to_proxy_x(p.x);
                float py = to_proxy_y(p.y);
#else
        	// changed to OutputContext object for proxy access in 5.0
                float px = p.x;
                float py = p.y;
		uiCtx.to_proxy_xy( px, py );
#endif
                make_handle(SELECTABLE, ctx, xy_cb, i<<3, px, py);
                Transform const& point_transform = p.get_transform();
                if ( !point_transform.is_identity() ) {
                    float A_offset = ( x > 0 ? off : -off );
                    gl_text("A", px+A_offset, py+off);
                }
            }
        }
        // draw bright white active handles
        if (ctx->event()==DRAW_LINES) glColor(0xffffff00);
        for (int iy=start; iy<grid.height(); iy++) {
            if (grid.is_hide(iy)) {
  	        continue;
  	    }
            for (int ix=0; ix<grid.width(iy); ix++) {
                int i = (iy<<16)+ix;
                if (!ShapeKnob::is_selected(ctx, xy_cb, i<<3)) continue;
                MPoint &p = grid.at(ix, iy);
#if DD_IMAGE_VERSION_MAJOR < 5
                const float x = to_proxy_x(p.x);
                const float y = to_proxy_y(p.y);
#else
        	// changed to OutputContext object for proxy access in 5.0
                float x = p.x;
                float y = p.y;
		uiCtx.to_proxy_xy( x, y );
#endif
                if (ix<grid.width(iy)-1 || grid.is_closed(iy)) {
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
                if (ix>0 || grid.is_closed(iy)) {
#if DD_IMAGE_VERSION_MAJOR < 5
                    const float dx = to_proxy_w(p.wx);
                    const float dy = to_proxy_y(p.wy);
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

        bool catch_empty_clicks = true;

        if (ctx->event()!=DRAG) {
            int nSelected = draw_xform_jack(ctx);
            if (mod_alt && mod_ctrl && !mod_shift) {
                catch_empty_clicks = false;
                if (nSelected==0)
                    begin_handle(ANYWHERE, ctx, handle_anywhere_new_curve_cb, 0);
                else
                    begin_handle(ANYWHERE, ctx, handle_anywhere_add_point_cb, 0);
            }
        }

        if ( catch_empty_clicks ) {
            // catch clicks that don't hit any other handles
            begin_handle( ANYWHERE, ctx, clicked_empty_space_cb, 0 );
        }
    }

    static bool clicked_empty_space_cb( ViewerContext* ctx, Knob* data, int index ) {
        Grid_Knob* knob = dynamic_cast< Grid_Knob* >( data );
        if ( knob ) {
            knob->clicked_empty_space( ctx, index );
        }
        // let other things happen in response to clicking/dragging on
        // empty space such as rubber-band selection.
        return false;
    }

    void clicked_empty_space( ViewerContext* ctx, int index ) {
        selection.clear();
    }

    void jack_changed(ViewerContext* ctx) {
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
        for (int y=0; y<grid.height(); y++) {
            for (int x=0; x<grid.width(y); x++) {
                int i = (y<<16)+x;
                if (is_selected(ctx, xy_cb, i<<3)) {
                    MPoint &p = grid.at(x, y);	
                    MPoint &o = jack_grid.at(x, y);	
                    p.x = m.a00*o.x + m.a01*o.y + m.a03;
                    p.y = m.a10*o.x + m.a11*o.y + m.a13;
                    p.ex = m.a00*o.ex + m.a01*o.ey; p.ey = m.a10*o.ex + m.a11*o.ey;
                    p.wx = m.a00*o.wx + m.a01*o.wy; p.wy = m.a10*o.wx + m.a11*o.wy;
                }
            }
        }
#if DD_IMAGE_VERSION_MAJOR < 5
        maybe_set_key(frame());
#else
        maybe_set_key();
#endif
    }
    static bool drag_translate(ViewerContext* ctx, Knob* p, int) {
        if (ctx->event() != DRAG) return false;
        Grid_Knob &k = *(Grid_Knob*)p;
        k.jack_tx = ctx->x();
        k.jack_ty = ctx->y();
        k.jack_changed(ctx);
        return true;
    }
    static bool drag_rotate(ViewerContext* ctx, Knob* p, int) {
        static double irotate;
        Grid_Knob &k = *(Grid_Knob*)p;
        if (ctx->event() == PUSH) {irotate = k.jack_irotate; return true;}
        if (ctx->event() != DRAG) return false;
        float mx = ctx->x();
        float my = ctx->y();
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
        if (mod_ctrl) {
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
    static bool drag_skew(ViewerContext* ctx, Knob* p, int)
    {
        if (ctx->event() != DRAG) return false;
        Grid_Knob &k = *(Grid_Knob*)p;
        if (!ctx->y()) return true;
        double v = clamp(ctx->x()/ctx->y(), -4, 4);
        if (!ctx->state(CTRL|SHIFT|ALT|META) && fabs(v)<.1) v = 0;
        k.jack_skew = v;
        k.jack_changed(ctx);
        return true;
    }
    static bool drag_scale(ViewerContext* ctx, Knob* p, int corner) {
        static double scale_ratio; // scale.y/scale.x
        Grid_Knob &k = *(Grid_Knob*)p;
        if (ctx->event() == PUSH) scale_ratio = k.jack_scale_y/k.jack_scale_x;
        if (ctx->event() != DRAG) return false;
        float mx = ctx->x();
        float my = ctx->y();
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
    static bool drag_scale_x(ViewerContext* ctx, Knob* p, int corner) {
        if (ctx->event() != DRAG) return false;
        Grid_Knob &k = *(Grid_Knob*)p;
        double scale = ctx->x()/(radius/dscale());
        scale = rint(scale*1000)/1000;
        if (corner) scale = -scale;
        double r = fabs(scale/k.jack_scale_y);
        if (!ctx->state(CTRL|SHIFT|ALT|META) && r > .9 && r < 1.1)
            scale = copysign(k.jack_scale_y, scale);
        k.jack_scale_x = scale;
        k.jack_changed(ctx);
        return true;
    }	
    static bool drag_scale_y(ViewerContext* ctx, Knob* p, int corner) {
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
    static bool drag_center(ViewerContext* ctx, Knob* p, int)
    {
        Grid_Knob &k = *(Grid_Knob*)p;
        if (ctx->event() == DROP) {
            if (strncmp(ctx->key_text(), ":nuke:values:", 13)==0) {
                float x = k.jack_tx+k.jack_cx, y = k.jack_ty+k.jack_cy;
                istringstream in(ctx->key_text()+13);
                int n = 0; in >> n;
                if (n>0) in >> x;
                if (n>1) in >> y;
                float dx = x-(k.jack_tx+k.jack_cx), dy = y-(k.jack_ty+k.jack_cy);
                for (int i=0; i<k.grid.height(); i++) {
                    for (int j=0; j<k.grid.width(i); j++) {
                        if (k.is_selected(ctx, xy_cb, ((i<<16)+j)<<3)) {
                            MPoint &p = k.grid.stroke[i].point[j];	
                            p.x += dx;
                            p.y += dy;
                        }
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
                float currentframe = k.uiContext().frame();
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
#if DD_IMAGE_VERSION_MAJOR < 5
                            k.maybe_set_key(k.frame());
#else
                            k.maybe_set_key();
#endif
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
                            for (int i=0; i<k.grid.height(); i++) {
                                for (int j=0; j<k.grid.width(i); j++) {
                                    if (k.is_selected(ctx, xy_cb, ((i<<16)+j)<<3)) {
                                        MPoint &p = k.grid.stroke[i].point[j];	
                                        p.x += dx;
                                        p.y += dy;
                                    }
                                }
                            }
#if DD_IMAGE_VERSION_MAJOR < 5
                            k.maybe_set_key(k.frame());
#else
                            k.maybe_set_key();
#endif
                        }
                    }
                }
                k.jack_tx += dxi; k.jack_ty += dyi; 
            } else return false;
            k.changed();
            return true;
        }
        if (ctx->event() != DRAG) return false;
        double dx = ctx->x()-k.jack_cx;
        double dy = ctx->y()-k.jack_cy;
        if (!dx && !dy) return false;
        k.jack_cx = ctx->x();
        k.jack_cy = ctx->y();
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
    int draw_xform_jack(DD::Image::ViewerContext *ctx) {
        // count the selected points in the grid
        int ix, iy, i, nSelected = 0;
        bool mmfound = false;
        float cx = 0.0f, cy = 0.0f, maxx=0.0f, maxy = 0.0f;
        selected_curve_ = 0;
        for (iy=0; iy<grid.height(); iy++) {
            for (ix=0; ix<grid.width(iy); ix++) {
                i = (iy<<16)+ix;
                if (is_selected(ctx, xy_cb, i<<3)) {
                    MPoint &p = grid.at(ix, iy);
                    if (mmfound) {
                        if (p.x<cx) cx = p.x;  if (p.x>maxx) maxx = p.x;
                        if (p.y<cy) cy = p.y;  if (p.y>maxy) maxy = p.y;
                    } else {
                        cx = maxx = p.x; cy = maxy = p.y;
                        mmfound = true;
                    }
                    nSelected++;
                    selected_curve_ = iy;
                }
            }
        }
        if (is_hide(selected_curve_))
	    return 0;
        selected_curve_good = nSelected;
        if (nSelected<2) { prevSelected = 0; return nSelected; }
        cx = (cx+maxx)*0.5f; cy = (cy+maxy)*0.5f;
        // reset the jack
        if (nSelected!=prevSelected) {
            jack_cx = cx; jack_cy = cy;
            jack_tx = 0.0f; jack_ty = 0.0f; jack_irotate = jack_rotate = 0;
            jack_skew = 0.0f; jack_scale_x = jack_scale_y = 1.0;
            grid.copy_to(&jack_grid);
            prevSelected = nSelected;
        }
        // draw the jack
        Matrix4 saved_matrix(ctx->modelmatrix);
        double x = jack_tx, y = jack_ty;
        ctx->modelmatrix.translate(jack_cx, jack_cy);
        // translation handle
        if (!(ctx->event()==PUSH && mod_ctrl)) {
            glLoadMatrixf(ctx->modelmatrix.array());
            make_handle(ctx, drag_translate, 0, x, y);
        }
        ctx->modelmatrix.translate(x, y);
        ctx->modelmatrix.scale(1/pixel_aspect, 1);
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
        if (ctx->event()==PUSH && mod_ctrl) {
            ctx->modelmatrix.scale(jack_scale_x, jack_scale_y, 1);
            ctx->modelmatrix.rotate(-jack_irotate*M_PI_2/90);
            ctx->modelmatrix.scale(pixel_aspect, 1, 1);
            ctx->modelmatrix.translate(-jack_cx, -jack_cy, 0);
            glLoadMatrixf(ctx->modelmatrix.array());
            make_handle(ctx, drag_center, 0, jack_cx, jack_cy);
        }
        ctx->modelmatrix = saved_matrix;
        glLoadMatrixf(ctx->modelmatrix.array());
        return nSelected;
    }
    // --- find the closest distance from click to spline ---
    float find_fraction(int index, float mx, float my) {
        int x = ((index>>16)&0x7fff)-1, y = ((index>>1)&0x7fff)-1;
        return find_fraction(x, y, mx, my);
    }
    // --- find the closest distance from click to spline ---
    float find_fraction(int x, int y, float mx, float my) {
        bool wrap = (x==grid.width(y)-1);
        int xx = wrap ? 0 : x+1;
#if DD_IMAGE_VERSION_MAJOR < 5
        mx = from_proxy_x(mx);
        my = from_proxy_y(my);
#else
        // changed to OutputContext object for proxy access in 5.0
	uiContext().from_proxy_xy( mx, my );
#endif
        float x1, y1, x2, y2, x3, y3, x4, y4;
        x1 = grid.at(x, y).x,   y1 = grid.at(x, y).y; 
        x2 = grid.at(x, y).ex,  y2 = grid.at(x, y).ey; 
        x4 = grid.at(xx, y).x,  y4 = grid.at(xx, y).y; 
        x3 = grid.at(xx, y).wx, y3 = grid.at(xx, y).wy; 
        float dm = 0.01f, dd = 10000.0f, mm = 0.5f;
        for (float m=0.0f+dm; m<1.0f; m+=dm) {
            float px = mx-bezier_at(m, x1, x2, x3, x4), 
                  py = my-bezier_at(m, y1, y2, y3, y4);
            float d = sqrt(px*px+py*py);
            if (d<dd) { dd = d; mm = m; }
        }
        return mm;
    }
    // --- user clicked on curve: ctrl+alt will create a new point ---
    bool handle_on_curve(ViewerContext* ctx, int ix, int iy) {
        if (mod_ctrl && mod_alt && !mod_shift) {
            int index = ((iy<<16)|(ix+1))<<3;
            if (ctx->event()==PUSH) {
                float f = find_fraction(ix, iy, ctx->x(), ctx->y());
                add_point(ix, iy, f);
                if (partner_) partner_->add_point(ix, iy, f);
                make_handle(
                        SELECTED_BY_THIS, ctx, xy_cb, index, ctx->x(), ctx->y());
                return true;
            } else if (ctx->event()==DRAG) {
                move_handle(ctx, index);
                return true;
            }
        }
        return false;
    }
    static bool handle_on_curve_cb(ViewerContext* ctx, Knob* data, int index) {
        int ix = index & 0xffff, iy = index >> 16;    
        return ((Grid_Knob*)data)->handle_on_curve(ctx, ix, iy);
    }
    //--- add a new curve starting at the clicked position ---
    bool handle_anywhere_new_curve(ViewerContext* ctx, int index);
    static bool handle_anywhere_new_curve_cb(ViewerContext* ctx, Knob* data, int index) {
        return ((Grid_Knob*)data)->handle_anywhere_new_curve(ctx, index);
    }
    //--- add a new point to the selected curve ---
    bool handle_anywhere_add_point(ViewerContext* ctx, int index) {
        if (ctx->button()==1 && mod_ctrl && mod_alt) {
            if (ctx->event()==PUSH) {
                int iy = selected_curve_;
                add_point(-1, iy, ctx->x(), ctx->y());
                smooth(-1, iy);
#if DD_IMAGE_VERSION_MAJOR < 5
                maybe_set_key(frame());
#else
                maybe_set_key();
#endif
                if (partner_) {
                    partner_->add_point(-1, iy, ctx->x(), ctx->y());
                    partner_->smooth(-1, iy);
#if DD_IMAGE_VERSION_MAJOR < 5
                    maybe_set_key(frame());
#else
                    maybe_set_key();
#endif
                }
                make_handle(SELECTED_BY_THIS, ctx, xy_cb, (iy<<16)<<3, ctx->x(), ctx->y());
            } else if (ctx->event()==DRAG) {
#if DD_IMAGE_VERSION_MAJOR < 5
                float x = from_proxy_x(ctx->x());
                float y = from_proxy_y(ctx->y());
#else
        	// changed to OutputContext object for proxy access in 5.0
                float x = ctx->x();
                float y = ctx->y();
		uiContext().from_proxy_xy( x, y );
#endif
                MPoint &p = grid.at(0, selected_curve_);
                p.x = x; p.y = y;
                set_point(0, selected_curve_, x, y);
                smooth(0, selected_curve_);
#if DD_IMAGE_VERSION_MAJOR < 5
                maybe_set_key(frame());
#else
                maybe_set_key();
#endif
            }
            return true;
        } 
        if (ctx->event()==DRAG) {
            int iy = selected_curve_, ix = 0;
            int index = ((iy<<16)|(ix))<<3;
            index++; // set the 'smooth' flag...
            move_handle(ctx, index); 
            return true;
        }
        return false;
    }

    static bool handle_anywhere_add_point_cb(ViewerContext* ctx, Knob* data, int index) {
        return ((Grid_Knob*)data)->handle_anywhere_add_point(ctx, index);
    }

    //--- move a point or handle ---  
    static bool xy_cb(ViewerContext* ctx, Knob* data, int index) {
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

    bool drop_handle(ViewerContext* ctx, int index) {
        int sub = index&7;
        if (sub!=0) return false; // allow only drag'n'drop onto main handles
        int i = index>>3, ix = i&0xffff, iy = i>>16;
        MPoint &p = grid.at(ix, iy);
        if (strncmp(ctx->key_text(), ":nuke:values:", 13)==0) {
            istringstream in(ctx->key_text()+13);
            double v; int n = 0; in >> n;
            if (n>0) { in >> v; p.x = v; }
            if (n>1) { in >> v; p.y = v; }
#if DD_IMAGE_VERSION_MAJOR < 5
            maybe_set_key(frame());
#else
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
#if DD_IMAGE_VERSION_MAJOR < 5
                        maybe_set_key(frame());
#else
                        maybe_set_key();
#endif
                    }
                }
            }
        } else return false;
        changed();
        return true;
    }

    //--- move a handle of a point ---
    bool move_handle(ViewerContext* ctx, int index);

    // Does a menu item to one point:
    bool do_menu(ViewerContext* ctx, int index);

    void set_track( Knob* track ) {
        this->track = track;
    }

    void transform_changed( int track_type ) {
#if DD_IMAGE_VERSION_MAJOR < 5
        transforms[ track_type ].evaluate( track, track->frame() );
#else
        // changed to OutputContext object for proxy access in 5.0
        transforms[ track_type ].evaluate( track, track->uiContext().frame() );
#endif
        update_selection();
        grid.revert_transform();
        grid.set_transform( transforms[ track_type ], track_type, selection );
        grid.apply_transform();
        for (unsigned i=0; i < gridlist.size(); i++) {
            gridlist[i].revert_transform();
            gridlist[i].set_transform( transforms[ track_type ], track_type, selection );
            gridlist[i].apply_transform();
        }
    }

    void transform_reseted( int track_type ) {
        update_selection();
        grid.revert_transform();
        grid.set_transform( transforms[ track_type ], track_type, selection );
        grid.apply_transform();
        for (unsigned i=0; i < gridlist.size(); i++) {
            gridlist[i].revert_transform();
            gridlist[i].set_transform( transforms[ track_type ], track_type, selection );
            gridlist[i].apply_transform();
        }
    }

    void update_transform( int track_type ) {
#if DD_IMAGE_VERSION_MAJOR < 5
        transforms[ track_type ].evaluate( track, track->frame() );
#else
        // changed to OutputContext object for proxy access in 5.0
        transforms[ track_type ].evaluate( track, track->uiContext().frame() );
#endif
    }

    void update_transforms( double frame ) {
        for ( int i = 0; i < TRACK_TYPE_COUNT; i++ ) {
            Transform& transform = transforms[ i ];
            transform.goto_frame( track, frame );
        }
        grid.revert_transform();
        grid.update_transform( track, frame );
        grid.apply_transform();
        for (unsigned i=0; i < gridlist.size(); i++) {
            gridlist[i].revert_transform();
            gridlist[i].update_transform( track, frame );
            gridlist[i].apply_transform();
        }
    }

    void reset_transform( int track_type ) {
        transforms[ track_type ].make_identity();
        transforms[ track_type ].to_knob( track );
	transform_reseted( track_type );
    }

    Transform const& get_transform( int track_type ) {
        return transforms[ track_type ];
    }

#ifdef DEBUG_TEST_KNOBS
    void print_transform() {
        std::cout << "transform hierarchy| " << reinterpret_cast< void* >( this ) << std::endl;
        for ( int i = 0; i < TRACK_TYPE_COUNT; i++ ) {
            Transform& transform = transforms[ i ];
            std::cout << i << "| " << transform << std::endl;
        }
        std::for_each( gridlist.begin(), gridlist.end(), std::mem_fun_ref( &MGrid::print_transform ) );
    }

    struct SelectionOperations {
        static void print( SelectionSet::value_type const& entry ) {
            int x = entry & 0xff;
            int y = entry >> 16;
            std::cout << y << " " << x << std::endl;
        }
    };

    void print_selection() {
        std::cout << "selection| " << reinterpret_cast< void* >( this ) << std::endl;
        update_selection();
        std::for_each( selection.begin(), selection.end(), SelectionOperations::print );
    }

    friend std::ostream& operator<< ( std::ostream& out, Grid_Knob const& knob );
#endif // DEBUG_TEST_KNOBS

#if DD_IMAGE_VERSION_MAJOR < 5
#else
    const char* Class() const { return "Grid_Knob"; }
#endif
};

/*! This may be called from a knobs() function in an Iop to make a
  control that updates a Bezier. */
Grid_Knob* Grid_knob(Knob_Callback f, MGrid* p, const char* name, const char* shapename) {
#if DD_IMAGE_VERSION_MAJOR < 5
    if (Knob::Make_Knobs) return new Grid_Knob(p, name, shapename);
    return (Grid_Knob*)f(INT_KNOB, p, name, 0, 0);
#else
    if (f.makeKnobs()) {
        return new Grid_Knob( &f, p, name, shapename);
    }
    return (Grid_Knob*)f( (int)INT_KNOB, Custom, p, name, 0, 0);
#endif
}


#ifdef DEBUG_TEST_KNOBS
std::ostream& operator<< ( std::ostream& out, Grid_Knob const& knob )
{
#if DD_IMAGE_VERSION_MAJOR < 5
    knob.to_script( out, Knob::TO_SCRIPT, false );
#else
    knob.to_script( out, &(knob.uiContext()), false );
#endif
    return out;
}
#endif // DEBUG_TEST_KNOBS


//==== WarpGeo ===============================================================

#include <DDImage/NullGeo.h>
#include <DDImage/Mesh.h>

class WarpGeo : public NullGeo {
    Iop* parent;
    Mesh* mmesh;

public:
    DD::Math::ThinPlateSpline<Vector3> uPlate, vPlate;
    MGrid *src, *dst; 	// storage for the source and destination grids
    int renderres;		// grids will be converted to a mesh with this number of subsections
    int mxres, myres; 	// total resolution of the mesh in x and y
    double distortion;	// multiply the amount of distortion by this value
    bool first_is_mask, reverse;
    /** Grids will be converted to a mesh with this number of subsections */
    int curveres;

#if DD_IMAGE_VERSION_MAJOR < 5
    WarpGeo(Iop* p) : NullGeo(), parent(p) {
#else
    WarpGeo( Node* node, Iop* p ) : NullGeo( node ), parent( p ) {
#endif
        mmesh = 0;
        src = dst = 0;
        renderres = 30;
        mxres = myres = 0;
        distortion = 1.0;
        first_is_mask = false;
        reverse = false;
        curveres = 7;
    }

    void copy_settings(WarpGeo *a) {
        src = a->src;
        dst = a->dst;
        renderres = a->renderres;
        first_is_mask = a->first_is_mask;
        reverse = !a->reverse;
        curveres = a->curveres;
    }

    /* Let the geometry know if we need to create new primitives: */
    void get_geometry_hash() {
        geo_hash[Group_Primitives].append(renderres);
        parent->full_size_format().append(geo_hash[Group_Points]);
        src->append(geo_hash[Group_Points]);
        dst->append(geo_hash[Group_Points]);
        geo_hash[Group_Points].append(distortion);
        geo_hash[Group_Points].append(first_is_mask);
        geo_hash[Group_Points].append(reverse);
    }

    void geometry_engine(Scene& scene, GeometryList& out) {
        if (rebuild(Mask_Primitives)) {
            out.delete_objects();
            out.add_object(0);

            // Add a simple Mesh:
            mmesh = new Mesh(renderres, renderres, false, false, QUADS);

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
            // first, create the thinplate splines
            int x, y, n = src->size();
            static vector<Vector3> u; u.clear(); u.reserve(n);
            static vector<Vector3> v; v.clear(); v.reserve(n);
            Vector3 up, vp;
            for (y=0; y<src->height(); y++) {
                int s = src->stroke[y].is_closed() ? 0 : 1 ;
                int w = src->width(y);
                if (w < 1) continue;
                for (x=s; x<w; x++) {
                    int x1 = x==0 ? w-1 : x-1;
                    MPoint &ps1 = src->at(x1, y), &ps2 = src->at(x, y);
                    MPoint &pd1 = dst->at(x1, y), &pd2 = dst->at(x, y);
                    if (y==0 && first_is_mask) { ps1=pd1; ps2=pd2; }
                    if (!reverse) {
                        up.set(pd1.x, pd1.y, ps1.x-pd1.x); u.push_back(up);
                        vp.set(pd1.x, pd1.y, ps1.y-pd1.y); v.push_back(vp);
                    } else {
                        up.set(ps1.x, ps1.y, pd1.x-ps1.x); u.push_back(up);
                        vp.set(ps1.x, ps1.y, pd1.y-ps1.y); v.push_back(vp);
                    }
                    if (curveres>1) for (int i=1; i<curveres; i++) { //++ proxyres
                        float m = ((float)i)/((float)curveres);
                        float pdx = bezier_at(m, pd1.x, pd1.ex, pd2.wx, pd2.x);
                        float pdy = bezier_at(m, pd1.y, pd1.ey, pd2.wy, pd2.y);
                        float psx = bezier_at(m, ps1.x, ps1.ex, ps2.wx, ps2.x);
                        float psy = bezier_at(m, ps1.y, ps1.ey, ps2.wy, ps2.y);
                        if (y==0 || !first_is_mask || dst->stroke[0].point_inside(psx, psy, curveres)) {
                            if (!reverse) {
                                up.set(pdx, pdy, psx-pdx); u.push_back(up);
                                vp.set(pdx, pdy, psy-pdy); v.push_back(vp);
                            } else {
                                up.set(psx, psy, pdx-psx); u.push_back(up);
                                vp.set(psx, psy, pdy-psy); v.push_back(vp);
                            }
                        }
                    }
                }
                if (s) {
                    x = w-1;
                    MPoint &ps = src->at(x, y), &pd = dst->at(x, y);
                    if (!reverse) {
                        up.set(pd.x, pd.y, ps.x-pd.x); u.push_back(up);
                        vp.set(pd.x, pd.y, ps.y-pd.y); v.push_back(vp);
                    } else {
                        up.set(ps.x, ps.y, pd.x-ps.x); u.push_back(up);
                        vp.set(ps.x, ps.y, pd.y-ps.y); v.push_back(vp);
                    }
                }
            }
            uPlate.setPoints(u);
            vPlate.setPoints(v);

            // now create the mesh
            PointList* points = out.writable_points(0);
            Attribute* uv = out.writable_attribute(0, Group_Points, "uv", VECTOR4_ATTRIB);
            assert(uv);

            const Format& f = parent->full_size_format();
            int xres = renderres, yres = renderres;
            bool mask = first_is_mask && dst->height()>0;
            for (y=0; y<yres; y++) {
                for (int x=0; x<xres; x++) {
                    int ix = mmesh->vertex_index(x, y);
                    Vector3& p = (*points)[ix];
                    Vector4& t = uv->vector4(ix);
                    float x0 = x, y0 = y;
                    // set the geometry according to the thinplate splines
                    float x1 = x0*f.width()/(xres-1), y1 = y0*f.height()/(yres-1);
#if 1
                    // deforms the texture while leaving the geometry alone
                    p.x = x1; 
                    p.y = y1;
                    if (!mask || dst->stroke[0].point_inside(x1, y1, curveres)) {
                        t.x = (x1+distortion*uPlate.value(x1, y1))/f.width(); 
                        t.y = (y1+distortion*vPlate.value(x1, y1))/f.height();
                    } else {
                        t.x = x1/f.width(); 
                        t.y = y1/f.height();
                    }
#else
                    // deforms the geometry while leaving the texture alone
                    if (!mask || dst->stroke[0].point_inside(x1, y1, curveres)) {
                        p.x = (x1-distortion*uPlate.value(x1, y1)); 
                        p.y = (y1-distortion*vPlate.value(x1, y1));
                    } else {
                        p.x = x1; 
                        p.y = y1;
                    }
                    t.x = x1/f.width(); 
                    t.y = y1/f.height();
#endif
                }
            }
        }

        GeoInfo& info = out[0];
        info.matrix.makeIdentity();
        info.material   = parent;
        info.display3d  = DISPLAY_TEXTURED;
        info.render_mode = RENDER_TEXTURED;
        info.selectable = true;
        info.selected   = false;
        info.source_geo = info.final_geo = info.select_geo = this;
    }

    void getfrompoint(double x1, double y1, Vector2& point) {
        bool mask = first_is_mask && dst->height()>0;
        if (!mask || dst->stroke[0].point_inside(x1, y1, curveres)) {
            point.x = x1+distortion*uPlate.value(x1, y1);
            point.y = y1+distortion*vPlate.value(x1, y1);
        } else {
            point.x = x1;
            point.y = y1;
        }
    }

    // modify passed box into the output box:
    void merge_bbox(Box& box) {
        for (int i=0; i<dst->height(); i++) {
            dst->stroke[i].merge_bbox(box);
            if (first_is_mask) return;
        }
    }

    const char* Class() const {return "InternalWarpGeo";}
    const char* node_help() const {return "internal";}
};

//==== SplineWarpMath =================================================================

/**
 * Grid based image warping operator
 */
class SplineWarpMath : public Render {
protected:
    WarpGeo warpgeo;			//!< Warp math

    Lock lock;

public:

#if DD_IMAGE_VERSION_MAJOR < 5
    SplineWarpMath() : Render(), warpgeo(this) {
#else
    SplineWarpMath( Node* node ) : Render( node ), warpgeo( node, this ) {
#endif
        warpgeo.src = new MGrid();
        warpgeo.src->color = 0xffaaaa00;
        warpgeo.dst = new MGrid();
        warpgeo.dst->color = 0xaaaaff00;
    }
#if DD_IMAGE_VERSION_MAJOR < 5
    SplineWarpMath(SplineWarpMath *sw) : Render(), warpgeo(this) {
#else
    SplineWarpMath(SplineWarpMath *sw) : Render( 0 ), warpgeo( 0, this ) {
#endif
        parent(sw);
        copy_settings(sw);
    }

    GeoOp* render_geo(int sample) {return &warpgeo;}

    void copy_settings(SplineWarpMath *a) {
        warpgeo.copy_settings(&a->warpgeo);
    }

    void _validate(bool for_real) {
        // Copy the base info from input0:
        warpgeo.invalidate();
        Render::_validate(for_real);
        // Fix up the bounding box by the rather crude guess of
        // forward-projecting some points on the edge:
        copy_info();
        warpgeo.merge_bbox(info_);
    }

    void distortion(double v) {warpgeo.distortion = v;}

    void _request(int x, int y, int r, int t, ChannelMask m, int s) {
        if (!warpgeo.distortion) input0().request(x,y,r,t,m,s);
        else input0().request(m,s);
    }

    void engine(int y, int x, int r, ChannelMask channels, Row& out_row) {
        if (!warpgeo.distortion) {
            input0().get(y,x,r,channels, out_row);
            return;
        }
        Vector2 newcenter;
        warpgeo.getfrompoint(x+.5, y+.5, newcenter);
        Pixel out(channels);
        Vector2 center, upone;
        for (; x < r; x++) {
            center = newcenter;
            warpgeo.getfrompoint(x+.5, y+1.5, upone);
            warpgeo.getfrompoint(x+1.5, y+.5, newcenter);
            input0().sample(center, newcenter-center, upone-center, out);
            foreach (z, channels) out_row.writable(z)[x] = out[z];
        }
    }

    const char* Class() const {return CLASSM;}
    const char* node_help() const {return "internal";}
};



//==== SplineWarpMath =================================================================

enum {SRC, SRCWARP, DST, DSTWARP, BLEND};
const char* const showtable[] = {"src", "srcwarp", "dst", "dstwarp", "blend", 0};


/**
 * Grid based image warping operator
 */
class SplineWarpIop : public SplineWarpMath {
    MGrid copypaste;
    SelectionSet copypaste_selection;
    float blend;
    SplineWarpMath *dst_iop;
    Grid_Knob *kSrcGrid, *kDstGrid;
    int showwhat;
    double distort[2];

    Knob* track;
    int track_type;

    int   dynamic_curv_count;
    int   the_curve_index;
    bool  show_the_curve_flag;
    bool show_source;
    bool show_destination;

public:

    void  increment_curve_count() {
        dynamic_curv_count++;
	knob("curve_count")->set_value(dynamic_curv_count);
	knob("index_the_curve")->set_value(dynamic_curv_count);
	knob("show_the_curve")->set_value(true);
    }
    void  decrement_curve_count(int index) {
        dynamic_curv_count--;
	knob("curve_count")->set_value(dynamic_curv_count);
	if (dynamic_curv_count == 0) {
	    the_curve_index = 0;
            knob("index_the_curve")->set_value(the_curve_index);
	    knob("show_the_curve")->set_value(true);
	} else if ((the_curve_index > dynamic_curv_count) ||
	           (index == the_curve_index - 1)) {
	    the_curve_index--;
	    if (the_curve_index == 0) {
	        the_curve_index = 1;
	    }
            knob("index_the_curve")->set_value(the_curve_index);
	    knob("show_the_curve")->
	        set_value(!(kSrcGrid->is_hide(the_curve_index - 1)) ||
		          !(kDstGrid->is_hide(the_curve_index - 1)));
        }
    }
    void set_bool_knob_by_curve_index(int index, bool v) {
        if (index == the_curve_index - 1) {
            show_the_curve_flag = v;
            knob("show_the_curve")->set_value(v);
	}
    }

// FIXME! {{
// lartola.20080117 This piece of code leaves undo/redo in a very
// flaky state. I'm disabling it for now until there's time to figure
// out what it's trying to do and why it's messing up the undo stack.
// 
#if 0
    void  update_curve_index(int index) {
        if (index == the_curve_index - 1)
	    return;
	the_curve_index = index + 1;
	knob("index_the_curve")->set_value(the_curve_index);
	knob("show_the_curve")->set_value(true);
    }
    void set_curve_swap_function(bool active) {
        if (active) {
	    knob("swap_the_curve")->enable();
	} else {
	    knob("swap_the_curve")->disable();
	}
    }
#else
    void  update_curve_index(int index) {
    }
    void set_curve_swap_function(bool active) {
    }
#endif
// FIXME! }}

protected:

    void makedst(bool for_real, double distortion) {
        if (!dst_iop) dst_iop = new SplineWarpMath(this);
        dst_iop->set_input(0, input(1));
        dst_iop->copy_settings(this);
        dst_iop->invalidate();
        dst_iop->distortion(distortion);
        dst_iop->validate(for_real);
    }

    void _validate(bool for_real) {
        switch (showwhat) {
            case SRC:
                copy_info(0);
                break;
            case SRCWARP:
                distortion(1);
                SplineWarpMath::_validate(for_real);
                break;
            case DST:
                copy_info(1);
                break;
            case DSTWARP:
                makedst(for_real, 1);
                info_ = dst_iop->info();
                break;
            case BLEND:
                if (blend<1.0) {
                    distortion(distort[0]);
                    SplineWarpMath::_validate(for_real);
                }
                if (blend>0.0) {
                    makedst(for_real, 1-distort[1]);
                    if (blend<1.0) {
                        info_.merge(dst_iop->info()); // merge changes by the dst_iop on top
                    } else {
                        info_ = dst_iop->info(); // set to dst_iop
                    }
                }
        }
    }

    void _request(int x, int y, int r, int t, ChannelMask m, int count) {
        switch (showwhat) {
            case SRC:
                input0().request(x,y,r,t,m,count);
                break;
            case SRCWARP:
                SplineWarpMath::_request(x, y, r, t, m, count);
                break;
            case DST:
                input1().request(x,y,r,t,m,count);
                break;
            case DSTWARP:
                dst_iop->_request(x, y, r, t, m, count);
                break;
            case BLEND:
                if (blend<1.0)
                    SplineWarpMath::_request(x, y, r, t, m, count);
                if (blend>0.0)
                    dst_iop->_request(x, y, r, t, m, count);
                break;
        }
    }

    void engine(int y, int x, int r, ChannelMask channels, Row& out) {
        switch (showwhat) {
            case SRC:
                input0().get(y,x,r,channels,out);
                break;
            case SRCWARP:
                SplineWarpMath::engine(y,x,r,channels,out);
                break;
            case DST:
                input1().get(y,x,r,channels,out);
                break;
            case DSTWARP:
                dst_iop->engine(y,x,r,channels,out);
                break;
            case BLEND: {
                            if (blend<1.0) {
                                SplineWarpMath::engine(y, x, r, channels, out);
                                if (blend<=0.0) {
                                    return;
                                }
                            } else {
                                dst_iop->engine(y, x, r, channels, out);
                                return;
                            }
                            // we need to blend both inputs. Src is already in 'out'
                            Row dr(x, r);
                            dst_iop->engine(y, x, r, channels, dr);
                            float b1 = 1.0f-blend, b2 = blend;
                            foreach(z, channels) {
                                const float *s1 = out[z]+x, *e = s1+r-x, *s2 = dr[z]+x;
                                float *dst = out.writable(z)+x;
                                while (s1<e) {
                                    *dst++ = b1 * *s1++ + b2 * *s2++;
                                }
                            }
                            break;
                        }
        }
    }

    void build_handles(ViewerContext* ctx) {
        switch (showwhat) {
            case BLEND: // should produce a blend of some kind
            case SRCWARP:
                validate(false);
                SplineWarpMath::build_handles(ctx);
                break;
            case DSTWARP:
                validate(false);
                dst_iop->build_handles(ctx);
                break;
            default:
                Iop::build_handles(ctx);
        }
    }

    int knob_changed(Knob* k);
    void knobs(Knob_Callback);

public:
#if DD_IMAGE_VERSION_MAJOR < 5
    SplineWarpIop() : SplineWarpMath()
#else
    SplineWarpIop( Node* node ) : SplineWarpMath( node )
#endif
    {
        inputs(2);
        blend = 0.0f;
        dst_iop = 0;
        showwhat = BLEND;
        kSrcGrid = kDstGrid = 0;
        distort[0] = distort[1] = 1.0;
        track = 0;
        track_type = TRACK_ALL;
	dynamic_curv_count = 0;
        the_curve_index = 0;
        show_the_curve_flag = true;
        show_source = false;
        show_destination = true;
    }

    const char *input_label(int i, char *buffer) const { return i?"dst":"src"; }
    const char* node_help() const {return HELP;}
    const char* Class() const {return CLASS;}
    static const Iop::Description d;

};


void SplineWarpIop::knobs(Knob_Callback f) {
    BeginGroup(f, "@b;Source Curves");
        Newline( f );
        Bool_knob( f, &show_source, "show_source", "show" );
        kSrcGrid = Grid_knob(f, warpgeo.src, "srcgrid", "srcshape");
        Newline(f, "clipboard");
        Script_knob(f, " ", "src_copy", "  copy  ");
        Tooltip(f, "Copy the current source curves into a temporary buffer. "
                "Source and destination curves share the same temporary buffer.");
        Script_knob(f, " ", "src_paste", " paste ");
        Tooltip(f, "Copy the curves from the temporary buffer onto the source curves.");
        Obsolete_knob(f, "src_hide", 0);
    EndGroup(f);

    BeginGroup(f, "@b;Destination Curves");
        Newline( f );
        Bool_knob( f, &show_destination, "show_destination", "show" );
        kDstGrid = Grid_knob(f, warpgeo.dst, "dstgrid", "dstshape");
        Newline(f, "clipboard");
        Script_knob(f, " ", "dst_copy", "  copy  ");
        Tooltip(f, "Copy the current destination curves into a temporary buffer. "
                "Source and destination curves share the same temporary buffer.");
        Script_knob(f, " ", "dst_paste", " paste ");
        Tooltip(f, "Copy the curves from the temporary buffer onto the "
                "destination curves.");
        Obsolete_knob(f, "dst_hide", 0);
    EndGroup(f);

    kSrcGrid->partner(kDstGrid);
    kDstGrid->partner(kSrcGrid);

    BeginGroup(f, "@b;Settings");
        Enumeration_knob(f, &showwhat, showtable, "showwhat", "show");
        Tooltip(f, "Select what image to output.");

        Bool_knob(f, &warpgeo.first_is_mask, "first_is_mask", "first bezier masks deformation");
        Tooltip(f, "The first bezier curve (curve #1) will be used as an outline "
                "for masking the SplineWarp's deformation. " );  
        Obsolete_knob(f, "style", 0); // old mapping/deform_texture control

        Int_knob(f, &warpgeo.curveres, "curveres", "curve resolution");
        Tooltip(f, "Increase this to improve the accuracy of how the warp matches the splines.");
        Int_knob(f, &warpgeo.renderres, "renderres", "preview resolution");
        Tooltip(f, "Increase this to improve the quality of the OpenGL preview.");

        Scale_knob(f, distort, IRange(0,1), "distortion");
        ClearFlags(f, Knob::LOG_SLIDER);
        Tooltip(f, "Multiply the overall distortion by this amount. "
                "Source and destination distortion can be set seperately.");
        Float_knob(f, &blend, "blend");
        Tooltip(f, "Blend between source and destination image.");
    EndGroup(f);

    BeginClosedGroup( f, "@b;Curve Controls" );
        // make a knob to show the number of curves
        Int_knob(f, &dynamic_curv_count, "curve_count", "number of curves");
        SetFlags(f, Knob::NO_ANIMATION|Knob::DISABLED);
        Tooltip(f, "Display the current number of curves.");
        Script_knob( f, " ", "show_all_curves", "  show all  " );
        Script_knob( f, " ", "hide_all_curves", "  hide all  " );
        Int_knob(f, &the_curve_index, "index_the_curve", "curve ID");
        SetFlags(f, Knob::NO_ANIMATION);
        Tooltip(f, "Display the ID of curve currently affected by the buttons "
                   "to the right.");
        Bool_knob(f, &show_the_curve_flag, "show_the_curve", " show ");
        SetFlags(f, Knob::NO_ANIMATION);
        Tooltip(f, "Toggles the curve labeled by the number in the \"curve ID\" "
                   " box on the left to be hidden.");
        Script_knob(f, 0, "swap_the_curve", " swap ");
        SetFlags(f, Knob::NO_ANIMATION);
        Tooltip(f, "Swap the source and destination curve labeled by the number "
                   "in the \"curve ID\" box on the left to be hidden. "
                   "This button is grayed out when any number of jacks are up. "
                   "Select a curve or change will always re-activate this button.");
    EndGroup( f );

    BeginClosedGroup( f, "@b;Tracking" );

        // Interactive transform jack
        Enumeration_knob( f, &track_type, track_type_labels, "track_type", "transform" );
        Newline( f );
        Script_knob( f, " ", "load_first_selected", " load first selected ");
        Script_knob( f, " ", "apply_to_selection", " apply to selection ");
        Script_knob( f, " ", "reset_selected", " reset selected ");
        Newline( f );
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
            | Knob::INVISIBLE
        );
    EndGroup( f );

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
            Script_knob( f, " ", "print_transform_tree", " print hierarchy ");
            SetFlags( f, debug_flags );
            Script_knob( f, " ", "print_selection", " print selection ");
            SetFlags( f, debug_flags );
        Newline( f );
            Script_knob( f, " ", "print_src_knob", " source script " );
            Script_knob( f, " ", "print_dst_knob", " destination script " );
    EndGroup( f );
    // Testing and debugging knobs }}
#endif // DEBUG_TEST_KNOBS

}

int SplineWarpIop::knob_changed(Knob* k) { 
#if (DD_IMAGE_VERSION_MAJOR > 4 && DD_IMAGE_VERSION_MINOR > 0)
    const char* name = k->name().data();
#else
    const char *name = k->name();
#endif
    if (!kSrcGrid || !kDstGrid ) return 1;
    if (strcmp("src_copy", name)==0) {
	kSrcGrid->update_selection();
        copypaste_selection = *(kSrcGrid->get_selection());
        warpgeo.src->copy_selection_to( &copypaste, copypaste_selection );
    } else if (strcmp("dst_copy", name)==0) {
	kDstGrid->update_selection();
        copypaste_selection = *(kDstGrid->get_selection());
        warpgeo.dst->copy_selection_to( &copypaste, copypaste_selection );
    } else if (strcmp("src_paste", name)==0) {
        kSrcGrid->paste_selection_from( copypaste, copypaste_selection );
    } else if (strcmp("dst_paste", name)==0) {
        kDstGrid->paste_selection_from( copypaste, copypaste_selection );
    } else if (strcmp("first_is_mask", name)==0) {
        if (k->get_value()) {
            kSrcGrid->first_is_mask(2);
            kDstGrid->first_is_mask(1);
        } else {
            kSrcGrid->first_is_mask(0);
            kDstGrid->first_is_mask(0);
        }
    } else if (k == &Knob::showPanel ||
            strcmp("show_source", name)==0 ||
            strcmp("show_destination", name)==0) {
        kSrcGrid->hide_grid( !show_source );
        kDstGrid->hide_grid( !show_destination );
        return 1;

    } else if (
        strcmp( "translate", name ) == 0
        || strcmp( "translate", name ) == 0
        || strcmp( "rotate", name ) == 0
        || strcmp( "scale", name ) == 0
        || strcmp( "skew", name ) == 0
        || strcmp( "center", name ) == 0
        || strcmp( "apply_to_selection", name ) == 0
        ) {

        kSrcGrid->transform_changed( track_type );
        kDstGrid->transform_changed( track_type );
        return 1;

    } else if (strcmp("track_type", name)==0) {

        Transform const& transform = kSrcGrid->get_transform( track_type );
        transform.to_knob( track );
        return 1;

    } else if (strcmp("load_first_selected", name)==0) {

        kDstGrid->update_selection();
        kSrcGrid->update_selection();

        Transform transform;
        if ( kDstGrid->is_grid_visible() ) {
            transform = kDstGrid->get_selection_transform( track_type );
        } else if ( kSrcGrid->is_grid_visible() ) {
            transform = kSrcGrid->get_selection_transform( track_type );
        } else { 
            transform.make_identity();
        }
        transform.to_knob( track );

        return 1;

    } else if (strcmp("reset_selected", name)==0) {

        kSrcGrid->reset_transform( track_type );
        kDstGrid->reset_transform( track_type );
        return 1;

#ifdef DEBUG_TEST_KNOBS
    } else if (strcmp("print_transform_tree", name)==0) {

        kSrcGrid->print_transform();
        kDstGrid->print_transform();

    } else if (strcmp("print_selection", name)==0) {

        kSrcGrid->print_selection();
        kDstGrid->print_selection();

    } else if (strcmp("print_transform", name)==0) {

        Transform transform;
#if DD_IMAGE_VERSION_MAJOR < 5
        transform.evaluate( track, k->frame() );
#else
        // changed to OutputContext object for proxy access in 5.0
        transform.evaluate( track, k->uiContext().frame() );
#endif
        std::cout << transform << std::endl;

    } else if (strcmp("print_src_knob", name)==0) {

        std::cout << name << "| " << *kSrcGrid << std::endl;

    } else if (strcmp("print_dst_knob", name)==0) {

        std::cout << name << "| " << *kDstGrid << std::endl;

#endif // DEBUG_TEST_KNOBS

    } else if (strcmp(name, "index_the_curve") == 0) {
        if (dynamic_curv_count == 0) {
	    the_curve_index = 0;
            k->set_value( the_curve_index );
	} else if (the_curve_index <= 0) {
	    the_curve_index = 1;
            k->set_value( the_curve_index );
	} else if (the_curve_index > dynamic_curv_count) {
	    the_curve_index = dynamic_curv_count;
            k->set_value( the_curve_index );
        }
	if (the_curve_index != 0) {
	    knob("show_the_curve")->
	        set_value( !(kSrcGrid->is_hide(the_curve_index - 1)) ||
		           !(kDstGrid->is_hide(the_curve_index - 1)));
	}
	else {
	    knob("show_the_curve")->set_value(true);
	}
	knob("swap_the_curve")->enable();
    } else if (strcmp(name, "show_the_curve") == 0) {
	if (the_curve_index <= 0) {
	    k->set_value(true);
	    return 1;
	}
        int iy = the_curve_index - 1;
        kSrcGrid->set_stroke_visible( iy, show_the_curve_flag );
        kDstGrid->set_stroke_visible( iy, show_the_curve_flag );
    } else if (strcmp(name, "swap_the_curve") == 0) {
	if (the_curve_index <= 0)
	    return 1;
	kSrcGrid->swap_stroke( the_curve_index - 1 , true );

    } else if (strcmp("show_all_curves", name)==0) {

        kSrcGrid->set_all_stroke_visible();
        kDstGrid->set_all_stroke_visible();
	if (the_curve_index != 0) {
	    knob("show_the_curve")->
	        set_value( !(kSrcGrid->is_hide(the_curve_index - 1)) ||
		           !(kDstGrid->is_hide(the_curve_index - 1)));
	}

    } else if (strcmp("hide_all_curves", name)==0) {

        kSrcGrid->set_all_stroke_visible( false );
        kDstGrid->set_all_stroke_visible( false );
	if (the_curve_index != 0) {
	    knob("show_the_curve")->
	        set_value( !(kSrcGrid->is_hide(the_curve_index - 1)) ||
		           !(kDstGrid->is_hide(the_curve_index - 1)));
	}

    } else
        return 0;
    return 1;
}


#if DD_IMAGE_VERSION_MAJOR < 5
static Iop* build() {return new SplineWarpIop();}
#else
static Iop* build( Node* node ) {return new SplineWarpIop( node );}
#endif

const Iop::Description SplineWarpIop::d(CLASS, "Transform/SplineWarp", build);

// moved here to handle SplineWarpIop forward reference
//--- draw handle 
void Grid_Knob::draw_handle(DD::Image::ViewerContext *ctx) {
#if DD_IMAGE_VERSION_MAJOR < 5
    goto_frame(frame());
    pixel_aspect = proxy_pixel_aspect();
#else
    // changed to OutputContext object for proxy access in 5.0
    OutputContext uiCtx = uiContext();
    goto_frame( (float)uiCtx.frame() );
    uiCtx.to_proxy_pixel_aspect( pixel_aspect );
#endif
    if (ctx->event()==PUSH) {
        mod_ctrl  = ctx->state(COMMAND);
        mod_alt   = ctx->state(OPTION);
        mod_shift = ctx->state(SHIFT);
    }
    draw_handle_edit(ctx);
    if (selected_curve_good) {
        SplineWarpIop* spline = reinterpret_cast< SplineWarpIop* >( this->op() );
        if ( spline ) {
            spline->update_curve_index(selected_curve_);
	    if (selected_curve_good > 1) {
	        spline->set_curve_swap_function(false);
	    } else {
	        spline->set_curve_swap_function(true);
	    }
        }
    }
}
//--- move a handle of a point ---
bool Grid_Knob::move_handle(ViewerContext* ctx, int index) {
    int i = index>>3, ix = i&0xffff, iy = i>>16;
    bool smoothflag = false;
    MPoint &p = grid.at(ix, iy);
    int sub = index&7;
    double x, y;
    switch (sub) {
        case 1: sub = 0; smoothflag = true; // fall through
        case 0: x = p.x; y = p.y; break;
        case 3: x = p.x+p.ex; y = p.y+p.ey; break;
        case 4: x = p.x+p.wx; y = p.y+p.wy; break;
    }

    if (ctx->event()==KEY) {
        if (!ctx->nudgeXY(x, y)) return false;
        new_nudge_undo();
    } else if (ctx->event() == DRAG) {
#if DD_IMAGE_VERSION_MAJOR < 5
        x = from_proxy_x(ctx->x());
        y = from_proxy_y(ctx->y());
#else
        // changed to OutputContext object for proxy access in 5.0
        x = ctx->x();
        y = ctx->y();
	uiContext().from_proxy_xy( x, y );
#endif
    } else if (ctx->event() == PUSH && smoothflag) {
        x = p.x; y = p.y;
    } else {
        return false;
    }

    bool  gang = (ctx->state() & SHIFT);
    bool rgang = !gang && !(ctx->state() & CTRL);
    switch (sub) {
        case 0: 
            p.x = x; p.y = y; 
            if (smoothflag) {
                grid.smooth(ix, iy);
            }
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
    maybe_set_key();
#endif
    return true;
}
//--- add a new curve starting at the clicked position ---
bool Grid_Knob::handle_anywhere_new_curve(ViewerContext* ctx, int index) {
    if (ctx->event()==PUSH) {
        int iy = add_curve();
        add_point(-1, iy, ctx->x(), ctx->y());
        if (partner_) {
            iy = partner_->add_curve();
            partner_->add_point(-1, iy, ctx->x(), ctx->y());
        }
        SplineWarpIop* spline = reinterpret_cast< SplineWarpIop* >( this->op() );
        if ( spline ) {
            spline->increment_curve_count();
        }
        make_handle(SELECTED_BY_THIS, ctx, xy_cb, (iy<<16)<<3, ctx->x(), ctx->y());
        return true;
    } else if (ctx->event()==DRAG) {
        int iy = grid.height()-1, ix = 0;
        int index = ((iy<<16)|(ix))<<3;
        move_handle(ctx, index);
        return true;
    }
    return false;
}

bool Grid_Knob::do_menu(ViewerContext* ctx, int index)
{
    int n = index>>3, ix = n&0xffff, iy = n>>16;
    int nPoints = grid.width(iy);
    // Call do_menu on all selected points:
    static bool recurse = false;
    if (!recurse && ShapeKnob::is_selected(ctx, xy_cb, n)) {
        recurse = true;
        for (int i = nPoints; i--;)
            if (ShapeKnob::is_selected(ctx, xy_cb, ((iy<<16)+i)<<3)) do_menu(ctx, i);
        recurse = false;
        return true;
    }
    // moved assert inside case's to avoid spurious crashes

    // transform cooked grid back to source space:
    switch (ctx->event()) {

    case SMOOTH:
        assert(ix>=0 && ix < nPoints);
        grid.stroke[iy].smooth(ix);
#if DD_IMAGE_VERSION_MAJOR < 5
        maybe_set_key(frame());
#else
        maybe_set_key();
#endif
        return true;

    case CUSP:
        assert(ix>=0 && ix < nPoints);
        grid.stroke[iy].cusp(ix);
#if DD_IMAGE_VERSION_MAJOR < 5
        maybe_set_key(frame());
#else
        maybe_set_key();
#endif
        return true;

    case ShapeKnob::SET_KEY:
    case ShapeKnob::DELETE_KEY:
    case ShapeKnob::COPY_KEY:
    case ShapeKnob::PASTE_KEY:
        assert(ix>=0 && ix < nPoints);
#if DD_IMAGE_VERSION_MAJOR < 5
        return handle(ctx->event(), frame());
#else
#if (DD_IMAGE_VERSION_MAJOR > 4 && DD_IMAGE_VERSION_MINOR > 0)
        return handle(ctx->event(), uiContext().frame());
#else
        return handle(ctx->event());
#endif
#endif

    case SELECTALL: {
        assert(ix>=0 && ix < nPoints);
        int i;
        for (i=0; i < nPoints; i++)
            if (!ShapeKnob::is_selected(ctx, xy_cb, ((iy<<16)+i)<<3)) break;
        if (i >= nPoints) return false; // all selected
        for (i = 0; i < nPoints; i++) {
            MPoint& p = grid.stroke[iy].point[i];
            make_handle(SELECTED_BY_THIS, ctx, xy_cb, ((iy<<16)+i)<<3, p.x, p.y);
        }
        return true;
    }

    case DELETE_POINT: 
        assert(ix>=0 && ix < nPoints);
        delete_point(ix, iy); 
        if (partner_) partner_->delete_point(ix, iy);
        return true;

    case DELETE_CURVE: {
        assert(ix>=0 && ix < nPoints);
        delete_stroke(iy);
        if (partner_) partner_->delete_stroke(iy);
        SplineWarpIop* spline = reinterpret_cast< SplineWarpIop* >( this->op() );
        if ( spline ) {
            spline->decrement_curve_count(ix);
        }
        return true;
    }

    case OPEN_CLOSE_CURVE: 
        assert(ix>=0 && ix < nPoints);
        flip_closed(iy);
        if (partner_) partner_->flip_closed(iy);
        return true;

    case HIDE_CURVE: {
        assert(ix>=0 && ix < nPoints);
        set_stroke_visible( iy, false );
        if (partner_) partner_->set_stroke_visible( iy, false );
        SplineWarpIop* spline = reinterpret_cast< SplineWarpIop* >( this->op() );
        if ( spline ) {
            spline->set_bool_knob_by_curve_index(iy, false);
        }
        return true;
    }

    case SWAP_CURVES:
       assert(ix>=0 && ix < nPoints);
       swap_stroke(iy);
       return true;

    default:
        return false;

    }

    return false;
}
