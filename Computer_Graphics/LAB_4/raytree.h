#ifndef _RAY_TREE_H
#define _RAY_TREE_H

#include "ray.h"
#ifdef WIN32
//#include "glut.h"
#include <GL\freeglut.h>
#else
#include <GL/glut.h>
#endif

#include <vector>

using namespace std;

// ====================================================================
// ====================================================================
// data structure to store a segment

class Segment
{

public:

  // CONSTRUCTOR & DESTRUCTOR
  Segment ()
  {
	Clear ();
  }
  Segment (const Ray & ray, float tstart, float tstop)
  {
	// first clamp the segment to "reasonable" values 
	// to make sure it is drawn correctly in OpenGL
	if (tstart < -100)
	  tstart = -100;
	if (tstop > 100)
	  tstop = 100;
	a = ray.pointAtParameter (tstart);
	b = ray.pointAtParameter (tstop);
  }
  Segment (const Segment & s)
  {
	a = s.a;
	b = s.b;
  }
  ~Segment ()
  {
  }

  void Clear ()
  {
	a = Vec3f (0, 0, 0);
	b = Vec3f (0, 0, 0);
  }
  void paint ()
  {
	glVertex3f (a.x (), a.y (), a.z ());
	glVertex3f (b.x (), b.y (), b.z ());
  }

private:
  // REPRESENTATION
  Vec3f a;
  Vec3f b;
};

// ====================================================================
// ====================================================================
//
// This class only contains static variables and static member
// functions so there is no need to call the constructor, destructor
// etc.  It's just a wrapper for the ray tree visualization data.
//

class RayTree
{

public:

  // most of the time the RayTree is NOT activated, so the segments are not updated
  static void Activate ()
  {
	Clear ();
	activated = 1;
  }
  static void Deactivate ()
  {
	activated = 0;
  }

  // when activated, these function calls store the segments of the tree
  static void SetMainSegment (const Ray & ray, float tstart, float tstop)
  {
	if (!activated)
	  return;
	main_segment = Segment (ray, tstart, tstop);
  }
  static void AddShadowSegment (const Ray & ray, float tstart, float tstop)
  {
	if (!activated)
	  return;
	shadow_segments.push_back (Segment (ray, tstart, tstop));
  }
  static void AddReflectedSegment (const Ray & ray, float tstart, float tstop)
  {
	if (!activated)
	  return;
	reflected_segments.push_back (Segment (ray, tstart, tstop));
  }

  static void paint ();

private:

  // HELPER FUNCTIONS
  static void paintHelper (const Vec4f & m, const Vec4f & s, const Vec4f & r, const Vec4f & t);
  static void Clear ()
  {
	main_segment.Clear ();
	shadow_segments.clear ();
	reflected_segments.clear ();
  }

  // REPRESENTATION
  static int activated;
  static Segment main_segment;
  static vector < Segment > shadow_segments;
  static vector < Segment > reflected_segments;
};

// ====================================================================
// ====================================================================

#endif
