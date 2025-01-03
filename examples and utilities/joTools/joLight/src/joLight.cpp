/// \file joLight.cpp
/// \author Johannes Saam
/// \brief Implementation of a point and directional relighting system

/// \param char to definde the command name
static const char* const CLASS = "joLight";

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

/// \class joLight
/// \brief the Spot Class
class joLight : public Iop {

public:
	
	/// \param float phi angele of the directinal light in spherical cooridinates
	float phi;
	/// \param float theta angele of the directinal light in spherical cooridinates
	float theta;
	/// \param bool point directional light switch
	bool isPoint;
	/// \param float x position of the light
	float posX;
	/// \param float y position of the light
	float posY;
	/// \param float z position of the light
	float posZ;
	/// \param float intensity of the light
	float intensity;
	/// \param float r value of the light color
	float colorR;
	/// \param float g value of the light color
	float colorG;
	/// \param float b value of the light color
	float colorB;
	/// \param bool switch for square distance attenuation
	bool attenuateMe;
	/// \param bool switch for specular
	bool doSpec;
	/// \param float phong specular roughness
	float roughness;
	/// \param bool switch to output just the light vector
	bool lightVectors;
	/// \param char .light out file name
	const char* filename;
	/// \param int image width
	int width;
	/// \param int image height
	int height;
	/// \brief ctor
	joLight();
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

joLight::joLight() : Iop() 
{
	/// set the inputs to 2
	inputs( 2 );
	/// set the standarv values for the variables
	phi = 0.0f;
	theta = 0.0f;
	isPoint = false;
	intensity = 1.0f;
	posX = 0.0f;
	posY = 0.0f;
	posZ = 0.0f;
	colorR = 1.0f;
	colorG = 1.0f;
	colorB = 1.0f;
	attenuateMe = true;
	doSpec = false;
	roughness = 150.0f;
	lightVectors = false;
	filename = "";

}
void joLight::knobs(Knob_Callback f) 
{
	Bool_knob( f, &isPoint, "pointLight" );

	Text_knob( f, "Directional Light Attributes\n");
	Float_knob( f, &phi, IRange( 0,2 * M_PI, false ), "phi" );
	Float_knob( f, &theta, IRange( -M_PI,M_PI, false ), "theta" );
	
	Text_knob( f, "Point Light Attributes\n");
	Float_knob( f, &posX, IRange( -10.0f, 10.0f, false ), "posX" );
	Float_knob( f, &posY, IRange( -10.0f, 10.0f, false ), "posY" );
	Float_knob( f, &posZ, IRange( -10.0f, 10.0f, false ), "posZ" );
	
	Text_knob( f, "General Light Attributes\n");
	Float_knob( f, &intensity, IRange( 0,2, false ), "intensity" );
	Bool_knob( f, &attenuateMe, "attenuate" );
	Float_knob( f, &colorR, IRange( 0, 1.0f, false ), "colorR" );
	Float_knob( f, &colorG, IRange( 0, 1.0f, false ), "colorG" );
	Float_knob( f, &colorB, IRange( 0, 1.0f, false ), "colorB" );
	
	Text_knob( f, "Specular Shading Attributes\n");
	Bool_knob( f, &doSpec, "specular" );
	Float_knob( f, &roughness, IRange( 0, 300.0f, false ), "roughness" );

	Bool_knob( f, &lightVectors, "Output just the light Vectors" );
	File_knob( f, &filename, "outFile" );

	Script_knob( f, "exportLight", "Write File" );
}


static Iop* joLight_c() 
{
	return new joLight();
}

const Iop::Description joLight::d(CLASS, "Filter/joLight", joLight_c);

void joLight::_validate(bool for_real)
{
	copy_info();
	input1().validate( for_real );

	if ( intensity <= 0 || info_.is_constant()) 
	{
		set_out_channels(Mask_None);
		return;
	}

	if ( isPoint && posX == 0 && posY == 0 && posZ == 0 ) 
	{
		set_out_channels(Mask_None);
		return;
	}

	Format test = info_.format();
	width = test.width();
	height = test.height();
	set_out_channels(Mask_All);
	info_.set( info_.x(), info_.y(), info_.r()+1, info_.t()+1 );
}

void joLight::_request(int x,int y,int r,int t, ChannelMask channels, int count)
{
	input0().request(x,y,r,t, channels, count);
	input1().request(x,y,r,t, channels, count);
}

void joLight::engine(int y, int x, int r, ChannelMask channels, Row& out)
{
	/// set re allocate the out row to prevent from crashing while zooming
	out.range(0,width);
	/// get and load the inputs
	Row in1(x, r);
	Row in2(x, r);

	input0().get(y, x, r, channels, in1);
	input1().get(y, x, r, channels, in2);
	/// step thru all the aviable channels
	foreach (z, channels) 
	{
		/// generate the output pointers
		float* outPixelsR = out.writable( Chan_Red );
		float* outPixelsG = out.writable( Chan_Green );
		float* outPixelsB = out.writable( Chan_Blue);
		/// steo thru the tile and do the magic
		for (int index = x; index < r; index++ ) 
		{
			/// get the inputs
			Vector N( in1[Chan_Red][index], in1[Chan_Green][index],in1[Chan_Blue][index] );
			Vector I( in2[Chan_Red][index], in2[Chan_Green][index],in2[Chan_Blue][index] );
			Vector L;
			
			/// check the inputs
			if ( I.length() > 0.0001 )
			{
		
				/// if we have directional light
				if ( !isPoint )
				{
					/// calculate the light vector based on sperical cooridinates
					attenuateMe = false;
					float tempX = sin( theta ) * cos( phi );
					float tempY = sin( phi );
					float tempZ = cos( phi ) * cos( theta );
					L.setVector( tempX, tempY, tempZ );
				} else
				{
					/// if point light calculate the light vector
					/// based on the position of the light an position of the shaded point
					L.setVector( posX - I.x, posY - I.y, posZ - I.z );
				}

				float distFac = 1.0f;
				float specAmount = 0;
				/// id attenuate is set
				if ( attenuateMe )
				{
					/// calculate the distance factor for the shaded point based on the square of the distance
					distFac = 1 / ( L.length() * L.length() ) ;
				}
				/// normalize the EYE vector and the light vector
				L.normalize();
				I.normalize();
				/// of the specs are turned on
				if ( doSpec )
				{
					/// caclulate the reflection vector of the light with the normal
					Vector R = L.reflection( N );
					R.normalize();
					/// if the spec is on the face facing the light
					if ( N.dot( L ) > 0 )
					{
						/// calculate the phong specular
						specAmount = pow( I.dot( R ), roughness );
					}
				}
				/// lambertian diffuse
				float diffuse = L.dot( N );
				/// output the light vectors if needed
				if( lightVectors )
				{
						outPixelsR[index] = L.x* distFac;
						outPixelsG[index] = L.y* distFac;
						outPixelsB[index] = L.z* distFac;

				} else
				{
					/// else caclulate the final shading with the lambert diffuse the phong spec an the light colors
					outPixelsR[index] = diffuse * colorR * intensity * distFac + specAmount;
					outPixelsG[index] = diffuse * colorG * intensity * distFac + specAmount;
					outPixelsB[index] = diffuse * colorB * intensity * distFac + specAmount;
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
