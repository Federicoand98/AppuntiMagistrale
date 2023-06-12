#ifndef _RAY_H
#define _RAY_H

#include <iostream>
#include "vectors.h"

using namespace std;

// Ray class mostly copied from Peter Shirley and Keith Morley

// ====================================================================
// ====================================================================

class Ray
{

public:

  // CONSTRUCTOR & DESTRUCTOR
  Ray (const Vec3f & orig, const Vec3f & dir)
  {
	origin = orig;
	direction = dir;
  }
  Ray (const Ray & r)
  {
	origin = r.origin;
	direction = r.direction;
  }

  // ACCESSORS
  const Vec3f & getOrigin () const
  {
	return origin;
  }
  const Vec3f & getDirection () const
  {
	return direction;
  }
  Vec3f pointAtParameter (float t) const
  {
	return origin + direction * t;
  }

private:
    Ray ()
  {
	assert (0);
  }									   // don't use this constructor

  // REPRESENTATION
  Vec3f origin;
  Vec3f direction;
};

inline ostream &
operator<< (ostream & os, const Ray & r)
{
  os << "Ray <" << r.getOrigin () << ", " << r.getDirection () << ">";
  return os;
}

// ====================================================================
// ====================================================================

#endif
