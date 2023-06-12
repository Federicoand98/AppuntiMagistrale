#include <stdio.h>
#include <math.h>
#include <string.h>
#include <assert.h>
#include <iomanip>

#include "matrix.h"
#include "vectors.h"

using namespace std;

// ===================================================================
// ===================================================================
// CONSTRUCTORS

Matrix::Matrix (const Matrix & m)
{
  for (int i = 0; i < 16; i++)
  {
	data[i] = m.data[i];
  }
}

Matrix & Matrix::operator= (const Matrix & m)
{
  for (int i = 0; i < 16; i++)
  {
	data[i] = m.data[i];
  }
  return (*this);
}


// ===================================================================
// ===================================================================
// MODIFIERS

void
Matrix::SetToIdentity ()
{
  for (int row = 0; row < 4; row++)
  {
	for (int col = 0; col < 4; col++)
	{
	  Set (row, col, (row == col));
	}
  }
}

void
Matrix::Clear ()
{
  for (int row = 0; row < 4; row++)
  {
	for (int col = 0; col < 4; col++)
	{
	  Set (row, col, 0);
	}
  }
}

void
Matrix::Transpose (Matrix & m) const 
{
  // be careful, <this> might be <m>
  Matrix tmp = Matrix (*this);
  for (int row = 0; row < 4; row++)
  {
	for (int col = 0; col < 4; col++)
	{
	  m.Set (row, col, tmp.Get (col, row));
	}
  }
}

// ===================================================================
// ===================================================================
// INVERSE

int
Matrix::Inverse (Matrix & m, float epsilon) const
{
  m = *this;

  float a1, a2, a3, a4, b1, b2, b3, b4, c1, c2, c3, c4, d1, d2, d3, d4;
  a1 = m.Get (0, 0);
  b1 = m.Get (0, 1);
  c1 = m.Get (0, 2);
  d1 = m.Get (0, 3);
  a2 = m.Get (1, 0);
  b2 = m.Get (1, 1);
  c2 = m.Get (1, 2);
  d2 = m.Get (1, 3);
  a3 = m.Get (2, 0);
  b3 = m.Get (2, 1);
  c3 = m.Get (2, 2);
  d3 = m.Get (2, 3);
  a4 = m.Get (3, 0);
  b4 = m.Get (3, 1);
  c4 = m.Get (3, 2);
  d4 = m.Get (3, 3);

  float det = det4x4 (a1, a2, a3, a4, b1, b2, b3, b4, c1, c2, c3, c4, d1, d2, d3, d4);

  if (fabs (det) < epsilon)
  {
	printf ("Matrix::Inverse --- singular matrix, can't invert!\n");
	assert (0);
	return 0;
  }

  m.Set (0, 0, det3x3 (b2, b3, b4, c2, c3, c4, d2, d3, d4));
  m.Set (1, 0, -det3x3 (a2, a3, a4, c2, c3, c4, d2, d3, d4));
  m.Set (2, 0, det3x3 (a2, a3, a4, b2, b3, b4, d2, d3, d4));
  m.Set (3, 0, -det3x3 (a2, a3, a4, b2, b3, b4, c2, c3, c4));

  m.Set (0, 1, -det3x3 (b1, b3, b4, c1, c3, c4, d1, d3, d4));
  m.Set (1, 1, det3x3 (a1, a3, a4, c1, c3, c4, d1, d3, d4));
  m.Set (2, 1, -det3x3 (a1, a3, a4, b1, b3, b4, d1, d3, d4));
  m.Set (3, 1, det3x3 (a1, a3, a4, b1, b3, b4, c1, c3, c4));

  m.Set (0, 2, det3x3 (b1, b2, b4, c1, c2, c4, d1, d2, d4));
  m.Set (1, 2, -det3x3 (a1, a2, a4, c1, c2, c4, d1, d2, d4));
  m.Set (2, 2, det3x3 (a1, a2, a4, b1, b2, b4, d1, d2, d4));
  m.Set (3, 2, -det3x3 (a1, a2, a4, b1, b2, b4, c1, c2, c4));

  m.Set (0, 3, -det3x3 (b1, b2, b3, c1, c2, c3, d1, d2, d3));
  m.Set (1, 3, det3x3 (a1, a2, a3, c1, c2, c3, d1, d2, d3));
  m.Set (2, 3, -det3x3 (a1, a2, a3, b1, b2, b3, d1, d2, d3));
  m.Set (3, 3, det3x3 (a1, a2, a3, b1, b2, b3, c1, c2, c3));

  m *= 1 / det;
  return 1;
}

float
Matrix::det4x4 (float a1, float a2, float a3, float a4, float b1, float b2, float b3, float b4, float c1, float c2, float c3, float c4, float d1, float d2, float d3, float d4)
{
  return a1 * det3x3 (b2, b3, b4, c2, c3, c4, d2, d3, d4) - b1 * det3x3 (a2, a3, a4, c2, c3, c4, d2, d3, d4) + c1 * det3x3 (a2, a3, a4, b2, b3, b4, d2, d3, d4) - d1 * det3x3 (a2, a3, a4, b2, b3, b4, c2, c3, c4);
}

float
Matrix::det3x3 (float a1, float a2, float a3, float b1, float b2, float b3, float c1, float c2, float c3)
{
  return a1 * det2x2 (b2, b3, c2, c3) - b1 * det2x2 (a2, a3, c2, c3) + c1 * det2x2 (a2, a3, b2, b3);
}

