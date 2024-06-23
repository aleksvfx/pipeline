/// \file joSpotLight.cpp
/// \author Johannes Saam
/// \brief Implementation of a spot relighting system


#define DEG_TO_RAD 0.0174532925f 
/// \param char to definde the command name
static const char* const CLASS = "joSpotLight";

/// \param char to definde the help context
static const char* const HELP =
"Direction light implementation"
"Rocks soon"
"\n\n\n";


#include <DDImage/Iop.h>
#include <DDImage/Row.h>
#include <DDImage/Tile.h>
#include <DDImage/Knobs.h>
#include "Point3.h"
#include "Vector.h"
/// \namespace DD::Image
using namespace DD::Image;
/// \class joSpotLight
/// \brief the Spotlight Class
class joSpotLight : public Iop {

public:
	/// \param float spot light position X
	float SPosX;
	/// \param float spot light position Y
	float SPosY;
	/// \param float spot light position Z
	float SPosZ;
	/// \param float spot light target position X
	float TPosX;
	/// \param float spot light target position Y
	float TPosY;
	/// \param float spot light target position Z
	float TPosZ;
	/// \param float cone angle
	float coneAngle;
	/// \param float cone delta angle
	float coneDeltaAngle;
	/// \param float beam distribution
	float beam;
	/// \param bool attenuation switch
	bool attenuate;
	/// \param float scene scale divisor
	float distDiv;
	/// \param float light intensity
	float intensity;
	/// \param float light color R
	float colorR;
	/// \param float light color G
	float colorG;
	/// \param float light color b
	float colorB;
	/// \param bool specular switch
	bool doSpec;
	/// \param float phong specular roughness
	float roughness;
	/// \param bool output just the light vector
	bool ligthVectors;
	/// \param char .light output file
	const char* filename;
	/// \param int image width
	int width;
	/// \param int image height
	int height;
	/// \brief ctor
	joSpotLight();
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

joSpotLight::joSpotLight() : Iop() 
{
	/// set the inputs to 2
	inputs( 2 );
	/// set the standarv values for the variables
	SPosX = 0.0f;
	SPosY = 0.0f;
	SPosZ = 0.0f;
	TPosX = 0.0f;
	TPosY = 0.0f;
	TPosZ = 0.0f;
	coneAngle = 30.0f;
	coneDeltaAngle = 0.0f;
	beam = 2.0;
	attenuate = false;
	distDiv = 1.0f;
	intensity = 1.0f;
	colorR = 1.0f;
	colorG = 1.0f;
	colorB = 1.0f;
	doSpec = false;
	roughness = 150;
	ligthVectors = false;
	filename = "";
}
void joSpotLight::knobs(Knob_Callback f) 
{
	Text_knob( f, "Spot Position");
	Float_knob( f, &SPosX, IRange( -10.0f, 10.0f, false ), "posX" );
	Float_knob( f, &SPosY, IRange( -10.0f, 10.0f, false ), "posY" );
	Float_knob( f, &SPosZ, IRange( -10.0f, 10.0f, false ), "posZ" );
	Text_knob( f, "Spot Target Position");
	Float_knob( f, &TPosX, IRange( -10.0f, 10.0f, false ), "TPosX" );
	Float_knob( f, &TPosY, IRange( -10.0f, 10.0f, false ), "TPosY" );
	Float_knob( f, &TPosZ, IRange( -10.0f, 10.0f, false ), "TPosZ" );

	Text_knob( f, "Spot Angles");
	Float_knob( f, &coneAngle, IRange( 0, 90, false ), "coneAngle" );
	Float_knob( f, &coneDeltaAngle, IRange( 0, 80, false ), "coneDeltaAngle" );
	Float_knob( f, &beam, IRange( 0, 10, false ), "beamDistribution" );

	Text_knob( f, "Distance Attenuation");
	Bool_knob( f, &attenuate, "attenuate" );
	Float_knob( f, &distDiv, IRange( 0.001f, 10000, false ), "distanceScale" );

	Text_knob( f, "Color");
	Float_knob( f, &intensity, IRange( 0,2, false ), "intensity" );
	Float_knob( f, &colorR, IRange( 0, 1.0f, false ), "colorR" );
	Float_knob( f, &colorG, IRange( 0, 1.0f, false ), "colorG" );
	Float_knob( f, &colorB, IRange( 0, 1.0f, false ), "colorB" );

	Text_knob( f, "Spec Options");
	Bool_knob( f, &doSpec, "specular");
	Float_knob( f, &roughness, "roughness");

	Bool_knob( f, &ligthVectors, "Output Light Vectors only" );
	File_knob( f, &filename, "outFile" );

	Script_knob( f, "exportSpotLight", "Write File" );
}


static Iop* joSpotLight_c() 
{
	return new joSpotLight();
}

const Iop::Description joSpotLight::d(CLASS, "Filter/joSpotLight", joSpotLight_c);

void joSpotLight::_validate(bool for_real)
{
	copy_info();
	input1().validate( for_real );
	if ( intensity <= 0 || info_.is_constant()) 
	{
		set_out_channels(Mask_None);
		return;
	}

	if ( SPosX == 0 && SPosY == 0 && SPosZ == 0 ) 
	{
		set_out_channels(Mask_None);
		return;
	}

	if ( coneDeltaAngle > coneAngle ) 
	{
		set_out_channels(Mask_None);
		return;
	}


	Format test = info_.format();
	width = test.width();
	height = test.height();
	set_out_channels( Mask_All );
	info_.set( info_.x(), info_.y(), info_.r()+ 1, info_.t() + 1 );
}

void joSpotLight::_request(int x,int y,int r,int t, ChannelMask channels, int count)
{
	input0().request(x,y,r,t, channels, count);
	input1().request(x,y,r,t, channels, count);
}
/// \fn smoothstep
/// \brief implementation of rendermans smoothstep
float smoothstep( float min,  float max, float x )
{
	
	if( x < min )
	{
		return 0.0f;
	} else if( x >= max )
	{
		return 1.0f;
	} else
	{
		return ( x - min ) / ( max - min );
	}

}

void joSpotLight::engine(int y, int x, int r, ChannelMask channels, Row& out)
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
			Vector N( in1[Chan_Red][index], in1[Chan_Green][index], in1[Chan_Blue][index] );
			Vector I( in2[Chan_Red][index], in2[Chan_Green][index], in2[Chan_Blue][index] );

