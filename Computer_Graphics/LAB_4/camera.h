#ifndef _CAMERA_H_
#define _CAMERA_H_

#include <assert.h>
#include "vectors.h"

#include "ray.h"

// ====================================================================

class Camera
{

public:
  // CONSTRUCTOR & DESTRUCTOR
  Camera (Vec3f & c, Vec3f & p, Vec3f & u);
  virtual ~ Camera ()
  {
  }

  // RENDERING
  virtual Ray generateRay (Vec2f point) = 0;

  // GL NAVIGATION
  virtual void glInit (int w, int h) = 0;
  void glPlaceCamera (void);
  virtual void dollyCamera (float dist) = 0;
  virtual void zoomCamera (float dist) = 0;
  virtual void truckCamera (float dx, float dy) = 0;
  virtual void rotateCamera (float rx, float ry) = 0;

  const Vec3f & getCOI () const
  {
	return point_of_interest;
  }
protected:
    Camera ()
  {
	assert (0);
  }									   // don't use

  // HELPER FUNCTIONS
  Vec3f getHorizontal () const
  {
	Vec3f answer;
	  Vec3f::Cross3 (answer, getDirection (), up);
	  answer.Normalize ();
	  return answer;
  }
  Vec3f getScreenUp () const
  {
	Vec3f answer;
	  Vec3f::Cross3 (answer, getHorizontal (), getDirection ());
	  return answer;
  }
  Vec3f getDirection () const
  {
	Vec3f answer = point_of_interest - camera_position;
	  answer.Normalize ();
	  return answer;
  }

  // REPRESENTATION
  Vec3f point_of_interest;
  Vec3f camera_position;
  Vec3f up;
  int width;
  int height;
};


// ====================================================================

class PerspectiveCamera:public Camera
{

public:
  // CONSTRUCTOR & DESTRUCTOR
  PerspectiveCamera (Vec3f & c, Vec3f & p, Vec3f & u, float a);
   ~PerspectiveCamera (void)
  {
  }

  // RENDERING
  Ray generateRay (Vec2f point);

  // GL NAVIGATION
  void glInit (int w, int h);
  void dollyCamera (float dist);
  void zoomCamera (float dist);
  void truckCamera (float dx, float dy);
  void rotateCamera (float rx, float ry);
  friend ostream & operator<< (ostream & ostr, const PerspectiveCamera & c)
  {
	cout << "PerspectiveCamera {" << endl;
	cout << "  point_of_interest  " << c.point_of_interest;
	cout << "  camera_position    " << c.camera_position;
	cout << "  direction          " << c.getDirection ();
	cout << "  up                 " << c.up;
	cout << "  angle              " << c.angle << endl;
	cout << "}" << endl;
	return ostr;
  }

private:
  PerspectiveCamera ()
  {
	assert (0);
  }									   // don't use

  // REPRESENTATION
  float angle;
};

// ====================================================================

#endif
