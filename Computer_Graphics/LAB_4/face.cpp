#include "face.h"
#include "matrix.h"
#include "utils.h"

// =========================================================================
// utility functions 
inline float
DistanceBetweenTwoPoints (const Vec3f & p1, const Vec3f & p2)
{
  Vec3f v (p1, p2);
  return v.Length ();
}

inline float
AreaOfTriangle (float a, float b, float c)
{
  // Area of Triangle =  (using Heron's Formula)
  //  sqrt[s*(s-a)*(s-b)*(s-c)]
  //    where s = (a+b+c)/2
  // also... Area of Triangle = 0.5 * x * c
  float s = (a + b + c) / (float) 2;
  return sqrt (s * (s - a) * (s - b) * (s - c));
}

float
AreaOfTriangle (const Vec3f & a, const Vec3f & b, const Vec3f & c)
{
  float aside = DistanceBetweenTwoPoints (a, b);
  float bside = DistanceBetweenTwoPoints (b, c);
  float cside = DistanceBetweenTwoPoints (c, a);
  return AreaOfTriangle (aside, bside, cside);
}

// =========================================================================
// =========================================================================

float
Face::getArea () const
{
  Vec3f a = (*this)[0]->get ();
  Vec3f b = (*this)[1]->get ();
  Vec3f c = (*this)[2]->get ();
  Vec3f d = (*this)[3]->get ();
  return AreaOfTriangle (DistanceBetweenTwoPoints (a, b), DistanceBetweenTwoPoints (a, c), DistanceBetweenTwoPoints (b, c)) + AreaOfTriangle (DistanceBetweenTwoPoints (c, d), DistanceBetweenTwoPoints (a, d), DistanceBetweenTwoPoints (a, c));
}

// =========================================================================

Vec3f
Face::RandomPoint () const
{
  Vec3f a = (*this)[0]->get ();
  Vec3f b = (*this)[1]->get ();
  Vec3f c = (*this)[2]->get ();
  Vec3f d = (*this)[3]->get ();

  float s = (float) rand () / RAND_MAX;
  float t = (float) rand () / RAND_MAX;

  Vec3f answer = s * t * a + s * (1 - t) * b + (1 - s) * t * c + (1 - s) * (1 - t) * d;
  return answer;
}

// =========================================================================
// the intersection routines

bool
Face::intersect (const Ray & r, Hit & h, bool intersect_backfacing) const
{
  // intersect with each of the subtriangles
  Vec3f a = (*this)[0]->get ();
  Vec3f b = (*this)[1]->get ();
  Vec3f c = (*this)[2]->get ();
  Vec3f d = (*this)[3]->get ();
  return triangle_intersect (r, h, a, b, c, intersect_backfacing) || triangle_intersect (r, h, a, c, d, intersect_backfacing);
}

bool
Face::triangle_intersect (const Ray & r, Hit & h, const Vec3f & a, const Vec3f & b, const Vec3f & c, bool intersect_backfacing) const
{

  // compute the intersection with the plane of the triangle
  Hit h2 = Hit (h);
  if (!plane_intersect (r, h2, a, b, c, intersect_backfacing))
	return 0;

  // figure out the barycentric coordinates:
  Vec3f Ro = r.getOrigin ();
  Vec3f Rd = r.getDirection ();
  // [ ax-bx   ax-cx  Rdx ][ beta  ]     [ ax-Rox ] 
  // [ ay-by   ay-cy  Rdy ][ gamma ]  =  [ ay-Roy ] 
  // [ az-bz   az-cz  Rdz ][ t     ]     [ az-Roz ] 
  // solve for beta gamma & t using Cramer's rule

  float detA = Matrix::det3x3 (a.x () - b.x (), a.x () - c.x (), Rd.x (),
							   a.y () - b.y (), a.y () - c.y (), Rd.y (),
							   a.z () - b.z (), a.z () - c.z (), Rd.z ());

  if (fabs (detA) <= 0.000001)
	return 0;
  assert (fabs (detA) >= 0.000001);

  float beta = Matrix::det3x3 (a.x () - Ro.x (), a.x () - c.x (), Rd.x (),
							   a.y () - Ro.y (), a.y () - c.y (), Rd.y (),
							   a.z () - Ro.z (), a.z () - c.z (), Rd.z ()) / detA;

  float gamma = Matrix::det3x3 (a.x () - b.x (), a.x () - Ro.x (), Rd.x (),
								a.y () - b.y (), a.y () - Ro.y (), Rd.y (),
								a.z () - b.z (), a.z () - Ro.z (), Rd.z ()) / detA;

  if (beta >= -0.00001 && beta <= 1.00001 && gamma >= -0.00001 && gamma <= 1.00001 && beta + gamma <= 1.00001)
  {
	h = h2;
	assert (h.getT () >= EPSILON);
	return 1;
  }

  return 0;
}


bool
Face::plane_intersect (const Ray & r, Hit & h, const Vec3f & a, const Vec3f & b, const Vec3f & c, bool intersect_backfacing) const
{

  // insert the explicit equation for the ray into the implicit equation of the plane

  // equation for a plane
  // ax + by + cz = d;
  // normal . p + direction = 0
  // plug in ray
  // origin + direction * t = p(t)
  // origin . normal + t * direction . normal = d;
  // t = d - origin.normal / direction.normal;

  Vec3f normal = computeNormal ();
  float d = normal.Dot3 (a);

  float numer = d - r.getOrigin ().Dot3 (normal);
  float denom = r.getDirection ().Dot3 (normal);

  if (denom == 0)
	return 0;						   // parallel to plane

  if (!intersect_backfacing && normal.Dot3 (r.getDirection ()) >= 0)
	return 0;						   // hit the backside

  float t = numer / denom;
  if (t > EPSILON && t < h.getT ())
  {
	h.set (t, this->getMaterial (), normal);
	assert (h.getT () >= EPSILON);
	return 1;
  }
  return 0;
}


inline Vec3f
ComputeNormal (const Vec3f & p1, const Vec3f & p2, const Vec3f & p3)
{
  Vec3f v12 = p2;
  v12 -= p1;
  Vec3f v23 = p3;
  v23 -= p2;
  Vec3f normal;
  Vec3f::Cross3 (normal, v12, v23);
  normal.Normalize ();
  return normal;
}

Vec3f
Face::computeNormal () const
{
  // note: this face might be non-planar, so average the two triangle normals
  Vec3f a = (*this)[0]->get ();
  Vec3f b = (*this)[1]->get ();
  Vec3f c = (*this)[2]->get ();
  Vec3f d = (*this)[3]->get ();
  return 0.5 * (ComputeNormal (a, b, c) + ComputeNormal (a, c, d));
}
