#include <math.h>
#include <assert.h>

#include <GL\freeglut.h>

#include "camera.h"
#include "matrix.h"

// ====================================================================
// ====================================================================
// CONSTRUCTORS

Camera::Camera (Vec3f & c, Vec3f & p, Vec3f & u)
{
  camera_position = c;
  point_of_interest = p;
  up = u;
  up.Normalize ();
}

PerspectiveCamera::PerspectiveCamera (Vec3f & c, Vec3f & p, Vec3f & u, float a):
Camera (c, p, u)
{
  angle = a;
}

// ====================================================================
// ====================================================================
// GL INIT
// Create a camera with the appropriate dimensions that
// crops the screen in the narrowest dimension.

void
PerspectiveCamera::glInit (int w, int h)
{
  width = w;
  height = h;
  glMatrixMode (GL_PROJECTION);
  glLoadIdentity ();
  float aspect = float (width) / float (height);
  float asp_angle = angle * 180 / M_PI;
  if (aspect > 1)
	asp_angle /= aspect;
  gluPerspective (asp_angle, aspect, 0.1, 1000.0);
}

// ====================================================================
// ====================================================================
// GL PLACE CAMERA
// Place a camera within an OpenGL scene

void
Camera::glPlaceCamera (void)
{
  Vec3f lookAt = camera_position + getDirection ();
  gluLookAt (camera_position.x (), camera_position.y (), camera_position.z (), point_of_interest.x (), point_of_interest.y (), point_of_interest.z (), up.x (), up.y (), up.z ());
}

// ====================================================================
// dollyCamera: Move camera along the direction vector
// ====================================================================

void
PerspectiveCamera::dollyCamera (float dist)
{
  Vec3f diff = camera_position - point_of_interest;
  float d = diff.Length ();
  diff.Normalize ();
  d *= powf (1.003, dist);
  camera_position = point_of_interest + diff * d;
}

// ====================================================================
// zoomCamera: Change the camera angle
// ====================================================================

void
PerspectiveCamera::zoomCamera (float dist)
{
  angle *= powf (1.003, dist);
  glInit (width, height);
}

// ====================================================================
// truckCamera: Translate camera perpendicular to the direction vector
// ====================================================================

void
PerspectiveCamera::truckCamera (float dx, float dy)
{
  Vec3f diff = camera_position - point_of_interest;
  float d = diff.Length ();
  Vec3f translate = (d * 0.0007) * (getHorizontal () * dx + getScreenUp () * dy);
  camera_position += translate;
  point_of_interest += translate;
}

// ====================================================================
// rotateCamera: Rotate around the up and horizontal vectors
// ====================================================================

void
PerspectiveCamera::rotateCamera (float rx, float ry)
{
  // Don't let the model flip upside-down (There is a singularity
  // at the poles when 'up' and 'direction' are aligned)
  float tiltAngle = acos (up.Dot3 (getDirection ()));
  if (tiltAngle - ry > 3.13)
	ry = tiltAngle - 3.13;
  else if (tiltAngle - ry < 0.01)
	ry = tiltAngle - 0.01;

  Matrix rotMat;
  rotMat.SetToIdentity ();
  rotMat *= Matrix::MakeTranslation (point_of_interest);
  rotMat *= Matrix::MakeAxisRotation (up, rx);
  rotMat *= Matrix::MakeAxisRotation (getHorizontal (), ry);
  rotMat *= Matrix::MakeTranslation (-point_of_interest);
  rotMat.Transform (camera_position);
}

// ====================================================================
// ====================================================================
// GENERATE RAY

Ray
PerspectiveCamera::generateRay (Vec2f point)
{
  Vec3f screenCenter = camera_position + getDirection ();
  float screenHeight = tan (angle / 2.0);
  Vec3f xAxis = getHorizontal () * 2 * screenHeight;
  Vec3f yAxis = getScreenUp () * 2 * screenHeight;
  Vec3f lowerLeft = screenCenter - (getScreenUp () * screenHeight) - (getHorizontal () * screenHeight);
  Vec3f screenPoint = lowerLeft + xAxis * point.x () + yAxis * point.y ();
  Vec3f dir = screenPoint - camera_position;
  dir.Normalize ();
  return Ray (camera_position, dir);   //camera_position,dir);
}

// ====================================================================