			/// convert the user variables to a light vector
			Vector A = Vector( TPosX, TPosY, TPosZ ) - Vector( SPosX, SPosY, SPosZ );
			Vector L = I - Vector( SPosX, SPosY, SPosZ );

			/// check the input
			if ( I.length() > 0.0001f )
			{
				/// normalize the light vector
				A.normalize();
				/// precalc the distance of the point to the light
				float dist = L.length() / distDiv;
				/// square it
				dist = dist * dist;
				/// normalize the light vec
				L.normalize();
				
				Vector result;
				/// precompute the cosines of inner and outer angle and the current light angle
				float cosoutside = cos( coneAngle * DEG_TO_RAD );
				float cosinside = cos( coneAngle * DEG_TO_RAD - coneDeltaAngle * DEG_TO_RAD );
				float coscurrent = L.dot( A );

				/// negate the L vetor for shading
				L.neg();
				/// if the point is inside the light cone
				if ( coscurrent > cosoutside )
				{
					
					float specAmount = 0.0f;
					/// calculate the attenuation to the outside based on RENDERMANS(C) spot light
					float atten = (float)pow( (double)coscurrent, (double)beam );
					atten *= smoothstep( cosoutside, cosinside, coscurrent ); 
					/// lambertian diffuse
					float diffuse = L.dot( N ); 
					/// if attenuate
					if ( attenuate )
					{
						/// multuply the attenuation with the square of the distance
						atten *= 1 / dist;
					}

					/// if specs are on
					if ( doSpec )
					{
						/// calc the reflect vector
						Vector R = L.reflection( N );
						/// normalize it
						R.normalize();
						/// if the spec is in the right angle
						if ( N.dot( L ) > 0 )
						{
							/// calc phong specular
							specAmount = pow( I.dot( R ), roughness );
						}
					}
					/// output just the light vectors if needed
					if( ligthVectors )
					{
						result.x = L.x * atten;
						result.y = L.y * atten;
						result.z = L.z * atten;

					} else
					/// else make the final shading addup and return it
					{
						result.x = colorR * diffuse * intensity * atten + specAmount;
						result.y = colorG * diffuse * intensity * atten + specAmount;
						result.z = colorB * diffuse * intensity * atten + specAmount;
					}
					
				} else
				{
					result.setVector( 0, 0, 0 );
				}
				
				outPixelsR[index] = result.x;
				outPixelsG[index] = result.y;
				outPixelsB[index] = result.z;
			} else
			{
				outPixelsR[index] = 0.0f;
				outPixelsG[index] = 0.0f;
				outPixelsB[index] = 0.0f;
			}

			
		}
		
	}
}
