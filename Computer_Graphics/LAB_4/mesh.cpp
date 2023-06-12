#define _CRT_SECURE_NO_WARNINGS // for fscanf

#include <stdio.h>
#include <assert.h>

#include <GL\freeglut.h>

#include "mesh.h"
#include "edge.h"
#include "vertex.h"
#include "face.h"
#include "glCanvas.h"
#include "vertex_parent.h"
#include "sphere.h"

#define INITIAL_VERTEX 10000
#define INITIAL_EDGE 10000
#define INITIAL_FACE 10000

// =======================================================================
// CONSTRUCTORS & DESTRUCTORS
// =======================================================================

Mesh::Mesh ()
{
  edges = new Bag < Edge * >(INITIAL_EDGE, Edge::extract_func);
  vertex_parents = new Bag < VertexParent * >(INITIAL_VERTEX, VertexParent::extract_func);
  bbox = NULL;
}

Mesh::~Mesh ()
{
  delete edges;
  edges = NULL;
  delete bbox;
  bbox = NULL;
}

// =======================================================================
// MODIFIERS:   ADD & REMOVE
// =======================================================================

Vertex *
Mesh::addVertex (const Vec3f & position)
{
  int index = numVertices ();
  Vertex *v = new Vertex (index, position);
  vertices.push_back (v);
  if (bbox == NULL)
	bbox = new BoundingBox (position, position);
  else
	bbox->Extend (position);
  return v;
}

// helper function to place a grid of points on the sphere
Vec3f
ComputeSpherePoint (float s, float t, const Vec3f center, float radius)
{
  float angle = 2 * M_PI * s;
  float y = -cos (M_PI * t);
  float factor = sqrt (1 - y * y);
  float x = factor * cos (angle);
  float z = factor * -sin (angle);
  Vec3f answer = Vec3f (x, y, z);
  answer *= radius;
  answer += center;
  return answer;
}

void
Mesh::addSphere (const Vec3f & center, float radius, Material * material)
{
  // we store the original sphere for raytracing
  Sphere *sp = new Sphere (center, radius, material);
  spheres.push_back (sp);

  // and convert it into quad patches for radiosity
  int h = args->sphere_horiz;
  int v = args->sphere_vert;
  assert (h % 2 == 0);
  int i, j;
  int va, vb, vc, vd;
  Vertex *a, *b, *c, *d;
  int offset = vertices.size ();

  // place vertices
  addVertex (center + radius * Vec3f (0, -1, 0));	// bottom
  for (j = 1; j < v; j++)
  {									   // middle
	for (i = 0; i < h; i++)
	{
	  float s = i / float (h);
	  float t = j / float (v);
	  addVertex (ComputeSpherePoint (s, t, center, radius));
	}
  }
  addVertex (center + radius * Vec3f (0, 1, 0));	// top

  // the middle patches
  for (j = 1; j < v - 1; j++)
  {
	for (i = 0; i < h; i++)
	{
	  va = 1 + i + h * (j - 1);
	  vb = 1 + (i + 1) % h + h * (j - 1);
	  vc = 1 + i + h * (j);
	  vd = 1 + (i + 1) % h + h * (j);
	  a = vertices[offset + va];
	  b = vertices[offset + vb];
	  c = vertices[offset + vc];
	  d = vertices[offset + vd];
	  addSphereFace (a, b, d, c, material);
	}
  }

  for (i = 0; i < h; i += 2)
  {
	// the bottom patches
	va = 0;
	vb = 1 + i;
	vc = 1 + (i + 1) % h;
	vd = 1 + (i + 2) % h;
	a = vertices[offset + va];
	b = vertices[offset + vb];
	c = vertices[offset + vc];
	d = vertices[offset + vd];
	addSphereFace (d, c, b, a, material);
	// the top patches
	va = 1 + h * (v - 1);
	vb = 1 + i + h * (v - 2);
	vc = 1 + (i + 1) % h + h * (v - 2);
	vd = 1 + (i + 2) % h + h * (v - 2);
	a = vertices[offset + va];
	b = vertices[offset + vb];
	c = vertices[offset + vc];
	d = vertices[offset + vd];
	addSphereFace (b, c, d, a, material);
  }
}


