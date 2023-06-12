#include "sphere.h"
#include <GL\freeglut.h>
#include "material.h"
#include "argparser.h"
#include "utils.h"


// ====================================================================
// ====================================================================

bool
Sphere::intersect (const Ray & r, Hit & h) const
{
	Vec3f PC = center - r.getOrigin();
	if( PC.Dot3( r.getDirection() ) <= 0 ) //sphere behind ray
		return false;

	float t = (PC.Dot3( r.getDirection() ) * r.getDirection()).Length();
	Vec3f Q = r.getOrigin() + t*r.getDirection();
	if( (Q - center).Length() > radius ) //miss
		return false;

	float tmp = sqrt( radius * radius - powf((Q - center).Length(), 2) );
	Vec3f normal = (r.getOrigin() + (t-tmp)*r.getDirection() ) - center;
	normal.Normalize();
	h.set( t-tmp, material, normal );
	return true;
}


// ====================================================================
// ====================================================================
