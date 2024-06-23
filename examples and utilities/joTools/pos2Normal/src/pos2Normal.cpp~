/// \file jopos2Normal.cpp
/// \author Johannes Saam
/// \brief Implementation of a world space to normal node

/// \param char to definde the command name
static const char* const CLASS = "pos2Normal";

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

/// \class pos2Normal
/// \brief the position two normal Class
class pos2Normal : public Iop {
	
	
public:
	/// \brief ctor
	pos2Normal();
	/// \param bool flip the X coordinate
	bool flipX;
	/// \param bool flip the Y coordinate
	bool flipY;
	/// \param bool flip the Z coordinate
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

pos2Normal::pos2Normal() : Iop() 
{
	/// set the inputs to 1
	inputs( 1 );
	/// set the standarv values for the variables
	flipX = false;
	flipY = false;
	flipZ = false;

	width = 0;

}

void pos2Normal::knobs(Knob_Callback f) 
{
	Bool_knob( f, &flipX, "FLIP X\n" );
	Bool_knob( f, &flipY, "FLIP Y\n" );
	Bool_knob( f, &flipZ, "FLIP Z\n" );

	
}

static Iop* pos2Normal_c() {return new pos2Normal();}

const Iop::Description pos2Normal::d(CLASS, "Filter/pos2Normal", pos2Normal_c);

void pos2Normal::_validate(bool for_real)
{
	copy_info();

	set_out_channels( Mask_All );
	

	info_.set( info_.x(), info_.y(), info_.r() + 1, info_.t() + 1 );
	Format temp = info_.format();
	width = temp.width();

}

void pos2Normal::_request(int x,int y,int r,int t, ChannelMask channels, int count)
{
	input0().request(x,y,r,y+1, channels, count);
}

void pos2Normal::engine(int y, int x, int r, ChannelMask channels, Row& out)
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
			Vector X1( in1[Chan_Red][index + 1], in1[Chan_Green][index + 1], in1[Chan_Blue][index + 1] );
			Vector C0( in1[Chan_Red][index], in1[Chan_Green][index], in1[Chan_Blue][index] );
			Vector Y1( in2[Chan_Red][index], in2[Chan_Green][index], in2[Chan_Blue][index] );
			/// check the input
			if ( C0.length() > 0.0001f && X1.length() > 0.0001f && Y1.length() > 0.0001f )
			{
				/// get the image gradient in x and y
				Vector vecX = X1 - C0;
				Vector vecY = Y1 - C0;
				/// cross it and normalize
				Vector result = vecX.cross( vecY );
				if ( result.length() != 0 )
					result.normalize();
				/// set the output
				result.x = result.x * flipperX;
				result.y = result.y * flipperY;
				result.z = result.z * flipperZ;

				outPixelsR[index] = result.x;
				outPixelsG[index] = result.y;
				outPixelsB[index] = result.z;
			} 
			else
			{
				outPixelsR[index] = 0.0f;
				outPixelsG[index] = 0.0f;
				outPixelsB[index] = 0.0f;
			}
		}
	}

}
