/// \file		Vector.h
/// \author		Johannes Saam
/// \brief		defines a Vector

#ifndef __VECTOR_H__
#define __VECTOR_H__

#include "Point3.h"
#include <iostream>

/// \class Vector
/// \brief impelemnts vector in 3d Space
class Vector : public Point3
{
	/// \brief implementation of the << operator
	friend std::ostream& operator<<(std::ostream &output, const Vector &v);
public:
	
	/// \brief default ctor
	Vector(void);
	/// \brief ctor from 3 floats
	Vector( float _x, float _y, float _z );
	/// \brief ctor from a point3
	Vector( Point3 p1 );

	/// \brief normalize the vector
	void normalize();
	/// \brief calculate the length of the Vector
	/// \return float representing the distance
	float length();
	/// \brief set the Vectors components with 3 floats
	void setVector( float _x, float _y, float _z );

	/// \brief Dot product between this and another vector
	/// \return float values
	float dot( Vector v1 );
	/// \brief Cross product between this and another vector
	/// \return Vector perpendiculat to the 2 inputs
	Vector cross( Vector v1 );
	/// \brief angle between this and another vector
	/// \return float values
	float angle( Vector v1 );
	
	/// \brief negate the vector
	void neg();
	/// \brief negate the vector and return a new one
	/// \return a new vector negated
	Vector negV();
	/// \brief reflect the vector and return a new one
	/// \return a new vector reflected
	Vector reflection( Vector n );
	/// \brief refract the vector with a gicen Index pf refraction and return a new one
	/// \return a new vector refracted
	Vector refraction( Vector n, float iorold, float iornew );
};

#endif
