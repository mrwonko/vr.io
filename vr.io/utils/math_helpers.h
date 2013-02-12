#ifndef _MATH_H
#define _MATH_H

#include <math.h>

#define PI 3.141592654f
#define RADIANS_TO_DEGREES(rad) ((float) rad * (float) (180.0 / PI))
#define SQRT(num) ...

#define PITCH 0
#define YAW 1
#define ROLL 2

// Angles, Vectors, etc

class QAngle
{

public:
	float x, y, z;
	QAngle(void); 
	void Init(float ix=0.0f, float iy=0.0f, float iz=0.0f) { x = ix; y=iy; z=iz; }
	float operator[](int i) const;
	float& operator[](int i);
};


inline QAngle::QAngle()
{
}

inline float& QAngle::operator[](int i)
{
	return ((float*)this)[i];
}

inline float QAngle::operator[](int i) const
{
	return ((float*)this)[i];
}

class Quaternion		
{						

public:
	inline Quaternion(void);
	inline void Init(float ix=0.0f, float iy=0.0f, float iz=0.0f, float iw=0.0f)	{ x = ix; y = iy; z = iz; w = iw; }
	float x, y, z, w;

	float operator[](int i) const;
	float& operator[](int i);
};


inline Quaternion::Quaternion()
{
	this->Init();
}

inline float& Quaternion::operator[](int i)
{
	return ((float*)this)[i];
}

inline float Quaternion::operator[](int i) const
{
	return ((float*)this)[i];
}


#endif