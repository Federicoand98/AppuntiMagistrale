#include "raytree.h"
#include <GL\freeglut.h>

// ====================================================================
// Initialize the static variables
int RayTree::activated = 0;
Segment RayTree::main_segment;
vector < Segment > RayTree::shadow_segments;
vector < Segment > RayTree::reflected_segments;

// ====================================================================

void
RayTree::paintHelper (const Vec4f & m, const Vec4f & s, const Vec4f & r, const Vec4f & t)
{
  glBegin (GL_LINES);
  glColor4f (m.r (), m.g (), m.b (), m.a ());
  main_segment.paint ();
  unsigned int i;
  glColor4f (s.r (), s.g (), s.b (), s.a ());
  for (i = 0; i < shadow_segments.size (); i++)
  {
	shadow_segments[i].paint ();
  }
  glColor4f (r.r (), r.g (), r.b (), r.a ());
  for (i = 0; i < reflected_segments.size (); i++)
  {
	reflected_segments[i].paint ();
  }
  glColor4f (t.r (), t.g (), t.b (), t.a ());
  glEnd ();
}

// ====================================================================

void
RayTree::paint ()
{
  glLineWidth (2);
  glDisable (GL_LIGHTING);

  // this allows you to see rays passing through objects
  // turn off the depth test and blend with the current pixel color
  glDisable (GL_DEPTH_TEST);
  glEnable (GL_BLEND);
  glBlendFunc (GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
  paintHelper (Vec4f (0.7, 0.7, 0.7, 0.3), Vec4f (0.1, 0.9, 0.1, 0.3), Vec4f (0.9, 0.1, 0.1, 0.3), Vec4f (0.1, 0.1, 0.9, 0.3));
  glDisable (GL_BLEND);
  glEnable (GL_DEPTH_TEST);

  // with the depth test enabled, draw the lines solid
  paintHelper (Vec4f (0.7, 0.7, 0.7, 1.0), Vec4f (0.1, 0.9, 0.1, 1.0), Vec4f (0.9, 0.1, 0.1, 1.0), Vec4f (0.1, 0.1, 0.9, 1.0));

  glEnable (GL_LIGHTING);
}

// ====================================================================