#pragma once
#include <poglib/basic.h>
#include <math.h>

f32         radians(const f32 deg);
f32         Q_rsqrt(const f32 number );


#ifndef IGNORE_MATH_UTIL_IMPLEMENTATION

f32 radians(const f32 deg) 
{
    return deg * PI / 180.0f;
}

f32 Q_rsqrt(const f32 number)
{
	long i;
	f32 x2, y;
	const f32 threehalfs = 1.5F;

	x2 = number * 0.5F;
	y  = number;
	i  = * ( long * ) &y;                       // evil floating point bit level hacking
	i  = 0x5f3759df - ( i >> 1 );               // what the fuck? 
	y  = * ( f32 * ) &i;
	y  = y * ( threehalfs - ( x2 * y * y ) );   // 1st iteration
//	y  = y * ( threehalfs - ( x2 * y * y ) );   // 2nd iteration, this can be removed

	return y;
}

#endif
