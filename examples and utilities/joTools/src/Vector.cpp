/// \file		Vector.cpp
/// \author		Johannes Saam

#include "Vector.h"
#include <iostream>
/// \namespace standard namespace
using namespace std;

std::ostream& operator<<( std::ostream &output, const Vector &v )
{
	return output<<"[ "<<v.x<<", "<<v.y<<", "<<v.z<<" ]";
}

Vector::Vector(void)
{
	x = y = z = 0;
};

Vector::Vector( float _x, float _y, float _z )
{
	x = _x;
	y = _y;
	z = _z;
};

Vector::Vector( Point3 p1 )
{
	x = p1.x;
	y = p1.y;
	z = p1.z;
};

float Vector::length()
{
	float len = sqrt( x *x + y * y + z * z );
	return len;
};

void Vector::normalize()
{
	float len = sqrt( x *x + y * y + z * z );
	x /= len;
	y /= len;
	z /= len;
}

void Vector::setVector( float _x, float _y, float _z )
{
	x = _x;
	y = _y;
	z = _z;
}

float Vector::dot( Vector v1 )
{
	return  float(x * v1.x + y * v1.y + z * v1.z);
}

Vector Vector::cross( Vector v1 )
{	
	return Vector( y * v1.z - z  * v1.y, z * v1.x - x  * v1.z, x * v1.y - y  * v1.x );
}

float Vector::angle( Vector v1 )
{
	Vector v2 = *this;
	return acos( v1.dot( v2 ) );
}


Vector Vector::reflection( Vector n )
{
	n.normalize();
	Vector d = *this;

	d.normalize();
	float scaler = d.dot( n ) * 2.0f;
	Vector r = n * scaler;

	return Vector( d - r );
}

Vector Vector::refraction( Vector n, float iorold, float iornew )
{
	n.normalize();
	
	const float PI = 3.14159265358979323846f;

	float jn = 1.0f;
	float refIndexT = iornew;
	float refIndexI = iorold;

	float r1 = ((float)(rand() % 1000)) / 1000.0f;
	float r2 = ((float)(rand() % 1000)) / 1000.0f;

	float a = sqrtf(1 - powf(r1, 2.0f / (jn + 1.0f)));
	float phi = 2 * PI * r2;

	float x = a * cos(phi);
	float y = a * sin(phi);
	float z = powf(r1, 1.0f / (jn + 1.0f));

	Vector inVec = *this;
	float vecDotN = inVec.dot( n );
	float bb = (vecDotN >= 0)? refIndexT / refIndexI : refIndexI / refIndexT;
	float aa = 0;
	float D2 = 1 - bb * bb *(1 - vecDotN * vecDotN);

	if(D2 < 0) {
		return Vector(0,0,0);
	}

	if(vecDotN > 0) {
		aa = bb * vecDotN - sqrt(D2);
	} else {
		aa = bb * vecDotN + sqrt(D2);
	}

	Vector refractedVec = inVec * bb - n * aa;
	return refractedVec;
};

void Vector::neg()
{
	x = -x;
	y = -y;
	z = -z;
}

Vector Vector::negV()
{
	return Vector( -x, -y, -z );
}

