#ifndef _FACE_H
#define _FACE_H

#include "edge.h"
#include "vertex.h"
#include "ray.h"
#include "hit.h"
#include "argparser.h"

// ===========================================================

class Face
{

public:

  // ========================
  // CONSTRUCTOR & DESTRUCTOR
  Face (Material * m)
  {
	edge = NULL;
	material = m;
  }
   ~Face ()
  {
  }

  // here's the hash function to use for faces so they
  // can be efficiently accessed within the Bag data structure
  static void extract_func (Face * t, int &a, int &b, int &c, int &d)
  {
	a = (*t)[0]->getIndex ();
	b = (*t)[1]->getIndex ();
	c = (*t)[2]->getIndex ();
	d = (*t)[3]->getIndex ();
  }

  // =========
  // ACCESSORS
  Vertex *operator[] (int i) const
  {
	assert (edge != NULL);
	if (i == 0)
	  return edge->getVertex ();
	if (i == 1)
	  return edge->getNext ()->getVertex ();
	if (i == 2)
	  return edge->getNext ()->getNext ()->getVertex ();
	if (i == 3)
	  return edge->getNext ()->getNext ()->getNext ()->getVertex ();
	assert (0);
  }
  Edge *getEdge () const
  {
	assert (edge != NULL);
	return edge;
  }
  Vec3f computeCentroid () const
  {
	return 0.25 * ((*this)[0]->get () + (*this)[1]->get () + (*this)[2]->get () + (*this)[3]->get ());
  }
  Material *getMaterial () const
  {
	return material;
  }
  float getArea () const;
  Vec3f RandomPoint () const;
  Vec3f computeNormal () const;

  // =========
  // MODIFIERS
  void setEdge (Edge * e)
  {
	assert (edge == NULL);
	assert (e != NULL);
	edge = e;
  }

  // ==========
  // RAYTRACING
  bool intersect (const Ray & r, Hit & h, bool intersect_backfacing) const;

  // =========
  // RADIOSITY
  int getRadiosityPatchIndex () const
  {
	return radiosity_patch_index;
  }
  void setRadiosityPatchIndex (int i)
  {
	radiosity_patch_index = i;
  }

protected:

  // helper functions
  bool triangle_intersect (const Ray & r, Hit & h, const Vec3f & a, const Vec3f & b, const Vec3f & c, bool intersect_backfacing) const;
  bool plane_intersect (const Ray & r, Hit & h, const Vec3f & a, const Vec3f & b, const Vec3f & c, bool intersect_backfacing) const;

  // don't use this constructor
  Face & operator= (const Face & f)
  {
	assert (0);
  }

  // ==============
  // REPRESENTATION
  Edge *edge;
  // NOTE: If you want to modify a face, remove it from the mesh,
  // delete it, create a new copy with the changes, and re-add it.
  // This will ensure the edges get updated appropriately.

  int radiosity_patch_index;							   // an awkward pointer to this patch in the Radiosity patch array
  Material *material;

};

// ===========================================================

#endif
