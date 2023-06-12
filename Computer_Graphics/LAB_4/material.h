#ifndef _MATERIAL_H_
#define _MATERIAL_H_

#include <assert.h>

#include "vectors.h"
#include "ray.h"
#include "hit.h"

class ArgParser;

// ====================================================================
// ====================================================================
// A simple Phong-like material with glossiness

class Material
{

public:

  Material (const Vec3f & d_color, const Vec3f & r_color, const Vec3f & e_color, float gls)
  {
	diffuseColor = d_color;
	reflectiveColor = r_color;
	emittedColor = e_color;
	glossiness = gls;
  }

  // ACCESSORS
  const Vec3f & getDiffuseColor () const
  {
	return diffuseColor;
  }
  const Vec3f & getReflectiveColor () const
  {
	return reflectiveColor;
  }
  const Vec3f & getEmittedColor () const
  {
	return emittedColor;
  }
  float getGlossiness () const
  {
	return glossiness;
  }

  // SHADE
  // compute the contribution to local illumination at this point for
  // a particular light source
  Vec3f Shade (const Ray & ray, const Hit & hit, const Vec3f & dirToLight, const Vec3f & lightColor, ArgParser * args) const;

protected:

  // REPRESENTATION
  Vec3f diffuseColor;
  Vec3f reflectiveColor;
  Vec3f emittedColor;
  float glossiness;
};

// ====================================================================
// ====================================================================

#endif
