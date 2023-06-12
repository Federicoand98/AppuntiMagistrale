#ifndef _HIT_H
#define _HIT_H

#include "vectors.h"
#include "ray.h"

class Material;
class Face;

// ====================================================================
// ====================================================================

class Hit
{

public:

  // CONSTRUCTOR & DESTRUCTOR
  Hit ()
  {
	t = (float)HUGE;
	material = NULL;
	normal = Vec3f (0.0, 0.0, 0.0);
  }
  Hit (const Hit & h)
  {
	t = h.t;
	material = h.material;
	normal = h.normal;
  }
  ~Hit ()
  {
  }

  // ACCESSORS
  float getT () const
  {
	return t;
  }
  Material *getMaterial () const
  {
	return material;
  }
  Vec3f getNormal () const
  {
	return normal;
  }

  // MODIFIER
  void set (float _t, Material * m, Vec3f n)
  {
	t = _t;
	material = m;
	normal = n;
  }

private:

  // REPRESENTATION
  float t;
  Material *material;
  Vec3f normal;
};

inline ostream &
operator<< (ostream & os, const Hit & h)
{
  os << "Hit <" << h.getT () << ", " << h.getNormal () << ">";
  return os;
}

// ====================================================================
// ====================================================================

#endif
