/// \file joNormalize.cpp
/// \author Johannes Saam
/// \brief Implementation of a Vector normalize node for nuke

/// \param char to definde the command name
static const char* const CLASS = "joNormalize";

/// \param char to definde the help context
static const char* const HELP =
"Normalizes a Vector input"
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

/// \class joNormalize
/// \brief the Vector Normalization Class
class joNormalize : public Iop {

public:
	/// \brief ctor
	joNormalize();
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

joNormalize::joNormalize() : Iop() 
{
	/// set the inputs to 1
	inputs( 1 );
}
void joNormalize::knobs(Knob_Callback f) 
{
	;
}


static Iop* joNormalize_c() 
{
	return new joNormalize();
}

const Iop::Description joNormalize::d(CLASS, "Filter/joNormalize", joNormalize_c);

void joNormalize::_validate(bool for_real)
{
	copy_info();
	set_out_channels( Mask_All );
	Format temp = info_.format();
	width = temp.width();
	info_.set( info_.x(), info_.y(), info_.r(), info_.t() );
}

void joNormalize::_request(int x,int y,int r,int t, ChannelMask channels, int count)
{
	input0().request(x,y,r,t, channels, count);
}

void joNormalize::engine(int y, int x, int r, ChannelMask channels, Row& out)
{
	/// set re allocate the out row to prevent from crashing while zooming
	out.range( 0, width );
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
		/// steo thru the tile and do the magic
		for (int index = x; index < r; index++ ) 
		{
			Vector one( in1[Chan_Red][index], in1[Chan_Green][index],in1[Chan_Blue][index] );
			
			if ( one.length() > 0.0001 )
			{
				Vector result = one;

				result.normalize();

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
