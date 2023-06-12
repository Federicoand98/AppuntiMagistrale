#ifndef _VERTEX_H
#define _VERTEX_H

#include <assert.h>
#include "vectors.h"

class Vertex;

// ==========================================================

class Vertex
{

public:

  // ========================
  // CONSTRUCTOR & DESTRUCTOR
  Vertex (int i, const Vec3f & pos):position (pos)
  {
	index = i;
  }
  virtual ~ Vertex ()
  {
  }

  // =========
  // ACCESSORS
  int getIndex () const
  {
	return index;
  }
  double x () const
  {
	return position.x ();
  }
  double y () const
  {
	return position.y ();
  }
  double z () const
  {
	return position.z ();
  }
  const Vec3f & get () const
  {
	return position;
  }

  // =========
  // MODIFIERS
  void set (Vec3f v)
  {
	position = v;
  }
  void set (double x, double y, double z)
  {
	position.Set ((float)x, (float)y, (float)z);
  }

private:

  // don't use these constructors
  Vertex ()
  {
	assert (0);
  }
  Vertex & operator= (const Vertex &)
  {
	assert (0);
  }
  Vertex (const Vertex &)
  {
	assert (0);
  }

  // ==============
  // REPRESENTATION
  Vec3f position;

  // this is the index from the original .obj file.
  // technically not part of the half-edge data structure
  int index;

  // NOTE: the vertices don't know anything about adjacency.  In some
  // versions of this data structure they have a pointer to one of
  // their incoming edges.  However, this data is very complicated to
  // maintain during mesh manipulation.

};

// ==========================================================

#endif