void
Mesh::addFace (Vertex * a, Vertex * b, Vertex * c, Vertex * d, Material * material, int sphere_face)
{

  // create the face
  Face *f = new Face (material);

  // create the edges
  Edge *ea = new Edge (a, f);
  Edge *eb = new Edge (b, f);
  Edge *ec = new Edge (c, f);
  Edge *ed = new Edge (d, f);

  // point the face to one of its edges
  f->setEdge (ea);

  // connect the edges to each other
  ea->setNext (eb);
  eb->setNext (ec);
  ec->setNext (ed);
  ed->setNext (ea);

  // add them to the master list
  edges->Add (ea);
  edges->Add (eb);
  edges->Add (ec);
  edges->Add (ed);

  // connect up with opposite edges (if they exist)
  Edge *ea_op = getEdge ((*ea)[1], (*ea)[0]);
  Edge *eb_op = getEdge ((*eb)[1], (*eb)[0]);
  Edge *ec_op = getEdge ((*ec)[1], (*ec)[0]);
  Edge *ed_op = getEdge ((*ed)[1], (*ed)[0]);

  if (ea_op != NULL)
  {
	ea_op->setOpposite (ea);
  }
  if (eb_op != NULL)
  {
	eb_op->setOpposite (eb);
  }
  if (ec_op != NULL)
  {
	ec_op->setOpposite (ec);
  }
  if (ed_op != NULL)
  {
	ed_op->setOpposite (ed);
  }

  // add the face to the appropriate master list
  if (sphere_face)
	sphere_faces.push_back (f);
  else
	quad_faces.push_back (f);

  // if it's a light, add it to that list too
  if (material->getEmittedColor ().Length () > 0)
  {
	lights.push_back (f);
  }
}


void
Mesh::removeFaceEdges (Face * f)
{
  Edge *ea = f->getEdge ();
  Edge *eb = ea->getNext ();
  Edge *ec = eb->getNext ();
  Edge *ed = ec->getNext ();
  assert (ed->getNext () == ea);

  // remove elements from master lists
  edges->Remove (ea);
  edges->Remove (eb);
  edges->Remove (ec);
  edges->Remove (ed);

  // clean up memory
  delete ea;
  delete eb;
  delete ec;
  delete ed;
}

Edge *
Mesh::getEdge (Vertex * a, Vertex * b) const
{
  assert (edges != NULL);
  Edge *answer = edges->Get (a->getIndex (), b->getIndex ());
  return answer;
}

Vertex *
Mesh::getChildVertex (Vertex * p1, Vertex * p2) const
{
  VertexParent *vp = vertex_parents->GetReorder (p1->getIndex (), p2->getIndex ());
  if (vp == NULL)
	return NULL;
  return vp->get ();
}

void
Mesh::setParentsChild (Vertex * p1, Vertex * p2, Vertex * child)
{
  vertex_parents->Add (new VertexParent (p1, p2, child));
}

//
// ===============================================================================
// the load function parses our (non-standard) extension of very simple .obj files
// ===============================================================================

