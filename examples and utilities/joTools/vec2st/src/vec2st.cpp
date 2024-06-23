/// \file vec2st.cpp
/// \author Johannes Saam
/// \brief Implementation of a vector to st codrinates node

/// \param char to definde the command name
static const char* const CLASS = "vec2st";

/// \param char to definde the help context
static const char* const HELP =
"converts any vector coded in RGB as XYZ"
"to st coordinates for a lookup in an"
"at map node"
"\n\n\n";


#include <DDImage/Iop.h>
#include <DDImage/Row.h>
#include <DDImage/Tile.h>
#include <DDImage/Knobs.h>
#include "Point3.h"
#include "Vector.h"
/// \namespace DD::Image
using namespace DD::Image;

/// \class vec2st
/// \brief the vec to st coordinates Class
class vec2st : public Iop {

public:

	/// \param bool flip s with t
	bool flip;
	/// \param bool inverst s
	bool flipS;
	/// \param bool inverse t
	bool flipT;

	/// \param int image width
	int width;
	/// \brief ctor
	vec2st();
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

vec2st::vec2st() : Iop() 
{
	/// set the inputs to 1
	inputs( 1 );
	/// set the standarv values for the variables
	flip = false;
	flipS = false;
	flipT = false;
	width = 0;

}
void vec2st::knobs(Knob_Callback f) 
{
	Bool_knob( f, &flip, "Flip s with t" );
	Bool_knob( f, &flipS, "Flip s" );
	Bool_knob( f, &flipT, "Flip t" );
}


static Iop* vec2st_c() 
{
	return new vec2st();
}

const Iop::Description vec2st::d(CLASS, "Filter/vec2st", vec2st_c);

void vec2st::_validate(bool for_real)
{
	copy_info();

	set_out_channels( Mask_All );
	Format temp = info_.format();
	width = temp.width();
	info_.set( info_.x(), info_.y(), info_.r(), info_.t() );
}

void vec2st::_request(int x,int y,int r,int t, ChannelMask channels, int count)
{
	input0().request(x,y,r,t, channels, count);
}

void vec2st::engine(int y, int x, int r, ChannelMask channels, Row& out)
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
			/// get the input
			Vector inVec( in1[Chan_Red][index], in1[Chan_Green][index],in1[Chan_Blue][index] );
			/// check the input
			if ( inVec.length() > 0.0001f )
			{
				float s;
				float t;
				/// flip if desiered
				if ( flip )
				{
					t = (float)acos( inVec.y ) / (float)M_PI;
					s = ( (float)M_PI + (float)atan2( (double)inVec.z, (double)inVec.x ) ) / ( 2 * (float)M_PI ); 
				/// calculate the s and t coordinates based on the spherical coodrinates
				} else
				{
					s = (float)acos( inVec.y ) / (float)M_PI;
					t = ( (float)M_PI + (float)atan2( (double)inVec.z, (double)inVec.x ) ) / ( 2 * (float)M_PI ); 
				}
				/// invers if desiered
				if ( flipS )
				{
					s = s*-1 + 1;
				}

				if ( flipT )
				{
					t = t*-1 + 1;
				}
				/// set the output
				outPixelsR[index] = s;
				outPixelsG[index] = t;
				outPixelsB[index] = 0.0f;
			} else
			{
				outPixelsR[index] = 0.0f;
				outPixelsG[index] = 0.0f;
				outPixelsB[index] = 0.0f;
			}
			
			
		}
		
	}
}