float
Matrix::det2x2 (float a, float b, float c, float d)
{
  return a * d - b * c;
}

// ===================================================================
// ===================================================================
// OVERLOADED OPERATORS

Matrix
operator+ (const Matrix & m1, const Matrix & m2)
{
  Matrix answer;
  for (int i = 0; i < 16; i++)
  {
	answer.data[i] = m1.data[i] + m2.data[i];
  }
  return answer;
}

Matrix
operator- (const Matrix & m1, const Matrix & m2)
{
  Matrix answer;
  for (int i = 0; i < 16; i++)
  {
	answer.data[i] = m1.data[i] - m2.data[i];
  }
  return answer;
}

Matrix
operator* (const Matrix & m1, const Matrix & m2)
{
  Matrix answer;
  for (int row = 0; row < 4; row++)
  {
	for (int col = 0; col < 4; col++)
	{
	  for (int i = 0; i < 4; i++)
	  {
		answer.Set (row, col, answer.Get (row, col) + m1.Get (row, i) * m2.Get (i, col));
	  }
	}
  }
  return answer;
}

Matrix
operator* (const Matrix & m, float f)
{
  Matrix answer;
  for (int i = 0; i < 16; i++)
  {
	answer.data[i] = m.data[i] * f;
  }
  return answer;
}

// ====================================================================
// ====================================================================
// TRANSFORMATIONS



Matrix
Matrix::MakeTranslation (const Vec3f & v)
{
  Matrix t;
  t.SetToIdentity ();
  t.Set (0, 3, v.x ());
  t.Set (1, 3, v.y ());
  t.Set (2, 3, v.z ());
  return t;
}


Matrix
Matrix::MakeScale (const Vec3f & v)
{
  Matrix s;
  s.Set (0, 0, v.x ());
  s.Set (1, 1, v.y ());
  s.Set (2, 2, v.z ());
  s.Set (3, 3, 1);
  return s;
}


Matrix
Matrix::MakeXRotation (float theta)
{
  Matrix rx;
  rx.SetToIdentity ();
  rx.Set (1, 1, (float) cos ((float) theta));
  rx.Set (1, 2, -(float) sin ((float) theta));
  rx.Set (2, 1, (float) sin ((float) theta));
  rx.Set (2, 2, (float) cos ((float) theta));
  return rx;
}

Matrix
Matrix::MakeYRotation (float theta)
{
  Matrix ry;
  ry.SetToIdentity ();
  ry.Set (0, 0, (float) cos ((float) theta));
  ry.Set (0, 2, (float) sin ((float) theta));
  ry.Set (2, 0, -(float) sin ((float) theta));
  ry.Set (2, 2, (float) cos ((float) theta));
  return ry;
}

Matrix
Matrix::MakeZRotation (float theta)
{
  Matrix rz;
  rz.SetToIdentity ();
  rz.Set (0, 0, (float) cos ((float) theta));
  rz.Set (0, 1, -(float) sin ((float) theta));
  rz.Set (1, 0, (float) sin ((float) theta));
  rz.Set (1, 1, (float) cos ((float) theta));
  return rz;
}

Matrix
Matrix::MakeAxisRotation (const Vec3f & v, float theta)
{
  Matrix r;
  r.SetToIdentity ();

  float x = v.x ();
  float y = v.y ();
  float z = v.z ();

  float c = cosf (theta);
  float s = sinf (theta);
  float xx = x * x;
  float xy = x * y;
  float xz = x * z;
  float yy = y * y;
  float yz = y * z;
  float zz = z * z;

  r.Set (0, 0, (1 - c) * xx + c);
  r.Set (1, 0, (1 - c) * xy + z * s);
  r.Set (2, 0, (1 - c) * xz - y * s);

  r.Set (0, 1, (1 - c) * xy - z * s);
  r.Set (1, 1, (1 - c) * yy + c);
  r.Set (2, 1, (1 - c) * yz + x * s);

  r.Set (0, 2, (1 - c) * xz + y * s);
  r.Set (1, 2, (1 - c) * yz - x * s);
  r.Set (2, 2, (1 - c) * zz + c);


  return r;
}


// ====================================================================
// ====================================================================

void
Matrix::Transform (Vec4f & v) const
{
  Vec4f answer (0, 0, 0, 0);
  for (int row = 0; row < 4; row++)
  {
	for (int col = 0; col < 4; col++)
	{
	  answer.data[row] += Get (row, col) * v.data[col];
	}
  }
  v = answer;
}

// ====================================================================
// ====================================================================

ostream & operator<< (ostream & ostr, const Matrix & m)
{
  for (int row = 0; row < 4; row++)
  {
	for (int col = 0; col < 4; col++)
	{
	  float tmp = m.Get (row, col);
	  if (fabs (tmp) < 0.00001)
		tmp = 0;
	  ostr << setw (12) << tmp << " ";
	}
	ostr << endl;
  }
  return ostr;
}

istream & operator>> (istream & istr, Matrix & m)
{
  for (int row = 0; row < 4; row++)
  {
	for (int col = 0; col < 4; col++)
	{
	  float tmp;
	  istr >> tmp;
	  m.Set (row, col, tmp);
	}
  }
  return istr;
}

// ====================================================================
// ====================================================================