void
Mesh::Load (const char *input_file, ArgParser * _args)
{
  args = _args;
  FILE *objfile = fopen (input_file, "r");
  if (objfile == NULL)
  {
	printf ("ERROR! CANNOT OPEN '%s'\n", input_file);
	return;
  }

  char line[200];
  char token[100];
  char atoken[100];
  char btoken[100];
  char ctoken[100];
  char dtoken[100];
  char etoken[100];
  float x, y, z;
  int a, b, c, d, e;

  int index = 0;
  int vert_count = 0;
  int vert_index = 1;

  Material *active_material = NULL;
  char* cc;

  while ( (cc=fgets (line, 200, objfile)) )
  {

	if (line[strlen (line) - 2] == '\\')
	{
	  cc=fgets (token, 100, objfile);
	  int tmp = strlen (line) - 2;
	  strncpy (&line[tmp], token, 100);
	}
	int token_count = sscanf (line, "%s\n", token);
	if (token_count == -1)
	  continue;
	a = b = c = d = e = -1;
	if (!strcmp (token, "usemtl") || !strcmp (token, "g"))
	{
	  vert_index = 1;				   //vert_count + 1;
	  index++;
	}
	else if (!strcmp (token, "v"))
	{
	  vert_count++;
	  sscanf (line, "%s %f %f %f\n", token, &x, &y, &z);
	  addVertex (Vec3f (x, y, z));
	}
	else if (!strcmp (token, "f"))
	{
	  int num = sscanf (line, "%s %s %s %s %s %s\n", token,
						atoken, btoken, ctoken, dtoken, etoken);
	  assert (num == 5);
	  sscanf (atoken, "%d", &a);
	  sscanf (btoken, "%d", &b);
	  sscanf (ctoken, "%d", &c);
	  sscanf (dtoken, "%d", &d);
	  a -= vert_index;
	  b -= vert_index;
	  c -= vert_index;
	  d -= vert_index;
	  assert (a >= 0 && a < numVertices ());
	  assert (b >= 0 && b < numVertices ());
	  assert (c >= 0 && c < numVertices ());
	  assert (d >= 0 && d < numVertices ());
	  assert (active_material != NULL);
	  addQuadFace (getVertex (a), getVertex (b), getVertex (c), getVertex (d), active_material);
	}
	else if (!strcmp (token, "s"))
	{
	  float x, y, z, r;
	  int num = sscanf (line, "%s %f %f %f %f", token, &x, &y, &z, &r);
	  assert (num == 5);
	  addSphere (Vec3f (x, y, z), r, active_material);
	}
	else if (!strcmp (token, "m"))
	{
	  // this is not standard .obj format!!
	  // materials
	  int i;
	  int num = sscanf (line, "%s %d", token, &i);
	  assert (num == 2);
	  assert (i >= 0 && i < (int) materials.size ());
	  active_material = materials[i];
	}
	else if (!strcmp (token, "material"))
	{
	  // this is not standard .obj format!!
	  // lights
	  float r, g, b;
	  cc=fgets (line, 200, objfile);
	  int num = sscanf (line, "%s %f %f %f", token, &r, &g, &b);
	  assert (num == 4);
	  assert (!strcmp (token, "diffuse"));
	  Vec3f diffuse (r, g, b);
	  cc=fgets (line, 200, objfile);
	  num = sscanf (line, "%s %f %f %f", token, &r, &g, &b);
	  assert (num == 4);
	  assert (!strcmp (token, "reflective"));
	  Vec3f reflective (r, g, b);
	  cc=fgets (line, 200, objfile);
	  num = sscanf (line, "%s %f %f %f", token, &r, &g, &b);
	  assert (num == 4);
	  assert (!strcmp (token, "emitted"));
	  Vec3f emitted (r, g, b);
	  cc=fgets (line, 200, objfile);
	  float glossiness;
	  num = sscanf (line, "%s %f", token, &glossiness);
	  assert (num == 2);
	  assert (!strcmp (token, "glossiness"));
	  materials.push_back (new Material (diffuse, reflective, emitted, glossiness));
	}
	else if (!strcmp (token, "vt"))
	{
	}
	else if (!strcmp (token, "vn"))
	{
	}
	else if (token[0] == '#')
	{
	}
	else
	{
	  printf ("LINE: '%s'", line);
	}
  }

  cout << " mesh loaded " << numFaces () << endl;

}

// =======================================================================
// PAINT
// =======================================================================

