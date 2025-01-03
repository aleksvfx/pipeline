/// \file joGradVec.cpp
/// \author Johannes Saam
/// \brief Implementation of a node that calculates the perpendicular vector to the red channel

/// \param char to definde the command name
static const char* const CLASS = "joGradVec";

/// \param char to definde the help context
static const char* const HELP =
"Conferts a position vector image to normals"
"by crossing the image gradients in X Y and Z"
"\n\n\n";


#include <DDImage/Iop.h>
#include <DDImage/Row.h>
#include <DDImage/Tile.h>
#include <DDImage/Knobs.h>
#include "Point3.h"
#include "Vector.h"
/// \namespace DD::Image
using namespace DD::Image;

/// \class joGradVec
/// \brief the R to perpendicual Class
class joGradVec : public Iop {
	
	
public:
	/// \brief ctor
	joGradVec();
	/// \param bool flip X
	bool flipX;
	/// \param bool flip Y
	bool flipY;
	/// \param bool flip Z
	bool flipZ;
	/// \param int image width
	int width;
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

joGradVec::joGradVec() : Iop() 
{	
	/// set the inputs to 1
	inputs( 1 );
	/// set the standarv values for the variables
	flipX = false;
	flipY = false;
	flipZ = false;

	width = 0;

}

void joGradVec::knobs(Knob_Callback f) 
{
	Bool_knob( f, &flipX, "FLIP X\n" );
	Bool_knob( f, &flipY, "FLIP Y\n" );
	Bool_knob( f, &flipZ, "FLIP Z\n" );

	
}

static Iop* joGradVec_c() {return new joGradVec();}

const Iop::Description joGradVec::d(CLASS, "Filter/joGradVec", joGradVec_c);

void joGradVec::_validate(bool for_real)
{
	copy_info();

	set_out_channels( Mask_All );
	

	info_.set( info_.x(), info_.y(), info_.r() + 2, info_.t() + 2 );
	Format temp = info_.format();
	width = temp.width();

}

void joGradVec::_request(int x,int y,int r,int t, ChannelMask channels, int count)
{
	input0().request(x,y,r+1,t+1, channels, count);
}

void joGradVec::engine(int y, int x, int r, ChannelMask channels, Row& out)
{
	/// set re allocate the out row to prevent from crashing while zooming
	out.range( 0, width );
	/// get and load the inputs
	Row in1(x, r);
	Row in2(x, r);


	input0().get(y, x, r, channels, in1);
	input0().get(y + 1, x, r, channels, in2);


	float flipperX = 1.0f;
	float flipperY = 1.0f;
	float flipperZ = 1.0f;
	if( flipX )
		flipperX = -1.0f;
	if( flipY )
		flipperY = -1.0f;
	if( flipZ )
		flipperZ = -1.0f;
	/// step thru all the aviable channels
	foreach (z, channels) 
	{
		/// generate the output pointers
		float* outPixelsR = out.writable(Chan_Red);
		float* outPixelsG = out.writable(Chan_Green);
		float* outPixelsB = out.writable(Chan_Blue);
		/// steo thru the tile and do the magic
		for (int index = x; index < r; index++ ) 
		{
			/// get the inputs
			float X1 = in1[Chan_Red][index + 1];
			float C0 = in1[Chan_Red][index];
			float Y1 = in2[Chan_Red][index];
			/// calculate the image gradients
			float vecX = X1 - C0;
			float vecY = Y1 - C0;
			/// set the output vector perpendiculat to the brightness
			Vector result( -vecX, -vecY, 0.0f );
			/// normalize it
			if ( result.length() != 0 )
				result.normalize();
			/// set the outputs
			result.x = result.x * flipperX;
			result.y = result.y * flipperY;
			result.z = result.z * flipperZ;

			outPixelsR[index] = result.x;
			outPixelsG[index] = result.y;
			outPixelsB[index] = result.z;
		}
	}

}
