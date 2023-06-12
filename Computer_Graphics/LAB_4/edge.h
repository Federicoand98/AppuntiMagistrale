#ifndef EDGE_H
#define EDGE_H

#include <limits.h>
#include <stdio.h>
#include <assert.h>
//#include <stdlib.h>

class Vertex;
class Face;

// ===================================================================
// half-edge data structure

class Edge
{

public:

  // ========================
  // CONSTRUCTORS & DESTRUCTOR
  Edge (Vertex * v, Face * f);
  ~Edge ();

  // here's the hash function to use for edges so they
  // can be efficiently accessed within the Bag data structure
  static void extract_func (Edge * e, int &a, int &b, int &c, int &d);

  // =========
  // ACCESSORS
  Vertex *getVertex () const
  {
	assert (vertex != NULL);
	return vertex;
  }
  Edge *getNext () const
  {
	assert (next != NULL);
	return next;
  }
  Face *getFace () const
  {
	assert (face != NULL);
	return face;
  }
  Edge *getOpposite () const
  {
	// warning!  the opposite edge might be NULL!
	return opposite;
  }
  Vertex *operator[] (int i) const
  {
	if (i == 0)
	  return getVertex ();
	if (i == 1)
	  return getNext ()->getVertex ();
	assert (0);
  }

  // =========
  // MODIFIERS
  void setOpposite (Edge * e)
  {
	assert (opposite == NULL);
	assert (e != NULL);
	assert (e->opposite == NULL);
	opposite = e;
	e->opposite = this;
  }
  void clearOpposite ()
  {
	if (opposite == NULL)
	  return;
	assert (opposite->opposite == this);
	opposite->opposite = NULL;
	opposite = NULL;
  }
  void setNext (Edge * e)
  {
	assert (next == NULL);
	assert (e != NULL);
	assert (face == e->face);
	next = e;
  }

  void Print ();

private:

  Edge (const Edge &)
  {
	assert (0);
  }
  Edge & operator= (const Edge &)
  {
	assert (0);
  }

  // ==============
  // REPRESENTATION
  // in the half edge data adjacency data structure, the edge stores everything!
  Vertex *vertex;
  Face *face;
  Edge *opposite;
  Edge *next;
};

// ===================================================================

#endif
