/// \file joReflect.cpp
/// \author Johannes Saam
/// \brief Implementation of a reflect Vector node for nuke

/// \param char to definde the command name
static const char* const CLASS = "joReflect";

/// \param char to definde the help context
static const char* const HELP =
"get the reflection vector"
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

/// \class joReflect
/// \brief the Vector Reflection Class
class joReflect : public Iop {

public:
	/// \brief ctor
	joReflect();
	/// \param float index of refraction
	float IOR;
	/// \param bool switch for fresnel
	bool fresnel;
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

joReflect::joReflect() : Iop() 
{
	/// set the inputs to 2
	inputs( 2 );
	/// set the standarv values for the variables
	fresnel = false;
	IOR = 1.6666f;


}
void joReflect::knobs(Knob_Callback f) 
{
	Bool_knob( f, &fresnel, "Do fresnel" );
	Float_knob( f, &IOR, "Index of refraction" );
}


static Iop* joReflect_c() 
{
	return new joReflect();
}

const Iop::Description joReflect::d(CLASS, "Filter/joReflect", joReflect_c);

void joReflect::_validate(bool for_real)
{
	copy_info();
	input1().validate( for_real );
	set_out_channels( Mask_All );
	Format temp = info_.format();
	width = temp.width();
	info_.set( info_.x(), info_.y(), info_.r(), info_.t() );
}

void joReflect::_request(int x,int y,int r,int t, ChannelMask channels, int count)
{
	input0().request(x,y,r,t, channels, count);
	input1().request(x,y,r,t, channels, count);
}

void joReflect::engine(int y, int x, int r, ChannelMask channels, Row& out)
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
			Vector N( in1[Chan_Red][index], in1[Chan_Green][index],in1[Chan_Blue][index] );
			Vector I( in2[Chan_Red][index], in2[Chan_Green][index], in2[Chan_Blue][index]);
			/// check the inputs
			if ( I.length() > 0.0001 )
			{
				/// calculate the reflection vector
				Vector R = I.reflection( N );
				/// normalize it
				if ( R.length() > 0.0001f )
					R.normalize();
				/// fresnel
				if( fresnel )
				{
					float fresnelfake = ( ( 1 - IOR ) / ( 1 + IOR ) ) * ( ( 1 - IOR ) / ( 1 + IOR ) );
					R = R * fresnelfake;
				}
				/// set the output
				outPixelsR[index] = R.x;
				outPixelsG[index] = R.y;
				outPixelsB[index] = R.z;

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
