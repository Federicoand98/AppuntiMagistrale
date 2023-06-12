#ifndef MESH_H
#define MESH_H

#include <vector>
using namespace std;

#include "vectors.h"
#include "bag.h"
#include "boundingbox.h"

class Vertex;
class Edge;
class Face;
class VertexParent;
class Material;
class Sphere;
class ArgParser;

// ======================================================================
// ======================================================================

class Mesh
{

public:

  // ========================
  // CONSTRUCTOR & DESTRUCTOR
  Mesh ();
  virtual ~ Mesh ();
  void Load (const char *input_file, ArgParser * _args);

  // ========
  // VERTICES
  int numVertices () const
  {
	return vertices.size ();
  }
  Vertex *addVertex (const Vec3f & pos);
  // this creates a relationship between 3 vertices (2 parents, 1 child)
  void setParentsChild (Vertex * p1, Vertex * p2, Vertex * child);
  // this accessor will find a child vertex (if it exists) when given
  // two parent vertices
  Vertex *getChildVertex (Vertex * p1, Vertex * p2) const;
  // look up vertex by index from original .obj file
  Vertex *getVertex (int i) const
  {
	assert (i >= 0 && i < numVertices ());
	Vertex *v = vertices[i];
	  assert (v != NULL);
	  return v;
  }

  // =====
  // EDGES
  int numEdges () const
  {
	return edges->Count ();
  }
  // this efficiently looks for an edge with the given vertices, using a hash table
  Edge *getEdge (Vertex * a, Vertex * b) const;

  // ===============
  // OTHER ACCESSORS
  BoundingBox *getBoundingBox () const
  {
	return bbox;
  }
  int numFaces () const
  {
	return quad_faces.size () + sphere_faces.size ();
  }
  Face *getFace (int i)
  {
	assert (i >= 0 && i < numFaces ());
	if (i < numQuadFaces ())
	  return quad_faces[i];
	else
	  return sphere_faces[i - numQuadFaces ()];
  }
  vector < Face * >&getLights ()
  {
	return lights;
  }
  const vector < Sphere * >&getSpheres () const
  {
	return spheres;
  }
  int numQuadFaces () const
  {
	return quad_faces.size ();
  }

  // ===============
  // OTHER FUNCTIONS
  void PaintWireframe ();
  void Subdivision ();

private:

  void addQuadFace (Vertex * a, Vertex * b, Vertex * c, Vertex * d, Material * material)
  {
	addFace (a, b, c, d, material, 0);
  }
  void addSphereFace (Vertex * a, Vertex * b, Vertex * c, Vertex * d, Material * material)
  {
	addFace (a, b, c, d, material, 1);
  }
  void addFace (Vertex * a, Vertex * b, Vertex * c, Vertex * d, Material * material, int sphere_face);
  void removeFaceEdges (Face * f);
  void addSphere (const Vec3f & center, float radius, Material * material);

  // helper functions
  Vertex *AddEdgeVertex (Vertex * a, Vertex * b);
  Vertex *AddMidVertex (Vertex * a, Vertex * b, Vertex * c, Vertex * d);

  // ==============
  // REPRESENTATION
  vector < Vertex * >vertices;		   // used by both the quads and the sphere quad representations
  vector < Face * >quad_faces;
  vector < Sphere * >spheres;
  vector < Face * >sphere_faces;	   // a quad patch representation of the spheres (is not subdivided)
  vector < Material * >materials;
  vector < Face * >lights;			   // a list of all the patches that are emitters
  Bag < Edge * >*edges;
  Bag < VertexParent * >*vertex_parents;
  BoundingBox *bbox;
  ArgParser *args;

};

// ======================================================================
// ======================================================================

#endif