void
Mesh::PaintWireframe ()
{

  glDisable (GL_LIGHTING);

  // draw all the interior edges
  glLineWidth (1);
  glColor3f (0, 0, 0);
  glBegin (GL_LINES);
  Iterator < Edge * >*iter = edges->StartIteration ();
  while (Edge * e = iter->GetNext ())
  {
	if (e->getOpposite () == NULL)
	  continue;
	Vec3f a = (*e)[0]->get ();
	Vec3f b = (*e)[1]->get ();
	glVertex3f (a.x (), a.y (), a.z ());
	glVertex3f (b.x (), b.y (), b.z ());
  }
  edges->EndIteration (iter);
  glEnd ();

  // draw all the boundary edges
  glLineWidth (3);
  glColor3f (1, 0, 0);
  glBegin (GL_LINES);
  iter = edges->StartIteration ();
  while (Edge * e = iter->GetNext ())
  {
	if (e->getOpposite () != NULL)
	  continue;
	Vec3f a = (*e)[0]->get ();
	Vec3f b = (*e)[1]->get ();
	glVertex3f (a.x (), a.y (), a.z ());
	glVertex3f (b.x (), b.y (), b.z ());
  }
  edges->EndIteration (iter);
  glEnd ();

  glEnable (GL_LIGHTING);

  HandleGLError ();
}

// =================================================================
// SUBDIVISION
// =================================================================

Vertex *
Mesh::AddEdgeVertex (Vertex * a, Vertex * b)
{
  Vertex *v = getChildVertex (a, b);
  if (v != NULL)
	return v;
  Vec3f pos = .5 * a->get () + 0.5 * b->get ();
  v = addVertex (pos);
  setParentsChild (a, b, v);
  return v;
}

Vertex *
Mesh::AddMidVertex (Vertex * a, Vertex * b, Vertex * c, Vertex * d)
{
  Vec3f pos = 0.25 * a->get () + 0.25 * b->get () + 0.25 * c->get () + 0.25 * d->get ();
  Vertex *v = addVertex (pos);
  return v;
}

void
Mesh::Subdivision ()
{
  printf ("Subdivide the mesh!\n");
  lights.clear ();

  vector < Face * >tmp = quad_faces;
  quad_faces.clear ();

  for (unsigned int i = 0; i < tmp.size (); i++)
  {
	Face *f = tmp[i];

	Vertex *a = (*f)[0];
	Vertex *b = (*f)[1];
	Vertex *c = (*f)[2];
	Vertex *d = (*f)[3];

	// add new vertices on the edges
	Vertex *ab = AddEdgeVertex (a, b);
	Vertex *bc = AddEdgeVertex (b, c);
	Vertex *cd = AddEdgeVertex (c, d);
	Vertex *da = AddEdgeVertex (d, a);

	// add new point in the middle of the patch
	Vertex *mid = AddMidVertex (a, b, c, d);

	assert (getEdge (a, b) != NULL);
	assert (getEdge (b, c) != NULL);
	assert (getEdge (c, d) != NULL);
	assert (getEdge (d, a) != NULL);

	// copy the color and emission from the old patch to the new
	Material *m = f->getMaterial ();
	removeFaceEdges (f);
	delete f;

	// create the new faces
	addQuadFace (a, ab, mid, da, m);
	addQuadFace (b, bc, mid, ab, m);
	addQuadFace (c, cd, mid, bc, m);
	addQuadFace (d, da, mid, cd, m);

	assert (getEdge (a, ab) != NULL);
	assert (getEdge (ab, b) != NULL);
	assert (getEdge (b, bc) != NULL);
	assert (getEdge (bc, c) != NULL);
	assert (getEdge (c, cd) != NULL);
	assert (getEdge (cd, d) != NULL);
	assert (getEdge (d, da) != NULL);
	assert (getEdge (da, a) != NULL);
  }
}

// =================================================================
