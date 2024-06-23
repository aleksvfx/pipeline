/// \file joSTMapper.cpp
/// \author Johannes Saam
/// \brief Implementation of a simple st remapper

/// \param char to definde the command name
static const char* const CLASS = "joSTMapper";

/// \param char to definde the help context
static const char* const HELP =
"get the reraction vector"
"based on the index of refraction"
"\n\n\n";


#include <DDImage/Iop.h>
#include <DDImage/Row.h>
#include <DDImage/Tile.h>
#include <DDImage/Knobs.h>
#include "Point3.h"
#include "Vector.h"
/// \namespace DD::Image
using namespace DD::Image;

/// \class joSTMapper
/// \brief the simple st mapper Class
class joSTMapper : public Iop {

public:
	/// \param int image width
	int width;
/// \param int image height
	int height;
	/// \param int map image width
	int mapWidth;
	/// \param int map image height
	int mapHeight;
	/// \brief ctor
	joSTMapper();
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

joSTMapper::joSTMapper() : Iop() 
{
	/// set the inputs to 2
	inputs( 2 );

}
void joSTMapper::knobs(Knob_Callback f) 
{

}


static Iop* joSTMapper_c() 
{
	return new joSTMapper();
}

const Iop::Description joSTMapper::d(CLASS, "Filter/joSTMapper", joSTMapper_c);

void joSTMapper::_validate(bool for_real)
{
	copy_info();
	input1().validate( for_real );

	set_out_channels(Mask_All);
	Format temp = input0().format();
	width = temp.width();
	height = temp.height();
	info_.set( info_.x(), info_.y(), info_.r(), info_.t() );
	
	Format mapFormat  = input1().format();
	mapWidth = mapFormat.width();
	mapHeight = mapFormat.height();

}

void joSTMapper::_request(int x,int y,int r,int t, ChannelMask channels, int count)
{
	input0().request(x,y,r,t, channels, count);
	input1().request(0,mapHeight,mapWidth,mapHeight, channels, count);
	
}

void joSTMapper::engine(int y, int x, int r, ChannelMask channels, Row& out)
{
	/// set re allocate the out row to prevent from crashing while zooming
	out.range( 0, width );
	/// get and load the inputs
	Row in1(x, r);
	/// step thru all the aviable channels	
	input0().get(y, x, r, channels, in1);
	
	foreach (z, channels) 
	{
		/// generate the output pointers
		float* outPixelsR = out.writable(Chan_Red);
		float* outPixelsG = out.writable(Chan_Green);
		float* outPixelsB = out.writable(Chan_Blue);
		/// step thru the tile and do the magic
		for (int index = x; index < r; index++ )
		{
			/// get the st coord input
			Vector ST( in1[Chan_Red][index], in1[Chan_Green][index], 0.0f );
			
			/// check the input
			if ( ST.length() > 0.0001f )
			{
				/// convert the st coordinates to image space of the input image
				int xMap = (int)( (float)mapWidth * ST.x );
				int yMap = (int)( (float)mapHeight * ST.y );
				/// get the input at the roght coordinates
				Row in2(0, mapWidth);
				input1().get( yMap, 0, xMap, turn_on(channels, Mask_RGB), in2);
				in2.range( 0, mapWidth );
				/// set the output
				outPixelsR[index] = ( in2[Chan_Red][xMap - 1] + in2[Chan_Red][xMap] + in2[Chan_Red][xMap + 1] ) / 3;
				outPixelsG[index] = ( in2[Chan_Green][xMap - 1] + in2[Chan_Green][xMap] + in2[Chan_Green][xMap + 1] ) / 3;
				outPixelsB[index] = ( in2[Chan_Blue][xMap - 1] + in2[Chan_Blue][xMap] + in2[Chan_Blue][xMap + 1] ) / 3;

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
