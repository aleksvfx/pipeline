/// \file z2Pos.cpp
/// \author Johannes Saam
/// \brief Implementation of z depth to wold position node

/// \param char to definde the command name
static const char* const CLASS = "z2Pos";

/// \param char to definde the help context
static const char* const HELP =
"Converts a z channel into position data based on the cameras fov"
"Doesn`t clamp the values yet... so use a color comress node after"
"\n\n\n";


#include <DDImage/Iop.h>
#include <DDImage/Row.h>
#include <DDImage/Tile.h>
#include <DDImage/Knobs.h>
#include "Point3.h"
#include "Vector.h"
/// \namespace DD::Image
using namespace DD::Image;

/// \class z2pos
/// \brief the z depth to position Class
class z2Pos : public Iop {
	float fov;
	int width;
	int height;
	
public:
	/// \brief ctor
	z2Pos();
	/// \brief valitadion for the node
	void _validate(bool);
	/// \brief request to the nodes inputs
	void _request(int,int,int,int,ChannelMask,int);
	/// \brief compute the nodes oututs
	void engine(int y,int x,int r, ChannelMask, Row&);
	/// \brief consturct the nodes interface
	virtual void knobs(Knob_Callback);
	/// \brief description class
	const char* Class() const {return CLASS;}
	/// \brief description class
	const char* node_help() const {return HELP;}
	/// \brief description class
	static const Description d;
};

z2Pos::z2Pos() : Iop() 
{
	/// set the inputs to 1
	inputs( 1 );
	/// set the standarv values for the variables
	fov = 35.0;
}

void z2Pos::knobs(Knob_Callback f) 
{
	Float_knob( f, &fov, "fov" );
}

static Iop* z2Pos_c() {return new z2Pos();}

const Iop::Description z2Pos::d(CLASS, "Filter/z2Pos", z2Pos_c);

void z2Pos::_validate(bool for_real)
{
	copy_info();

	set_out_channels( Mask_All );
	

	info_.set( info_.x(), info_.y(), info_.r(), info_.t() );
	Format temp = info_.format();

	width = temp.width();
	height = temp.height();
}

void z2Pos::_request(int x,int y,int r,int t, ChannelMask channels, int count)
{
	input0().request(x,y,r,t, channels, count);
}

void z2Pos::engine(int y, int x, int r, ChannelMask channels, Row& out)
{
	/// set re allocate the out row to prevent from crashing while zooming
	out.range(0, width);
	/// get and load the inputs
	Row in1(x, r);

	input0().get(y, x, r, channels, in1);
	/// step thru all the aviable channels
	foreach (z, channels) 
	{
		/// generate the output pointers
		float* outPixelsR = out.writable(Chan_Red);
		float* outPixelsG = out.writable(Chan_Green);
		float* outPixelsB = out.writable(Chan_Blue);


		float fovPrepX = sin( fov / 2 );
		float fovStepX = fovPrepX * 2 / (float)width;

		float fovPrepY = sin( fov / 2 ) * (float)width / (float)height;
		float fovStepY = fovPrepY * 2 / (float)height;
		/// steo thru the tile and do the magic
		for (int index = x; index < r; index++ ) 
		{
			/// get the input
			float depth = in1[Chan_Red][index];
			/// set the x pos based on the focal length
			float xValue = fovPrepX - index * fovStepX;
			/// set the y pos based on the focal length
			float yValue = fovPrepY - y * fovStepY;
			/// set the output
			outPixelsR[index] = depth * xValue;
			outPixelsG[index] = depth * yValue;
			outPixelsB[index] = depth;
		}
	}

}
