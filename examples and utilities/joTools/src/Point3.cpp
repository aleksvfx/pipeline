/// \file		Point3.cpp
/// \author		Johannes Saam

#include "Point3.h"


std::ostream& operator<<( std::ostream &output, const Point3 &p )
{
	return output<<"[ "<<p.x<<", "<<p.y<<", "<<p.z<<" ]";
}


Point3::Point3()
{
	x = 0;
	y = 0;
	z = 0;
};

Point3::Point3( float _x, float _y, float _z )
{
	x = _x;
	y = _y;
	z = _z;
};

Point3 Point3::operator+( Point3 p1 )
{
	return Point3( p1.x + x, p1.y + y, p1.z + z );
};

Point3 Point3::operator-( Point3 p1 )
{
	return Point3( x - p1.x, y - p1.y, z - p1.z );
};

Point3 Point3::operator*( Point3 p1 )
{
	return Point3( p1.x * x, p1.y * y, p1.z * z );
};

Point3 Point3::operator/( Point3 p1 )
{
	return Point3( x / p1.x, y / p1.y, z / p1.z );
};

Point3 Point3::operator*( float value )
{
	return Point3( value * x, value * y, value * z );
};

Point3 Point3::operator/( float value )
{
	return Point3( x / value, y / value, z / value );
}

void Point3::setPoint3( float _x, float _y, float _z )
{
	x = _x;
	y = _y;
	z = _z;
};

void Point3::setPoint3X( float value )
{
	x = value;
};

void Point3::setPoint3Y( float value )
{
	y = value;
};

void Point3::setPoint3Z( float value )
{
	z = value;
};

float Point3::dist( Point3 p1 )
{
	Point3 tmp = p1 - *this;
	float len = sqrt( tmp.x *tmp.x + tmp.y * tmp.y + tmp.z * tmp.z );
	return len;
};

