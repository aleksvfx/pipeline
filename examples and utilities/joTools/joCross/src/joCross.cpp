/// \file joCross.cpp
/// \author Johannes Saam
/// \brief Implementation of a cross product node for nuke

/// \param char to definde the command name
static const char* const CLASS = "joCross";

/// \param char to definde the help context
static const char* const HELP =
"calculates the cross product between two input"
"Vectors based coded in RGB = XYZ"
"\n\n\n";


#include <DDImage/Iop.h>
#include <DDImage/Row.h>
#include <DDImage/Tile.h>
#include <DDImage/Knobs.h>
#include "Point3.h"
#include "Vector.h"

/// \namespace DD::Image
using namespace DD::Image;

/// \class joCross
/// \brief the Vector Cross Product Class
class joCross : public Iop {

public:
	/// \brief ctor
	joCross();
	/// \param int image width
	int width;
	bool normalize;
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

joCross::joCross() : Iop() 
{
	/// set the inputs to 2
	inputs( 2 );
	/// set the standarv values for the variables
	normalize = false;
}
void joCross::knobs(Knob_Callback f) 
{
	Bool_knob( f, &normalize, "Normalize" );
}


static Iop* joCross_c() 
{
	return new joCross();
}

const Iop::Description joCross::d(CLASS, "Filter/joCross", joCross_c);

void joCross::_validate(bool for_real)
{
	copy_info();
	input1().validate( for_real );
	set_out_channels( Mask_All );
	Format temp = info_.format();
	width = temp.width();
	info_.set( info_.x(), info_.y(), info_.r(), info_.t() );
}

void joCross::_request(int x,int y,int r,int t, ChannelMask channels, int count)
{
	input0().request(x,y,r,t, channels, count);
	input1().request(x,y,r,t, channels, count);
}

void joCross::engine(int y, int x, int r, ChannelMask channels, Row& out)
{
	/// set re allocate the out row to prevent from crashing while zooming
	out.range( 0, width );
	/// get and load the inputs
	Row in1(x, r);
	Row in2(x, r);

	input0().get(y, x, r, channels, in1);
	input1().get(y, x, r, channels, in2);
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
			Vector one( in1[Chan_Red][index], in1[Chan_Green][index],in1[Chan_Blue][index] );
			Vector two( in2[Chan_Red][index], in2[Chan_Green][index], in2[Chan_Blue][index]);
			/// check the inputs
			if ( one.length() > 0.0001 && two.length() > 0.0001 )
			{
				/// cross the input
				Vector result = one.cross( two );
				/// normalize it
				if( result.length() > 0.0001 && normalize )
					result.normalize();
				/// set the output
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
