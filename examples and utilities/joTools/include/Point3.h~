 /*! \mainpage JoLight NUKE(c) Z Depth relighting and Vector math Tools

	<center><img src="../joLight.png" alt="JoLight"/></center><br/>
	
	\subpage Intro - Introduction to the Tool.

	\subpage Install - How to install the tool.
	
	\subpage Nodes - The implemented nodes.

	\subpage Future - work to come on the Tool.

	\subpage Thanks - People that helped me.
*/

/*! \page Intro
	Relighting is a very powerful tool for comopositors. It can help to maximaize efficency on film productions. Turnaround times and time consuming rerenderings can be minimized.<br>
	But advanced relighting tools needs many different information coded in layers. This tool can gather all those information based on just a z Depth layer. This can be used to quickly add to or change lighting in a scene to save compositors the day.<br>
*/

/*! \page Install
	Compile the plugins by running the make.sh and make sure you have  directory in ~/NUKE. Set that dir as your NUKE_PATH and copy the init.tcl and menu.tcl. If you already have a nuke environment set up copy the plugins to your plugin path and ad the content of the menu.tcl to your already existing. Also copy all the tcl scripts in the TCL folder to your NUKE_PATH. Ask your system administrator for help or mail me at johannes_saam at yahoo dot de.<br>
*/

/*! \page Nodes
	<b>NODES</b><br><br>
	Note that all vectors are coded in XYZ equals to RGB<br>
	<i>z2Pos</i><br>
	Converts a input depth channel coded in RGB to a position map in Camera space. Make sure you adjust the FOV<br>
	<i>pos2Normal</i><br>
	Converts a position Map into surface normals. You can flip all the individual components.<br>
	<i>joLight</i><br>
	This node is the core relighting node. It takes a normal and a position map as inputs. The point light check box switches between a directional light wich can be controled by phi and theta angels or a point light. The point light can be moved around the scene with the position sliders. Point lights can be attenuated, too. A square distance attenuation is choosen here. The lights intensity and color can be changed and specular highlights can be set if choosen to do so. The roughness value controles the roughness of the phong specular. You can output just the light vectors to shade the model as you whish. For example add a dot product node and dot the light vector with the normal to get a lambertian surface.<br> You can export a .light file to access the light position data from a 3d package. Use the mel script coming with the tool to reimport the light data back to MAYA. Make sure that you test the values of your renderer. Mental ray was tested and doubles up the values. <br>
	<i>joSpotLight</i><br>
	Also a relightin node. The color and shading controles are the same as with the joLight node. To control the light position the light and lights target. Control the spotlights cone angel and cone delta angle for falloff. The beam distribution controles the falloff renderman stylee. Outputing light vectors only is aviable on that node, too. You can also export a .light file.<br>
	<i>joReflect</i><br>
	Calculates the reflection vector. Inputs are the normal map and a position map.<br>
	<i>joRefract</i><br>
	Calculates the refraction vector. Inputs are the normal map and a position map. Use the Index of refraction to controle the material property.<br>
	<i>joDot</i><br>
	Calculates the Dot product between two input vectors.<br>
	<i>joCross</i><br>
	Calculates the Cross product between two input vectors.<br>
	<i>joNormalize</i><br>
	Normalizes the input vector.<br>
	<i>vec2st</i><br>
	Converts any vector to st ( also known as uv) coordinates for a texture lookup.<br>
	<i>joSTMapper</i><br>
	Same as the STMap already implemented in nuke. But much easier. It takes input st coordinates and does a texture lookop in the other input.<br>
	<i>joGradVec</i><br>
	Beta version of a node that calculates vectors perpendicualr to an input channel. With that images can ve warped and displaced based on an alpaha image. Currently it takes the red channel and calculates the vectors.<br>
	<i>joFresnel</i><br>
	Composites refraction and reflection together based on Fresnel`s law. Use the IOR to control it. Inputs are the reflection image and the refraction image. To compute the fresnel factor other inputs are needed. input 3 is the normal image input 4 the position map and input 5 the reflection vector.<br>
*/

/*! \page Future
	The main future work would be shadowing. Creating shadows and proper raytracing is in developenment but still has some issues. The idea is to convert the surface points to a point cloud in a special data structure. More depth maps from different angles tranformed to the one camera matrix could add to the realism. Once that point cloud is constructed many differnt features would bepossible. Shadowing raytracing inter reflections ambient occlusion color bleeding even sub surface scattering would be derivable from that data structure.<br>
	Please note that adding in other rendered passes can improve the lighting. Using renderd bent normal passes insdead of the derived normal map ads fake shadows. Ambient Occlusion helps for sure and can be derived from the bend normals and the proper normals.<br>
*/

/*! \page Thanks
	The author wants to thank Joao Montenegro, Davind Minor and Gerard Keating form the MCS computer animation 2007 and Jon Macey and James Witworth from the NCCA. Special thanks to Michael Garrett for testing and ideas.<br>
*/


/// \file		Point3.h
/// \author		Johannes Saam
/// \brief		defines a 3D point

#ifndef __POINT3_H__
#define __POINT3_H__

#include <math.h>
#include <iostream>

/// \class Point3
/// \brief impelemnts point in 3d Space
class Point3
{
	friend std::ostream& operator<<(std::ostream &output, const Point3 &p);
public:
	/// \param x X Value of the Point
	float x;
	/// \param x X Value of the Point
	float y;
	/// \param x X Value of the Point
	float z;

	/// \brief default Constructor
	Point3();
	/// \brief Constructor
	Point3( float _x, float _y, float _z );

	/// \brief + operator implementation with another Point3
	/// \return Point3
	Point3 operator+( Point3 p1 );
	/// \brief - operator implementation with another Point3
	/// \return Point3
	Point3 operator-( Point3 p1 );
	/// \brief * operator implementation with another Point3
	/// \return Point3
	Point3 operator*( Point3 p1 );
	/// \brief / operator implementation with another Point3
	/// \return Point3
	Point3 operator/( Point3 p1 );

	/// \brief * operator implementation with a float
	/// \return Point3
	Point3 operator*( float value );
	/// \brief / operator implementation with a float
	/// \return Point3
	Point3 operator/( float value );

	/// \brief set the Point Based on three floats
	void setPoint3( float _x, float _y, float _z );
	/// \brief set the Points x
	void setPoint3X( float value );
	/// \brief set the Points y
	void setPoint3Y( float value );
	/// \brief set the Points z
	void setPoint3Z( float value );
	/// \brief calculaTes the distance between this and another point
	/// \return 
	float dist( Point3 p1 );

};

#endif

