/// \file joFresnel.cpp
/// \author Johannes Saam
/// \brief Implementation of a fresnel combiner nuke

/// \param char to definde the command name
static const char* const CLASS = "joFresnel";

/// \param char to definde the help context
static const char* const HELP =
"Fresenel combiner combines reflection"
"and refraction based on tefractive index"
"and get the right mix between absorbed"
"and reflected light"
"\n\n\n";


#include <DDImage/Iop.h>
#include <DDImage/Row.h>
#include <DDImage/Tile.h>
#include <DDImage/Knobs.h>
#include "Point3.h"
#include "Vector.h"
/// \namespace DD::Image
using namespace DD::Image;

/// \class joFresnel
/// \brief the fresnel combiner Class
class joFresnel : public Iop {

public:
	/// \brief ctor
	joFresnel();
	/// \param float the indes of refraction
	float IOR;
	/// \param bool output only the fresnel falloff
	bool falloff;
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

joFresnel::joFresnel() : Iop() 
{
	/// set the inputs to 5
	inputs( 5 );
	/// set the standarv values for the variables
	IOR = 1.6666f;
	falloff = false;


}
void joFresnel::knobs(Knob_Callback f) 
{
	Float_knob( f, &IOR, "Index of refraction" );
	Bool_knob( f, &falloff, "Output Blend Factor" );
}


static Iop* joFresnel_c() 
{
	return new joFresnel();
}

const Iop::Description joFresnel::d(CLASS, "Filter/joFresnel", joFresnel_c);

void joFresnel::_validate(bool for_real)
{
	copy_info();
	input(1)->validate( for_real );
	input(2)->validate( for_real );
	input(3)->validate( for_real );
	input(4)->validate( for_real );
	if( IOR <= 0 || info_.is_constant() )
	{
		set_out_channels(Mask_None);
		return;
	}
	set_out_channels( Mask_All );
	Format temp = info_.format();
	width = temp.width();
	info_.set( info_.x(), info_.y(), info_.r(), info_.t() );
}

void joFresnel::_request(int x,int y,int r,int t, ChannelMask channels, int count)
{
	input(0)->request(x,y,r,t, channels, count);
	input(1)->request(x,y,r,t, channels, count);
	input(2)->request(x,y,r,t, channels, count);
	input(3)->request(x,y,r,t, channels, count);
	input(4)->request(x,y,r,t, channels, count);
}

void joFresnel::engine(int y, int x, int r, ChannelMask channels, Row& out)
{
	/// set re allocate the out row to prevent from crashing while zooming
	out.range( 0, width );
	/// get and load the inputs
	Row in1(x, r);
	Row in2(x, r);
	Row in3(x, r);
	Row in4(x, r);
	Row in5(x, r);

	input(0)->get(y, x, r, channels, in1);
	input(1)->get(y, x, r, channels, in2);
	input(2)->get(y, x, r, channels, in3);
	input(3)->get(y, x, r, channels, in4);
	input(4)->get(y, x, r, channels, in5);
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
			Vector refl( in1[Chan_Red][index], in1[Chan_Green][index],in1[Chan_Blue][index] );
			Vector refr( in2[Chan_Red][index], in2[Chan_Green][index], in2[Chan_Blue][index]);
			Vector N( in3[Chan_Red][index], in3[Chan_Green][index], in3[Chan_Blue][index]);
			Vector I( in4[Chan_Red][index], in4[Chan_Green][index], in4[Chan_Blue][index]);
			Vector R( in5[Chan_Red][index], in5[Chan_Green][index], in5[Chan_Blue][index]);
	
			/// check the input
			if( I.length() > 0.0001 )
			{
				/// normalize the EYE vector
				I.normalize();
				/// get the indecies of refraction
				float refIndexT = IOR; //transmitted 
				float refIndexI = 1.0f; //incident 
				
				/// calculate the accurate fresnel index
				float cosT =  R.dot( N ); 
				float cosI =  I.dot( N ); 
				float indexTCosI = refIndexT * cosI; 
				float indexICosT = refIndexI * cosT; 
				float indexTCosT = refIndexT * cosT; 
				float indexICosI = refIndexI * cosI; 
				float rParal = ( indexTCosI - indexICosT ) / ( indexTCosI + indexICosT ); 
				float rPerp = ( indexICosI - indexTCosT ) / ( indexICosI + indexTCosT ); 
				float r = ( rParal * rParal + rPerp * rPerp ) / 2; 
				/// scale the inputs with fresnel index
				refr = refr * (1-r); 

				refl = refl * r; 
				/// add the inputs up
				Vector result = refr + refl;
				/// set the output
				if( falloff )
				{
					outPixelsR[index] = r;
					outPixelsG[index] = r;
					outPixelsB[index] = r;
				}else
				{
					outPixelsR[index] = result.x;
					outPixelsG[index] = result.y;
					outPixelsB[index] = result.z;
				}
			} else
			{
				outPixelsR[index] = 0.0f;
				outPixelsG[index] = 0.0f;
				outPixelsB[index] = 0.0f;
			}
		}
		
	}
}
