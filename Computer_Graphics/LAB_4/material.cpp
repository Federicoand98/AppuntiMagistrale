#include "material.h"

// PHONG LOCAL ILLUMINATION

// this function should be called to compute the light contributed by
// a particular light source to the intersection point.  Note that
// this function does not calculate any global effects (e.g., shadows). 

Vec3f
Material::Shade (const Ray & ray, const Hit & hit, const Vec3f & dirToLight, const Vec3f & lightColor, ArgParser * args) const
{

  Vec3f point = ray.pointAtParameter (hit.getT ());
  Vec3f n = hit.getNormal ();
  Vec3f e = ray.getDirection () * -1.0f;
  Vec3f l = dirToLight;

  Vec3f answer = Vec3f (0, 0, 0);

  // emitted component
  // -----------------
  answer += 0.2f * getEmittedColor ();

  // diffuse component
  // -----------------
  float dot_nl = n.Dot3 (l);
  if (dot_nl < 0.0f)
	dot_nl = 0.0f;
  answer += lightColor * getDiffuseColor () * dot_nl;

  // specular component (Phong)
  // ------------------
  // make up reasonable values for other Phong parameters
  Vec3f specularColor = 0.8f * reflectiveColor;
  float exponent = 5.0f / (glossiness + 0.01f);
  // compute ideal reflection angle
  Vec3f r = (l * -1.0f) + n * (2.0f * dot_nl);
  r.Normalize ();
  float dot_er = e.Dot3 (r);
  if (dot_er < 0.0f)
	dot_er = 0.0f;
  answer += lightColor * specularColor * pow (dot_er, exponent) * dot_nl;

  return answer;
}
