// Concordia University Graphics Library
// Author:     Peter Grogono
// Thanks to:  Qingzhe Huang, Yiping Le, Mark Kilgard,
//             Ken Shoemake, F.S. Hill, and others.
// Date:       06 June 2003
// Revised:    11 November 2005

// Commenting conventions:
// 1. Specifications given in the header file are not repeated here.
// 2. Comments are given only if the purpose of the code is not obvious.
// 3. Explanations of complex algorithms (e.g., quaternion to matrix)
//    are given in accompanying documentation rather than here.

#include "cugl.h"

#include <cmath>
#include <iostream>
#include <fstream>
#include <cstring>
#include <iomanip>

using namespace std;

using namespace cugl;
namespace cugl
{

const char *ErrorStrings[] =
{
   "No error",                                        // NO_ERROR
   "Invalid coordinate index",                        // BAD_INDEX
   "Invalid matrix mode",                             // BAD_MATRIX_MODE
   "Matrix is not a rotation transformation",         // BAD_ROTATION_MATRIX
   "Matrix is singular",                              // SINGULAR_MATRIX
   "Division by zero",                                // ZERO_DIVISOR
   "Normalizing null vector",                         // ZERO_NORM
   "Invalid interpolator argument",                   // BAD_INTERPOLATOR_ARG
   "Failed to open file",                             // OPEN_FAILED
   "File is not in BMP format",                       // NOT_BMP_FILE
   "File is not in 24-bit BMP format",                // NOT_24_BITS
   "File is in compressed BMP format",                // COMPRESSED_BMP_FILE
   "Memory allocation failed",                        // NOT_ENOUGH_MEMORY
   "Pixmap has not been initialized",                 // NO_PIX_MAP
   "Parameters do not define a plane",                // BAD_PLANE
   "Line lies in plane",                              // BAD_LINE
   "Too many materials",                              // TOO_MANY_MATERIALS
   "Too many points"                                  // TOO_MANY_POINTS
};

CUGLErrorType cuglError = NO_ERROR;

CUGLErrorType getError()
{
   CUGLErrorType currentValue = cuglError;
   cuglError = NO_ERROR;
   return currentValue;
}

const char *getErrorString(CUGLErrorType err)
{
   return ErrorStrings[err];
}

void checkOpenGLStatus()
{
   GLenum glError = glGetError();
   if (glError != GL_NO_ERROR)
      cerr <<
      "OpenGL error " << glError << ": " <<
      gluErrorString(glError) << endl;
}

void checkCUGLStatus()
{
   CUGLErrorType error = getError();
   if (error != NO_ERROR)
      cerr <<
      "CUGL error " << error << ": " <<
      getErrorString(error) << endl;
}

// Constructors for class Point.

Point::Point(GLfloat x, GLfloat y, GLfloat z, GLfloat w)
      : x(x), y(y), z(z), w(w)
{}


Point::Point (const Quaternion & q)
{
   Vector v = q.vector();
   x = v[0];
   y = v[1];
   z = v[2];
   w = q.scalar();
}

// Member functions for class Point.

// Return a reference to a component of point: i = 0,1,2,3.
GLfloat & Point::operator[](int i)
{
   switch (i)
   {
      default:
         cuglError = BAD_INDEX;
         // return x;
      case 0:
         return x;
      case 1:
         return y;
      case 2:
         return z;
      case 3:
         return w;
   }
}

// Return a const reference to a component of point: i = 0,1,2,3.
const GLfloat & Point::operator[](int i) const
{
   switch (i)
   {
      default:
         cuglError = BAD_INDEX;
         // return x;
      case 0:
         return x;
      case 1:
         return y;
      case 2:
         return z;
      case 3:
         return w;
   }
}

void Point::normalize()
{
   if (w == 0)
      cuglError = ZERO_DIVISOR;
   else
   {
      x = x / w;
      y = y / w;
      z = z / w;
      w = 1;
   }
}

Point Point::unit() const
{
   if (w == 0)
   {
      cuglError = ZERO_DIVISOR;
      return *this;
   }
   else
      return Point(x/w, y/w, z/w, 1);
}

// Friend functions for class Point

ostream & operator<<(ostream & os, const Point & p)
{
   if ( ! os.good() )
      return os;
   int width = os.width();
   os << setw(0) << '(' <<
   setw(width) << p[0] << ',' <<
   setw(width) << p[1] << ',' <<
   setw(width) << p[2] << ',' <<
   setw(width) << p[3] << ')';
   os.flush();
   return os;
}

// Member functions for class Line

void Line::draw() const
{
   glBegin(GL_LINES);
   s.draw();
   f.draw();
   glEnd();
}

// Friend functions for class Line

Point meet(const Line & k, const Plane & p)
{
   GLfloat den = p.a*(k.f.x-k.s.x) + p.b*(k.f.y-k.s.y) + p.c*(k.f.z-k.s.z) + p.d*(k.f.w-k.s.w);
   if (den == 0)
   {
      cuglError = BAD_LINE;
      return Point();
   }
   GLfloat x = p.b*(k.s.x*k.f.y-k.s.y*k.f.x) + p.c*(k.s.x*k.f.z-k.s.z*k.f.x) + p.d*(k.s.x*k.f.w-k.s.w*k.f.x);
   GLfloat y = p.a*(k.s.y*k.f.x-k.s.x*k.f.y) + p.c*(k.s.y*k.f.z-k.s.z*k.f.y) + p.d*(k.s.y*k.f.w-k.s.w*k.f.y);
   GLfloat z = p.a*(k.s.z*k.f.x-k.s.x*k.f.z) + p.b*(k.s.z*k.f.y-k.s.y*k.f.z) + p.d*(k.s.z*k.f.w-k.s.w*k.f.z);
   GLfloat w = p.a*(k.s.w*k.f.x-k.s.x*k.f.w) + p.b*(k.s.y*k.f.w-k.s.w*k.f.y) + p.c*(k.s.z*k.f.w-k.s.w*k.f.z);
   return Point(x, y, z, w);
}

ostream & operator<<(ostream & os, const Line & k)
{
   if ( ! os.good() )
      return os;
   int width = os.width();
   os <<
   setw(width) << k.s << "->" <<
   setw(width) << k.f;
   os.flush();
   return os;
}

// Constructors for class Plane.

// Construct the plane with equation ax + by + cd + dw = 0.
Plane::Plane(GLfloat a, GLfloat b, GLfloat c, GLfloat d)
      : a(a), b(b), c(c), d(d)
{
   if (a == 0 && b == 0 && c == 0)
   {
      cuglError = BAD_PLANE;
      a = 0;
      b = 1;
      c = 0;
      d = 0;
   }
}

// Construct the plane that contains points p, q, and r.
Plane::Plane(const Point & p, const Point & q, const Point & r)
{
   a = p[1] * (q[3]*r[2] - q[2]*r[3]) + q[1] * (p[2]*r[3] - p[3]*r[2]) + r[1] * (p[3]*q[2]-p[2]*q[3]);
   b = p[0] * (q[3]*r[2] - q[2]*r[3]) + q[0] * (p[2]*r[3] - p[3]*r[2]) + r[0] * (p[3]*q[2]-p[2]*q[3]);
   c = p[0] * (q[3]*r[1] - q[1]*r[3]) + q[0] * (p[1]*r[3] - p[3]*r[1]) + r[0] * (p[3]*q[1]-p[1]*q[3]);
   d = p[0] * (q[1]*r[2] - q[2]*r[1]) + q[0] * (p[2]*r[1] - p[1]*r[2]) + r[0] * (p[1]*q[2]-p[2]*q[1]);
   if (a == 0 && b == 0 && c == 0)
   {
      cuglError = BAD_PLANE;
      a = 0;
      b = 1;
      c = 0;
      d = 0;
   }
}

Plane::Plane(const Line & k, const Point & p)
{
   a =   p[1] * (k.s[2]*k.f[3] - k.s[3]*k.f[2]) + k.s[1] * (p[3]*k.f[2] - p[2]*k.f[3]) + k.f[1] * (p[2]*k.s[3] - p[3]*k.s[2]);
   b = - p[2] * (k.s[3]*k.f[0] - k.s[0]*k.f[3]) + k.s[2] * (p[0]*k.f[3] - p[3]*k.f[0]) + k.f[2] * (p[3]*k.s[0] - p[0]*k.s[3]);
   c =   p[3] * (k.s[0]*k.f[1] - k.s[1]*k.f[0]) + k.s[3] * (p[1]*k.f[0] - p[0]*k.f[1]) + k.f[3] * (p[0]*k.s[1] - p[1]*k.s[0]);
   d = - p[0] * (k.s[1]*k.f[2] - k.s[2]*k.f[1]) + k.s[0] * (p[2]*k.f[1] - p[1]*k.f[2]) + k.f[0] * (p[1]*k.s[2] - p[2]*k.s[1]);
   if (a == 0 && b == 0 && c == 0)
   {
      cuglError = BAD_PLANE;
      a = 0;
      b = 1;
      c = 0;
      d = 0;
   }
}


// Construct the plane orthogonal to v that contains p.
Plane::Plane(const Vector & v, const Point & p)
{
   a = v[0] * p[3];
   b = v[1] * p[3];
   c = v[2] * p[3];
   d = - (a * p[0] + b * p[1] + c * p[2]);
   if (a == 0 && b == 0 && c == 0)
   {
      cuglError = BAD_PLANE;
      a = 0;
      b = 1;
      c = 0;
      d = 0;
   }
}

// Member functions for class Plane.

// Normalize the plane.
void Plane::normalize()
{
   GLfloat s = float(sqrt(a*a + b*b + c*c));
   if (s == 0)
      cuglError = ZERO_DIVISOR;
   else
   {
      a = a / s;
      b = b / s;
      c = c / s;
      d = d / s;
   }
}

// Return an equivalent normal plane.
Plane Plane::unit() const
{
   GLfloat s = float(sqrt(a*a + b*b + c*c));
   if (s == 0)
   {
      cuglError = ZERO_DIVISOR;
      return *this;
   }
   else
      return Plane(a/s, b/s, c/s, d/s);
}

// Use this plane as an OpenGL clipping plane.
void Plane::clipPlane(GLenum index) const
{
   GLdouble eqn[4];
   eqn[0] = a;
   eqn[1] = b;
   eqn[2] = c;
   eqn[3] = d;
   glClipPlane(index, & eqn[0]);
}

// Friend functions for class Plane

ostream & operator<<(ostream & os, const Plane & p)
{
   if ( ! os.good() )
      return os;
   int width = os.width();
   os << setw(0) << '<' <<
   setw(width) << p.a << ',' <<
   setw(width) << p.b << ',' <<
   setw(width) << p.c << ',' <<
   setw(width) << p.d << '>';
   os.flush();
   return os;
}


// Constructor for class Vector.

// Construct the normal to a polygon defined by a set of points.
Vector::Vector(Point points[], int numPoints)
{
   // Normal to a set of points using Newell's algorithm.
   x = 0;
   y = 0;
   z = 0;
   for (int i = 0; i < numPoints; i++)
   {
      int next = i == numPoints - 1 ? 0 : i + 1;
      x += (points[i][1] - points[next][1]) * (points[i][2] + points[next][2]);
      y += (points[i][2] - points[next][2]) * (points[i][0] + points[next][0]);
      z += (points[i][0] - points[next][0]) * (points[i][1] + points[next][1]);
   }
}

// Member functions for class Vector

// Return a reference to a component of a vector.
GLfloat & Vector::operator[](int i)
{
   switch (i)
   {
      default:
         cuglError = BAD_INDEX;
         // return x;
      case 0:
         return x;
      case 1:
         return y;
      case 2:
         return z;
   }
}

// Return a const reference to a component of a vector.
const GLfloat & Vector::operator[](int i) const
{
   switch (i)
   {
      default:
         cuglError = BAD_INDEX;
         // return x;
      case 0:
         return x;
      case 1:
         return y;
      case 2:
         return z;
   }
}

// Normalize this vector in place and return the result.
void Vector::normalize()
{
   GLfloat len = length();
   if (len == 0)
      cuglError = ZERO_NORM;
   else
   {
      x /= len;
      y /= len;
      z /= len;
   }
}

// Return a unit vector with the same direction as this vector.
Vector Vector::unit() const
{
   GLfloat len = length();
   if (len == 0)
   {
      cuglError = ZERO_NORM;
      return Vector(0, 0, 0);
   }
   else
      return Vector(x/len, y/len, z/len);
}

// Divide each component of a vector by a scaling constant.
Vector Vector::operator/(GLfloat scale) const
{
   if (scale == 0)
   {
      cuglError = ZERO_DIVISOR;
      return *this;
   }
   else
      return Vector(x / scale, y / scale, z / scale);
}

// Scale this vector and return the result.
Vector Vector::operator/=(GLfloat scale)
{
   if (scale == 0)
      cuglError = ZERO_DIVISOR;
   else
   {
      x /= scale;
      y /= scale;
      z /= scale;
   }
   return *this;
}

ostream & operator<<(ostream & os, const Vector & v)
{
   if ( ! os.good() )
      return os;
   int width = os.width();
   os << setw(0) << '(' <<
   setw(width) << v[0] << ',' <<
   setw(width) << v[1] << ',' <<
   setw(width) << v[2] << ')';
   os.flush();
   return os;
}


// Constructors for class Matrix

// Construct the identity matrix.
Matrix::Matrix()
{
   for (int i = 0; i < 4; i++)
      for (int j = 0; j < 4; j++)
         m[i][j] = i == j ? 1.0f : 0.0f;
}

// Copy matrix r
Matrix::Matrix(GL_Matrix r)
{
   for (int i = 0; i < 4; i++)
      for (int j = 0; j < 4; j++)
         m[i][j] = r[i][j];
}

// Copy GL matrix
Matrix::Matrix(GLenum mode)
{
   if (  mode == GL_PROJECTION_MATRIX ||
         mode == GL_MODELVIEW_MATRIX)
   {
      GL_Matrix g;
      glGetFloatv(mode, &g[0][0]);
      for (int i = 0; i < 4; i++)
         for (int j = 0; j < 4; j++)
            m[i][j] = g[i][j];
   }
   else
   {
      cuglError = BAD_MATRIX_MODE;
      // Construct identity matrix.
      for (int i = 0; i < 4; i++)
         for (int j = 0; j < 4; j++)
            m[i][j] = i == j ? 1.0f : 0.0f;
   }
}

// Gives correct result for glRotatef
Matrix::Matrix(const Vector & axis, double theta)
{
   Vector u = axis.unit();
   double s = sin(theta);
   double c = cos(theta);
   double cc = 1 - c;

   m[0][0] = GLfloat(cc * u[0] * u[0] + c);
   m[1][0] = GLfloat(cc * u[1] * u[0] - s * u[2]);
   m[2][0] = GLfloat(cc * u[2] * u[0] + s * u[1]);
   m[3][0] = 0;

   m[0][1] = GLfloat(cc * u[0] * u[1] + s * u[2]);
   m[1][1] = GLfloat(cc * u[1] * u[1] + c);
   m[2][1] = GLfloat(cc * u[2] * u[1] - s * u[0]);
   m[3][1] = 0;

   m[0][2] = GLfloat(cc * u[0] * u[2] - s * u[1]);
   m[1][2] = GLfloat(cc * u[1] * u[2] + s * u[0]);
   m[2][2] = GLfloat(cc * u[2] * u[2] + c);
   m[3][2] = 0;

   m[0][3] = 0;
   m[1][3] = 0;
   m[2][3] = 0;
   m[3][3] = 1;
}

Matrix::Matrix(const Vector & u, const Vector & v)
{
   Vector axis = cross(v, u);
   double sina = axis.length();
   double cosa = sqrt(1 - sina * sina);
   axis.normalize();
   double omc = 1 - cosa;

   m[0][0] = GLfloat(omc * axis[0] * axis[0] + cosa);
   m[1][0] = GLfloat(omc * axis[1] * axis[0] - sina * axis[2]);
   m[2][0] = GLfloat(omc * axis[2] * axis[0] + sina * axis[1]);
   m[3][0] = 0;

   m[0][1] = GLfloat(omc * axis[0] * axis[1] + sina * axis[2]);
   m[1][1] = GLfloat(omc * axis[1] * axis[1] + cosa);
   m[2][1] = GLfloat(omc * axis[2] * axis[1] - sina * axis[0]);
   m[3][1] = 0;

   m[0][2] = GLfloat(omc * axis[0] * axis[2] - sina * axis[1]);
   m[1][2] = GLfloat(omc * axis[1] * axis[2] + sina * axis[0]);
   m[2][2] = GLfloat(omc * axis[2] * axis[2] + cosa);
   m[3][2] = 0;

   m[0][3] = 0;
   m[1][3] = 0;
   m[2][3] = 0;
   m[3][3] = 1;
}


Matrix::Matrix(const Quaternion & q)
{
   m[0][0] = GLfloat(1 - 2 * (q.v.y * q.v.y + q.v.z * q.v.z));
   m[1][0] = GLfloat(2 * (q.v.x * q.v.y - q.v.z * q.s));
   m[2][0] = GLfloat(2 * (q.v.z * q.v.x + q.v.y * q.s));
   m[3][0] = 0;

   m[0][1] = GLfloat(2 * (q.v.x * q.v.y + q.v.z * q.s));
   m[1][1] = GLfloat(1 - 2 * (q.v.z * q.v.z + q.v.x * q.v.x));
   m[2][1] = GLfloat(2 * (q.v.y * q.v.z - q.v.x * q.s));
   m[3][1] = 0;

   m[0][2] = GLfloat(2 * (q.v.z * q.v.x - q.v.y * q.s));
   m[1][2] = GLfloat(2 * (q.v.y * q.v.z + q.v.x * q.s));
   m[2][2] = GLfloat(1 - 2 * (q.v.y * q.v.y + q.v.x * q.v.x));
   m[3][2] = 0;

   m[0][3] = 0;
   m[1][3] = 0;
   m[2][3] = 0;
   m[3][3] = 1;
}


// Matrix to reflect in plane p
void Matrix::reflect(const Plane & p)
{
   GLfloat q = p.a * p.a + p.b * p.b + p.c * p.c;
   m[0][0] = q - 2 * p.a * p.a;
   m[1][0] = - 2 * p.a * p.b;
   m[2][0] = - 2 * p.a * p.c;
   m[3][0] = - 2 * p.a * p.d;

   m[0][1] = - 2 * p.a * p.b;
   m[1][1] = q - 2 * p.b * p.b;
   m[2][1] = - 2 * p.b * p.c;
   m[3][1] = - 2 * p.b * p.d;

   m[0][2] = - 2 * p.a * p.c;
   m[1][2] = - 2 * p.b * p.c;
   m[2][2] = q - 2 * p.c * p.c;
   m[3][2] = - 2 * p.c * p.d;

   m[0][3] = 0;
   m[1][3] = 0;
   m[2][3] = 0;
   m[3][3] = q;
}


// Shadow matrix for point and plane.
// OpenGL does not seem to like matrices with m[3][3] < 0.
// Consequently, we ensure that m[3][3] >= 0.
void Matrix::shadow(const Point & s, const Plane & p)
{
   GLfloat k = p.a * s[0] + p.b * s[1] + p.c * s[2] + p.d * s[3];
   GLfloat m33 = p.d * s[3] - k;
   if (m33 >= 0)
   {
      m[0][0] = p.a * s[0] - k;
      m[1][0] = p.b * s[0];
      m[2][0] = p.c * s[0];
      m[3][0] = p.d * s[0];

      m[0][1] = p.a * s[1];
      m[1][1] = p.b * s[1] - k;
      m[2][1] = p.c * s[1];
      m[3][1] = p.d * s[1];

      m[0][2] = p.a * s[2];
      m[1][2] = p.b * s[2];
      m[2][2] = p.c * s[2] - k;
      m[3][2] = p.d * s[2];

      m[0][3] = p.a * s[3];
      m[1][3] = p.b * s[3];
      m[2][3] = p.c * s[3];
      m[3][3] = m33;
   }
   else
   {
      m[0][0] = k - p.a * s[0];
      m[1][0] = - p.b * s[0];
      m[2][0] = - p.c * s[0];
      m[3][0] = - p.d * s[0];

      m[0][1] = - p.a * s[1];
      m[1][1] = k - p.b * s[1];
      m[2][1] = - p.c * s[1];
      m[3][1] = - p.d * s[1];

      m[0][2] = - p.a * s[2];
      m[1][2] = - p.b * s[2];
      m[2][2] = k - p.c * s[2];
      m[3][2] = - p.d * s[2];

      m[0][3] = - p.a * s[3];
      m[1][3] = - p.b * s[3];
      m[2][3] = - p.c * s[3];
      m[3][3] = - m33;
   }
}

// Member functions for class Matrix

Matrix Matrix::transpose() const
{
   Matrix t;
   for (int r = 0; r < 4; r++)
      for (int c = 0; c < 4; c++)
         t(r,c) = m[c][r];
   return t;
}

Matrix Matrix::inv() const
{
   // Construct augmented 4 * 8 matrix.
   GLdouble a[4][8];
   for (int r = 0; r < 4; ++r)
      for (int c = 0; c < 8; ++c)
         a[r][c] = c < 4 ? m[c][r] : c == r + 4;
   for (int col = 0; col < 4; ++col)
   {
      // Find largest entry in current column
      int max = col;
      for (int r = col + 1; r < 4; ++r)
         if (a[max][col] < a[r][col])
            max = r;
      // swap rows to get largest element on diagonal
      if (max != col)
      {
         for (int c = 0; c < 8; ++c)
         {
            GLdouble t = a[max][c];
            a[max][c] = a[col][c];
            a[col][c] = t;
         }
      }
      // Zero on diagonal indicates a singular matrix
      if (fabs(a[col][col]) < 1e-6)
      {
//            cuglError = SINGULAR_MATRIX;
         cout << "Singular!\n";
         return *this;
      }
      // Scale rows to get zeroes bove and below diagonal
      for (int r = 0; r < 4; ++r)
      {
         if (r != col)
         {
            GLdouble fac = a[r][col] / a[col][col];
            for (int c = 0; c < 8; ++c)
               a[r][c] -= fac * a[col][c];
         }
      }
   }
   // Scale rows so that left half is the identity matrix
   for (int r = 0; r < 4; ++r)
   {
      GLdouble fac = a[r][r];
      for (int c = 0; c < 8; ++c)
         a[r][c] /= fac;
   }
   // Get the result from the right half
   Matrix res;
   for (int r = 0; r < 4; ++r)
      for (int c = 0; c < 4; ++c)
         res(c,r) = a[r][c+4];
   return res;
}

//Matrix Matrix::inv() const
//{
//   GLfloat det =
//      - m[0][0] * m[1][1] * m[2][2] * m[3][3]
//      + m[0][0] * m[1][1] * m[2][3] * m[3][2]
//      + m[0][0] * m[2][1] * m[1][2] * m[3][3]
//      - m[0][0] * m[2][1] * m[1][3] * m[3][2]
//      - m[0][0] * m[3][1] * m[1][2] * m[2][3]
//      + m[0][0] * m[3][1] * m[1][3] * m[2][2]
//      + m[1][0] * m[0][1] * m[2][2] * m[3][3]
//      - m[1][0] * m[0][1] * m[2][3] * m[3][2]
//      - m[1][0] * m[2][1] * m[0][2] * m[3][3]
//      + m[1][0] * m[2][1] * m[0][3] * m[3][2]
//      + m[1][0] * m[3][1] * m[0][2] * m[2][3]
//      - m[1][0] * m[3][1] * m[0][3] * m[2][2]
//      - m[2][0] * m[0][1] * m[1][2] * m[3][3]
//      + m[2][0] * m[0][1] * m[1][3] * m[3][2]
//      + m[2][0] * m[1][1] * m[0][2] * m[3][3]
//      - m[2][0] * m[1][1] * m[0][3] * m[3][2]
//      - m[2][0] * m[3][1] * m[0][2] * m[1][3]
//      + m[2][0] * m[3][1] * m[0][3] * m[1][2]
//      + m[3][0] * m[0][1] * m[1][2] * m[2][3]
//      - m[3][0] * m[0][1] * m[1][3] * m[2][2]
//      - m[3][0] * m[1][1] * m[0][2] * m[2][3]
//      + m[3][0] * m[1][1] * m[0][3] * m[2][2]
//      + m[3][0] * m[2][1] * m[0][2] * m[1][3]
//      - m[3][0] * m[2][1] * m[0][3] * m[1][2];
//
//   Matrix inv;
//   if (det == 0)
//      cuglError = SINGULAR_MATRIX;
//   else
//   {
//      inv.m[0][0] =  (-m[1][1] * m[2][2] * m[3][3] + m[1][1] * m[2][3] * m[3][2]
//                      + m[2][1] * m[1][2] * m[3][3] - m[2][1] * m[1][3] * m[3][2]
//                      - m[3][1] * m[1][2] * m[2][3] + m[3][1] * m[1][3] * m[2][2]) / det;
//      inv.m[0][1] = -(-m[0][1] * m[2][2] * m[3][3] + m[0][1] * m[2][3] * m[3][2]
//                      + m[2][1] * m[0][2] * m[3][3] - m[2][1] * m[0][3] * m[3][2]
//                      - m[3][1] * m[0][2] * m[2][3] + m[3][1] * m[0][3] * m[2][2]) / det;
//      inv.m[0][2] = - (m[0][1] * m[1][2] * m[3][3] - m[0][1] * m[1][3] * m[3][2]
//                       - m[1][1] * m[0][2] * m[3][3] + m[1][1] * m[0][3] * m[3][2]
//                       + m[3][1] * m[0][2] * m[1][3] - m[3][1] * m[0][3] * m[1][2]) / det;
//      inv.m[0][3] =- (-m[0][1] * m[1][2] * m[2][3] + m[0][1] * m[1][3] * m[2][2]
//                      + m[1][1] * m[0][2] * m[2][3] - m[1][1] * m[0][3] * m[2][2]
//                      - m[2][1] * m[0][2] * m[1][3] + m[2][1] * m[0][3] * m[1][2]) / det;
//      inv.m[1][0] = -(-m[1][0] * m[2][2] * m[3][3] + m[1][0] * m[2][3] * m[3][2]
//                      + m[2][0] * m[1][2] * m[3][3] - m[2][0] * m[1][3] * m[3][2]
//                      - m[3][0] * m[1][2] * m[2][3] + m[3][0] * m[1][3] * m[2][2]) / det;
//      inv.m[1][1] =  (-m[0][0] * m[2][2] * m[3][3] + m[0][0] * m[2][3] * m[3][2]
//                      + m[2][0] * m[0][2] * m[3][3] - m[2][0] * m[0][3] * m[3][2]
//                      - m[3][0] * m[0][2] * m[2][3] + m[3][0] * m[0][3] * m[2][2]) / det;
//      inv.m[1][2] = -(-m[0][0] * m[1][2] * m[3][3] + m[0][0] * m[1][3] * m[3][2]
//                      + m[1][0] * m[0][2] * m[3][3] - m[1][0] * m[0][3] * m[3][2]
//                      - m[3][0] * m[0][2] * m[1][3] + m[3][0] * m[0][3] * m[1][2]) / det;
//      inv.m[1][3] =  (-m[0][0] * m[1][2] * m[2][3] + m[0][0] * m[1][3] * m[2][2]
//                      + m[1][0] * m[0][2] * m[2][3] - m[1][0] * m[0][3] * m[2][2]
//                      - m[2][0] * m[0][2] * m[1][3] + m[2][0] * m[0][3] * m[1][2]) / det;
//      inv.m[2][0] =  (-m[1][0] * m[2][1] * m[3][3] + m[1][0] * m[2][3] * m[3][1]
//                      + m[2][0] * m[1][1] * m[3][3] - m[2][0] * m[1][3] * m[3][1]
//                      - m[3][0] * m[1][1] * m[2][3] + m[3][0] * m[1][3] * m[2][1]) / det;
//      inv.m[2][1] = -(-m[0][0] * m[2][1] * m[3][3] + m[0][0] * m[2][3] * m[3][1]
//                      + m[2][0] * m[0][1] * m[3][3] - m[2][0] * m[0][3] * m[3][1]
//                      - m[3][0] * m[0][1] * m[2][3] + m[3][0] * m[0][3] * m[2][1]) / det;
//      inv.m[2][2] =  -(m[0][0] * m[1][1] * m[3][3] - m[0][0] * m[1][3] * m[3][1]
//                       - m[1][0] * m[0][1] * m[3][3] + m[1][0] * m[0][3] * m[3][1]
//                       + m[3][0] * m[0][1] * m[1][3] - m[3][0] * m[0][3] * m[1][1]) / det;
//      inv.m[2][3] =   (m[0][0] * m[1][1] * m[2][3] - m[0][0] * m[1][3] * m[2][1]
//                       - m[1][0] * m[0][1] * m[2][3] + m[1][0] * m[0][3] * m[2][1]
//                       + m[2][0] * m[0][1] * m[1][3] - m[2][0] * m[0][3] * m[1][1]) / det;
//      inv.m[3][0] = -(-m[1][0] * m[2][1] * m[3][2] + m[1][0] * m[2][2] * m[3][1]
//                      + m[2][0] * m[1][1] * m[3][2] - m[2][0] * m[1][2] * m[3][1]
//                      - m[3][0] * m[1][1] * m[2][2] + m[3][0] * m[1][2] * m[2][1]) / det;
//      inv.m[3][1] =  (-m[0][0] * m[2][1] * m[3][2] + m[0][0] * m[2][2] * m[3][1]
//                      + m[2][0] * m[0][1] * m[3][2] - m[2][0] * m[0][2] * m[3][1]
//                      - m[3][0] * m[0][1] * m[2][2] + m[3][0] * m[0][2] * m[2][1]) / det;
//      inv.m[3][2] =   (m[0][0] * m[1][1] * m[3][2] - m[0][0] * m[1][2] * m[3][1]
//                       - m[1][0] * m[0][1] * m[3][2] + m[1][0] * m[0][2] * m[3][1]
//                       + m[3][0] * m[0][1] * m[1][2] - m[3][0] * m[0][2] * m[1][1]) / det;
//      inv.m[3][3] =  -(m[0][0] * m[1][1] * m[2][2] - m[0][0] * m[1][2] * m[2][1]
//                       - m[1][0] * m[0][1] * m[2][2] + m[1][0] * m[0][2] * m[2][1]
//                       + m[2][0] * m[0][1] * m[1][2] - m[2][0] * m[0][2] * m[1][1]) / det;
//   }
//   return inv;
//}

double Matrix::angle() const
{
   double cosAngle = 0.5 * (m[0][0] + m[1][1] + m[2][2] - 1);
   if (fabs(cosAngle) > 1)
   {
      cuglError = BAD_ROTATION_MATRIX;
      return 0;
   }
   else
      return acos(cosAngle);
}

Vector Matrix::axis() const
{
   GLfloat twoSine = GLfloat(2 * sin(angle()));
   if (twoSine == 0)
   {
      cuglError = BAD_ROTATION_MATRIX;
      return Vector();
   }
   else
      return Vector(
                (m[1][2] - m[2][1]) / twoSine,
                (m[2][0] - m[0][2]) / twoSine,
                (m[0][1] - m[1][0]) / twoSine );
}

ostream & operator<<(ostream & os, const Matrix & m)
{
   if ( ! os.good() )
      return os;
   int width = os.width();
   for (int i = 0; i < 4; i++)
   {
      os << setw(0) << '(';
      for (int j = 0; j < 4; j++)
         os << setw(width) << m(j,i);
      os << ')' << endl;
   }
   os.flush();
   return os;
}

Quaternion Matrix::quaternion() const
{
   // Gives consistent results.
   double sqs = m[0][0] + m[1][1] + m[2][2] + m[3][3];
   if (sqs < 0)
   {
      cuglError = BAD_ROTATION_MATRIX;
      return Quaternion();
   }
   GLfloat scale = GLfloat(sqrt(sqs));
   return Quaternion(scale / 2,
                     Vector
                     (
                        (m[1][2] - m[2][1]) / (2 * scale),
                        (m[2][0] - m[0][2]) / (2 * scale),
                        (m[0][1] - m[1][0]) / (2 * scale)
                     ));
}

// Constructors for class Quaternion

Quaternion::Quaternion(Matrix m)
{
   double sqs = m(0,0) + m(1,1) + m(2,2) + m(3,3);
   if (sqs < 0)
   {
      cuglError = BAD_ROTATION_MATRIX;
      s = 1;
      v = Vector(0,0,0);
   }
   GLfloat scale = GLfloat(sqrt(sqs));
   s = scale / 2;
   v = Vector
       (
          (m(1,2) - m(2,1)) / (2 * scale),
          (m(2,0) - m(0,2)) / (2 * scale),
          (m(0,1) - m(1,0)) / (2 * scale)
       );
}

Quaternion::Quaternion(double xr, double yr, double zr)
{
   double cos_z = cos(zr);
   double sin_z = sin(zr);
   double cos_y = cos(yr);
   double sin_y = sin(yr);
   double cos_x = cos(xr);
   double sin_x = sin(xr);
   double ds = sqrt(1 + cos_z * cos_y + cos_z * cos_x + sin_z * sin_y * sin_x + cos_y * cos_x) / 2;
   double s4 = 4 * ds;
   v.x = GLfloat((cos_y * sin_x + cos_z * sin_x - sin_z * sin_y * cos_x) / s4);
   v.y = GLfloat((sin_z * sin_x + cos_z * sin_y * cos_x + sin_y) / s4);
   v.z = GLfloat((sin_z * cos_y + sin_z * cos_x - cos_z * sin_y * sin_x) / s4);
   s = GLfloat(ds);
}

Quaternion::Quaternion (const Point & p)
{
   v[0] = p[0];
   v[1] = p[1];
   v[2] = p[2];
   s    = p[3];
}

Quaternion::Quaternion(const Vector & u, const Vector & w)
{
   Vector axis = cross(u, w);
   double angle = acos(dot(u, w));
   v = GLfloat(sin(angle/2)) * axis.unit();
   s = GLfloat(cos(angle/2));
}

// Member functions for class Quaternion

Quaternion Quaternion::operator/(GLfloat scale) const
{
   if (scale == 0)
   {
      cuglError = ZERO_DIVISOR;
      return *this;
   }
   else
      return Quaternion(s / scale, v / scale);
}

void Quaternion::normalize()
{
   GLfloat m = magnitude();
   if (m == 0)
      cuglError = ZERO_DIVISOR;
   else
   {
      s /= m;
      v /= m;
   }
}

Quaternion Quaternion::unit() const
{
   GLfloat m = magnitude();
   if (m == 0)
   {
      cuglError = ZERO_DIVISOR;
      return *this;
   }
   else
      return Quaternion(s/m, v/m);
}

Quaternion Quaternion::inv() const
{
   GLfloat n = norm();
   if (n == 0)
   {
      cuglError = ZERO_DIVISOR;
      return *this;
   }
   else
      return (*this).conj() / n;
}

void Quaternion::matrix(Matrix & m) const
{
   m(0,0) = GLfloat(1 - 2 * (v.y * v.y + v.z * v.z));
   m(1,0) = GLfloat(2 * (v.x * v.y - v.z * s));
   m(2,0) = GLfloat(2 * (v.z * v.x + v.y * s));
   m(3,0) = 0;

   m(0,1) = GLfloat(2 * (v.x * v.y + v.z * s));
   m(1,1) = GLfloat(1 - 2 * (v.z * v.z + v.x * v.x));
   m(2,1) = GLfloat(2 * (v.y * v.z - v.x * s));
   m(3,1) = 0;

   m(0,2) = GLfloat(2 * (v.z * v.x - v.y * s));
   m(1,2) = GLfloat(2 * (v.y * v.z + v.x * s));
   m(2,2) = GLfloat(1 - 2 * (v.y * v.y + v.x * v.x));
   m(3,2) = 0;

   m(0,3) = 0;
   m(1,3) = 0;
   m(2,3) = 0;
   m(3,3) = 1;
}

void Quaternion::matrix(GL_Matrix m) const
{
   // Gives same direction as glRotatef
   m[0][0] = GLfloat(1 - 2 * (v.y * v.y + v.z * v.z));
   m[1][0] = GLfloat(2 * (v.x * v.y - v.z * s));
   m[2][0] = GLfloat(2 * (v.z * v.x + v.y * s));
   m[3][0] = 0;

   m[0][1] = GLfloat(2 * (v.x * v.y + v.z * s));
   m[1][1] = GLfloat(1 - 2 * (v.z * v.z + v.x * v.x));
   m[2][1] = GLfloat(2 * (v.y * v.z - v.x * s));
   m[3][1] = 0;

   m[0][2] = GLfloat(2 * (v.z * v.x - v.y * s));
   m[1][2] = GLfloat(2 * (v.y * v.z + v.x * s));
   m[2][2] = GLfloat(1 - 2 * (v.y * v.y + v.x * v.x));
   m[3][2] = 0;

   m[0][3] = 0;
   m[1][3] = 0;
   m[2][3] = 0;
   m[3][3] = 1;
}

void Quaternion::apply() const
{
   GL_Matrix m;
   matrix(m);
   glMultMatrixf(&m[0][0]);
}

void Quaternion::euler(double & xr, double & yr, double & zr) const
{
   double sqs = s * s;
   double sqx = v.x * v.x;
   double sqy = v.y * v.y;
   double sqz = v.z * v.z;

   xr = yr = zr = 0;
   xr = atan2(static_cast<double>(2 * (v.y * v.z + v.x * s)), - sqx - sqy + sqz + sqs);
   yr = asin(- 2 * (v.x * v.z - v.y * s));
   zr = atan2(static_cast<double>(2 * (v.x * v.y + v.z * s)),sqx - sqy - sqz + sqs);
}

void Quaternion::integrate(const Vector & omega, double dt)
{
   // 090801 reversed order of multiplication
   *this = Quaternion(omega.unit(), omega.length() * dt) * (*this);
}

const double BALLRADIUS = 0.8f;
const double BRADSQ = BALLRADIUS * BALLRADIUS;

// Helper function for trackball().
GLfloat project(GLfloat x, GLfloat y)
// Expect: |x| <= 1 and |y| <= 1.
// Project the point (x,y) onto either a sphere or a
// hyperboloid, depending on its distance from the origin.
{
   double dsq = x * x + y * y;
   double d = ::sqrt(dsq);
   if (d < BALLRADIUS * 0.5 * sqrt(2.0))
      // sphere
      return GLfloat(sqrt(BRADSQ - dsq));
   else
      // hyperbola
      return GLfloat(BRADSQ / (2 * d));
}

void Quaternion::trackball(GLfloat x1, GLfloat y1, GLfloat x2, GLfloat y2)
{
   // Project the mouse points onto a sphere or hyperboloid.
   Vector v1(x1, y1, project(x1, y1));
   Vector v2(x2, y2, project(x2, y2));

   // Construct the quaternion from these vectors.
   Vector a = v2 * v1;
   Vector d = v1 - v2;
   double t = d.length() / (2 * BALLRADIUS);
   if (t > 1)
      t = 1;
   if (t < -1)
      t = -1;
   double theta = asin(t);
   (*this) *= Quaternion(GLfloat(cos(theta)), a.unit() * GLfloat(sin(theta)));
};

// Friend functions for class Quaternion

//Vector ln(const Quaternion & q)
//{
//   double theta = q.s;
//   return (GLfloat(theta == 0 ? 1 : theta / sin(theta))) * q.vector();
//}
//
//Quaternion exp(const Vector & v)
//{
//   double theta = v.length();
//   return Quaternion(GLfloat(cos(theta)), GLfloat(theta == 0 ? 1 : sin(theta) / theta) * v);
//}

ostream & operator<<(ostream & os, const Quaternion & q)
{
   if ( ! os.good() )
      return os;
   int width = os.width();
   os <<
   setw(width) << q.s << ' ' <<
   setw(width) << q.v;
   os.flush();
   return os;
}





// Constants for class Camera

const Point EYE(0, 0, 1);
const Point MODEL(0, 0, 0);
const Vector UP(0, 1, 0);

// Constructors for class Camera
// property "smooth" is becoming property of ancester class BaseCamera
// so does steps and maxSteps
Camera::Camera() :
      eye(EYE),
      eyeOld(EYE),
      eyeNew(EYE),
      model(MODEL),
      modelOld(MODEL),
      modelNew(MODEL),
      up(UP)
{ }

Camera::Camera(const Point & eye, const Point & model, const Vector & up) :
      eye(eye),
      eyeOld(eye),
      eyeNew(eye),
      model(model),
      modelOld(model),
      modelNew(model),
      up(up.unit())
{ }

Camera::Camera(const Point & eye, const Point & model) :
      eye(eye),
      eyeOld(eye),
      eyeNew(eye),
      model(model),
      modelOld(model),
      modelNew(model),
      up(Vector(0,1,0)) //peter - this was up(J), which didn't work properly
      //up(J),
{  }

Camera::Camera(const Point & eye) :
      eye(eye),
      eyeOld(eye),
      eyeNew(eye),
      model(MODEL),
      modelOld(MODEL),
      modelNew(MODEL),
      up(J)
{ }

// Member functions for class Camera

void Camera::set(const Point & e, const Point & m, const Vector & u)
{
   eye = e;
   eyeOld = e;
   eyeNew = e;
   model = m;
   modelOld = m;
   modelNew = m;
   up = u.unit();
}

void Camera::set(const Point & e, const Point & m)
{
   eye = e;
   eyeOld = e;
   eyeNew = e;
   model = m;
   modelOld = m;
   modelNew = m;
   up = J;
}

void Camera::set(const Point & e)
{
   eye = e;
   eyeOld = e;
   eyeNew = e;
   model = MODEL;
   modelOld = MODEL;
   modelNew = MODEL;
   up = J;
}

void Camera::apply() const
{
   gluLookAt
   (
      eye[0], eye[1], eye[2],
      model[0], model[1], model[2],
      up[0], up[1], up[2]
   );
}

void Camera::update(const Point & e, const Point & m)
{
   eyeNew = e;
   modelNew = m;
   if (smooth)
   {
      if (steps > 0)
      {
         eyeOld = eye;
         modelOld = model;
      }
      steps = maxSteps;

   }
   else
   {
      eye = eyeNew;
      model = modelNew;
   }
}

void Camera::moveUp(GLfloat distance)
{
   Point left, right;
   Vector disp = distance * up;

   //here the original Point operator+ overloading is returning Point
   //which is a value, however, update requires & reference,
   //since I am not clear about the purpose of Point+Point
   //I won't change code in Point overloading
   //update(eyeOld + disp, modelOld + disp);
   left = eyeOld+disp;
   right = modelOld+disp;
   update(left, right);
}

void Camera::moveForward(GLfloat distance)
{
   //same problem in moveup function
   Point left, right;
   Vector disp = distance * (model - eye).unit();
   //update(eyeOld + disp, modelOld + disp);
   left = eyeOld + disp;
   right = modelOld + disp;
   update(left, right);
}

void Camera::moveLeft(GLfloat distance)
{
   Point left, right;
   Vector disp = distance * cross(up, model - eye).unit();
   //update(eyeOld + disp, modelOld + disp);
   left = eyeOld + disp;
   right = modelOld + disp;
   update(left, right);
}

void Camera::tiltUp(double angle)
{
   Point temp;
   Vector axis = cross(eyeOld - modelOld, up).unit();
   Quaternion rot(axis, angle);
   temp = eyeOld+rot.apply(modelOld - eyeOld);//added by nick
   //the same problem is that g++ is subtly different in parameter passing.
   //it might just check the syntax during parsing, and decide strictly by
   //comparing the function prototype
   //update(eyeOld, eyeOld + rot.apply(modelOld - eyeOld));
   update(eyeOld, temp);
}

//same problem in tiltUp
void Camera::panLeft(double angle)
{
   Point temp;
   Quaternion rot(up, -angle);
   temp = eyeOld+rot.apply(modelOld - eyeOld);
   //update(eyeOld, eyeOld + rot.apply(modelOld - eyeOld));
   update(eyeOld, temp);
}

void Camera::idle()
{
   if (steps > 0)
   {
      GLfloat t = GLfloat(steps) / GLfloat(maxSteps);
      eye = eyeNew + t * (eyeOld - eyeNew);
      model = modelNew + t * (modelOld - modelNew);
      steps--;
   }
   else
   {
      eyeOld = eyeNew;
      modelOld = modelNew;
      steps = 0;
   }
   //added by nick
   //removed by peter
   //apply();
}

ostream & operator<<(ostream & os, const Camera & c)
{
   return os <<
          "Eye: " << c.eye <<
          "  Model: " << c.model <<
          "  Up: " << c.up << endl;
}

// Constructors for class Interpolator

Interpolator::Interpolator()
      : first(Quaternion(0, Vector(1, 0, 0))), last(Quaternion(0, Vector(0, 1, 0)))
{
   initialize();
}

Interpolator::Interpolator(const Quaternion & qFirst, const Quaternion & qLast)
      : first(qFirst), last(qLast)
{
   initialize();
}


// Member functions for class Interpolator

void Interpolator::set(const Quaternion & qFirst, const Quaternion & qLast)
{
   first = qFirst;
   last = qLast;
   initialize();
   steps=maxSteps;
}

void Interpolator::idle()
{
   if (steps > 0)
   {
      GLfloat t = GLfloat(steps) / GLfloat(maxSteps);
      current = getQuaternion(t);
      --steps;
   }
   else
   {
      current = last;
      steps = 0;
   }
}


//added by nick, this is just a wrapper of original apply(t)
//because I want to setup a calling "template" for all class
//which inherits from BaseCamera who owns "idle" function.
void Interpolator::apply() const
{
   double t;
   t = 1 - (GLfloat)steps/(GLfloat)maxSteps;
   apply(t);
}

void Interpolator::apply(double t) const
{
   GL_Matrix m;
   getMatrix(t, m);
   glMultMatrixf(&m[0][0]);
}

Quaternion Interpolator::getQuaternion(double t) const
// Returns the interpolated quaternion
// corresponding to t, where 0 <= t <= 1.
{
   Quaternion q;
   if (sinOmega == 0)
      q = first;
   else
      q = first * GLfloat((sin(omega * (1 - t)) / sinOmega)) +
          last * GLfloat((sin(omega * t) / sinOmega));
   q.normalize();
   return q;
}

void Interpolator::initialize()
// Spherical interpolation using angle "omega".
// Ensure cos(Omega) is positive to get shortest path.
{
   first.normalize();
   last.normalize();
   cosOmega = dot(first, last);
   if (cosOmega < 0)
      cosOmega = - cosOmega;
   omega = acos(cosOmega);
   sinOmega = sin(omega);
   if (sinOmega == 0)
      cuglError = BAD_INTERPOLATOR_ARG;
}



// Constructor and destructor for class Revolute.

Revolute::Revolute(int numSteps, GLfloat profile[][2]) :
      numSteps(numSteps),
      numSlices(45),
      eccentricity(0),
      ready(false)
{
   coor = new GLfloatArray[numSteps];
   for (int n = 0; n < numSteps; n++)
   {
      coor[n] = new GLfloat[2];
      for (int c = 0; c < 2; c++)
         coor[n][c] = profile[n][c];
   }
}

Revolute::~Revolute()
{
   for (int n = 0; n < numSteps; n++)
      delete [] coor[n];
   delete [] coor;
   delete [] texCoor;
   delete [] points;
   delete [] normals;
   delete [] faceNormals;
}

// Member functions for class Revolute.

void Revolute::setSlices(int slices)
{
   if (slices % 2 == 0)
      slices++;
   if (slices < 3)
      slices = 3;
   numSlices = slices;
   ready = false;
}

void Revolute::setEccentricity(double ecc)
{
   if (ecc < 0 || ecc >= 1)
      ecc = 0;
   eccentricity = ecc;
   ready = false;
}

void Revolute::process()
{
   // Allocate space for working arrays.
   texCoor = new GLfloat[numSteps];
   points = new Point[numSteps * numSlices];
   normals = new Vector[numSteps * numSlices];
   faceNormals = new Vector[numSteps * numSlices];

   // Indexes
   int dCurr, dNext, pCurr, pNext, sCurr, sNext;

   // Compute texture coordinates along axis and then normalize them.
   GLfloat texmax = 0;
   texCoor[0] = 0;
   for (pCurr = 1; pCurr < numSteps; pCurr++)
   {
      GLfloat dist = GLfloat(sqrt(sqr(coor[pCurr][0] - coor[pCurr-1][0]) + sqr(coor[pCurr][1] - coor[pCurr-1][1])));
      texCoor[pCurr] = texCoor[pCurr-1] + dist;
      texmax += dist;
   }
   for (pCurr = 1; pCurr < numSteps; pCurr++)
      texCoor[pCurr] = texCoor[pCurr] / texmax;

   // Compute 3D points on the surface of the solid.
   for (sCurr = 0; sCurr < numSlices; sCurr++)
   {
      double theta = (2 * PI * sCurr) / numSlices;
      GLfloat sint = GLfloat(sin(theta));
      GLfloat cost = GLfloat(cos(theta));
      if (eccentricity != 0)
      {
         double e = 1 - sqr(eccentricity);
         sint *= GLfloat(e);
         cost /= GLfloat(sqrt(e));
      }
      for (pCurr = 0; pCurr < numSteps; pCurr++)
      {
         points[numSteps * sCurr + pCurr] =
            Point(
               coor[pCurr][0] * cost,
               coor[pCurr][0] * sint,
               coor[pCurr][1] );
      }
   }

   // Find the normal for each quadrilateral face
   // of the solid, using Newell's method.
   for (pCurr = 0; pCurr < numSteps - 1; pCurr++)
   {
      pNext = pCurr + 1;
      for (sCurr = 0; sCurr < numSlices; sCurr++)
      {
         sNext = sCurr == numSlices - 1 ? 0 : sCurr + 1;

         // Get the vertexes.
         Point quad[4];
         quad[0] = points[numSteps * sCurr + pCurr];
         quad[1] = points[numSteps * sNext + pCurr];
         quad[2] = points[numSteps * sNext + pNext];
         quad[3] = points[numSteps * sCurr + pNext];

         // Find the normal and store it.
         GLfloat xnorm = 0;
         GLfloat ynorm = 0;
         GLfloat znorm = 0;
         for (dCurr = 0; dCurr < 3; dCurr++)
         {
            dNext = dCurr == 2 ? 0 : dCurr + 1;
            xnorm += (quad[dCurr][1] - quad[dNext][1]) * (quad[dCurr][2] + quad[dNext][2]);
            ynorm += (quad[dCurr][2] - quad[dNext][2]) * (quad[dCurr][0] + quad[dNext][0]);
            znorm += (quad[dCurr][0] - quad[dNext][0]) * (quad[dCurr][1] + quad[dNext][1]);
         }
         faceNormals[numSteps * sCurr + pCurr] = Vector(xnorm, ynorm, znorm);
      }
   }

   // Find normals at each vertex by averaging face normals.
   for (pCurr = 0; pCurr < numSteps; pCurr++)
   {
      pNext = pCurr == numSteps - 1 ? 0 : pCurr + 1;
      for (sCurr = 0; sCurr < numSlices; sCurr++)
      {
         sNext = sCurr == 0 ? numSlices - 1 : sCurr - 1;
         Vector norm;
         if (pCurr == 0)
            norm =
               faceNormals[numSteps * sCurr + pCurr] +
               faceNormals[numSteps * sNext + pCurr];
         else if (pCurr == numSteps - 1)
            norm =
               faceNormals[numSteps * sCurr + pCurr - 1] +
               faceNormals[numSteps * sNext + pCurr - 1];
         else
            norm =
               faceNormals[numSteps * sCurr + pCurr - 1] +
               faceNormals[numSteps * sNext + pCurr - 1] +
               faceNormals[numSteps * sCurr + pCurr] +
               faceNormals[numSteps * sNext + pCurr];
         normals[numSteps * sCurr + pCurr] = norm.unit();
      }
   }
   ready = true;
}

void Revolute::draw(bool showNormals)
{
   if (!ready)
      return;

   int pCurr, pNext, sCurr, sNext;
   GLfloat slices = GLfloat(numSlices);
   glBegin(GL_QUAD_STRIP);
   for (pCurr = 0; pCurr < numSteps - 1; pCurr++)
   {
      pNext = pCurr + 1;
      for (sCurr = 0; sCurr < numSlices; sCurr += 2)
      {
         sNext = sCurr == numSlices - 1 ? 0 : sCurr + 1;

         glTexCoord2f(texCoor[pCurr], sCurr / slices);
         normals[numSteps * sCurr + pCurr].drawNormal();
         points[numSteps * sCurr + pCurr].draw();

         glTexCoord2f(texCoor[pNext], sCurr / slices);
         normals[numSteps * sCurr + pNext].drawNormal();
         points[numSteps * sCurr + pNext].draw();

         glTexCoord2f(texCoor[pCurr], sNext / slices);
         normals[numSteps * sNext + pCurr].drawNormal();
         points[numSteps * sNext + pCurr].draw();

         glTexCoord2f(texCoor[pNext], sNext / slices);
         normals[numSteps * sNext + pNext].drawNormal();
         points[numSteps * sNext + pNext].draw();
      }
   }
   glEnd();
   if (showNormals)
   {
      GLboolean lighting;
      glGetBooleanv(GL_LIGHTING, &lighting);
      if (lighting)
         glDisable(GL_LIGHTING);
      glColor3f(1, 1, 1);
      glBegin(GL_LINES);
      for (pCurr = 0; pCurr < numSteps; pCurr++)
         for (sCurr = 0; sCurr < numSlices; sCurr++)
         {
            int i = numSteps * sCurr + pCurr;
            points[i].draw();
            (points[i] + normals[i]).draw();
         }
      glEnd();
      if (lighting)
         glEnable(GL_LIGHTING);
   }
}

// Construct a solid of revolution

void revolve(int numSteps, GLfloat coor[][2], int numSlices, bool drawNormals)
{
   // The number of slices must be odd.
   if (numSlices % 2 == 0)
      numSlices++;

   // Allocate space for working arrays.
   Point *points = new Point[numSteps * numSlices];
   Vector *normals = new Vector[numSteps * numSlices];
   Vector *faceNormals = new Vector[numSteps * numSlices];

   // Indexes
   int dCurr, dNext, pCurr, pNext, sCurr, sNext;

   // Compute 3D points on the surface of the solid.
   for (sCurr = 0; sCurr < numSlices; sCurr++)
   {
      double theta = (2 * PI * sCurr) / numSlices;
      GLfloat sint = GLfloat(sin(theta));
      GLfloat cost = GLfloat(cos(theta));
      for (pCurr = 0; pCurr < numSteps; pCurr++)
      {
         points[numSteps * sCurr + pCurr] =
            Point(
               coor[pCurr][0] * cost,
               coor[pCurr][0] * sint,
               coor[pCurr][1] );
      }
   }

   // Find the normal for each quadrilateral face
   // of the solid, using Newell's method.
   for (pCurr = 0; pCurr < numSteps - 1; pCurr++)
   {
      pNext = pCurr + 1;
      for (sCurr = 0; sCurr < numSlices; sCurr++)
      {
         sNext = sCurr == numSlices - 1 ? 0 : sCurr + 1;

         // Get the vertexes.
         Point quad[4];
         quad[0] = points[numSteps * sCurr + pCurr];
         quad[1] = points[numSteps * sNext + pCurr];
         quad[2] = points[numSteps * sNext + pNext];
         quad[3] = points[numSteps * sCurr + pNext];

         // Find the normal and store it.
         GLfloat xnorm = 0;
         GLfloat ynorm = 0;
         GLfloat znorm = 0;
         for (dCurr = 0; dCurr < 3; dCurr++)
         {
            dNext = dCurr == 2 ? 0 : dCurr + 1;
            xnorm += (quad[dCurr][1] - quad[dNext][1]) * (quad[dCurr][2] + quad[dNext][2]);
            ynorm += (quad[dCurr][2] - quad[dNext][2]) * (quad[dCurr][0] + quad[dNext][0]);
            znorm += (quad[dCurr][0] - quad[dNext][0]) * (quad[dCurr][1] + quad[dNext][1]);
         }
         faceNormals[numSteps * sCurr + pCurr] = Vector(xnorm, ynorm, znorm);
      }
   }

   // Find normals at each vertex by averaging face normals.
   for (pCurr = 0; pCurr < numSteps; pCurr++)
   {
      pNext = pCurr == numSteps - 1 ? 0 : pCurr + 1;
      for (sCurr = 0; sCurr < numSlices; sCurr++)
      {
         sNext = sCurr == 0 ? numSlices - 1 : sCurr - 1;
         Vector norm;
         if (pCurr == 0)
            norm =
               faceNormals[numSteps * sCurr + pCurr] +
               faceNormals[numSteps * sNext + pCurr];
         else if (pCurr == numSteps - 1)
            norm =
               faceNormals[numSteps * sCurr + pCurr - 1] +
               faceNormals[numSteps * sNext + pCurr - 1];
         else
            norm =
               faceNormals[numSteps * sCurr + pCurr - 1] +
               faceNormals[numSteps * sNext + pCurr - 1] +
               faceNormals[numSteps * sCurr + pCurr] +
               faceNormals[numSteps * sNext + pCurr];
         normals[numSteps * sCurr + pCurr] = norm.unit();

         // Draw normals if requested.
         if (drawNormals)
         {
            glBegin(GL_LINES);
            points[numSteps * sCurr + pCurr].draw();
            (points[numSteps * sCurr + pCurr] + norm).draw();
            glEnd();
         }
      }
   }

   // Tell OpenGL about each vertex and its normal.
   glBegin(GL_QUAD_STRIP);
   for (pCurr = 0; pCurr < numSteps - 1; pCurr++)
   {
      pNext = pCurr + 1;
      for (sCurr = 0; sCurr < numSlices; sCurr += 2)
      {
         sNext = sCurr == numSlices - 1 ? 0 : sCurr + 1;
         normals[numSteps * sCurr + pCurr].drawNormal();
         points[numSteps * sCurr + pCurr].draw();
         normals[numSteps * sCurr + pNext].drawNormal();
         points[numSteps * sCurr + pNext].draw();
         normals[numSteps * sNext + pCurr].drawNormal();
         points[numSteps * sNext + pCurr].draw();
         normals[numSteps * sNext + pNext].drawNormal();
         points[numSteps * sNext + pNext].draw();
      }
   }
   glEnd();

   delete [] points;
   delete [] normals;
   delete [] faceNormals;
}

// Utility functions for reading a BMP file.

// Read low-order byte then high-order byte
unsigned short getShort(ifstream & is)
{
   unsigned char c1, c2;
   //this is really ridiculous, in Linux, it doesn't overload the char and unsign char for
   //function get(unsigned char& ch)
   c1=is.get();
   c2=is.get();
   //is.get(c1);
   //is.get(c2);
   return ( (c2 << 8) | c1 );
}

// Read bytes from low-order to high-order
unsigned long getULong(ifstream & is)
{
   unsigned char c1, c2, c3, c4;
   c1=is.get();
   c2=is.get();
   c3=is.get();
   c4=is.get();

   //is.get(c1);
   //is.get(c2);
   //is.get(c3);
   //is.get(c4);
   return ( (c4 << 24) | (c3 << 16) | (c2 << 8) | c1);
}

// Read bytes from low-order to high-order
long getLong(ifstream & is)
{
   unsigned char c1, c2, c3, c4;
   c1=is.get();
   c2=is.get();
   c3=is.get();
   c4=is.get();

   //is.get(c1);
   //is.get(c2);
   //is.get(c3);
   //is.get(c4);
   return (long(c4) << 24) | (long(c3) << 16) | (long(c2) << 8) | long(c1);
}

// Write low-order byte then high-order byte
void putShort(ofstream & os, unsigned short us)
{
   os.put((unsigned char)(us & 0xFF));
   os.put((unsigned char)(us >> 8));
};

// Write bytes from low-order to high-order
void putULong(ofstream & os, unsigned long ul)
{
   os.put((unsigned char)( ul        & 0xFF));
   os.put((unsigned char)((ul >>  8) & 0xFF));
   os.put((unsigned char)((ul >> 16) & 0xFF));
   os.put((unsigned char)((ul >> 24) & 0xFF));
};

// Write bytes from low-order to high-order
void putLong(ofstream & os, long n)
{
   os.put((unsigned char)( n        & 0xFF));
   os.put((unsigned char)((n >>  8) & 0xFF));
   os.put((unsigned char)((n >> 16) & 0xFF));
   os.put((unsigned char)((n >> 24) & 0xFF));
};

bool isPowerOfTwo(unsigned long ul)
{
   int c = 0;
   while (ul)
   {
      c += ul & 1;
      ul >>= 1;
   }
   return c == 1;
}

unsigned long highBit(unsigned long ul)
{
   unsigned long m = 1 << 31;
   while ((m & ul) == 0)
      m >>= 1;
   return m << 1;
}

// Constructors and destructor for class PixelMap

PixelMap::PixelMap() :
      numRows(0),
      numCols(0),
      size(0),
      fileName(0),
      pixels(0)
{}

PixelMap::PixelMap(const char *bmpFileName) :
      numRows(0),
      numCols(0),
      size(0),
      fileName(0),
      pixels(0)
{
   read(bmpFileName);
}

PixelMap::PixelMap(GLint x, GLint y, GLsizei w, GLsizei h) :
      numRows(h - h % 4),
      numCols(w - w % 4),
      size(0),
      fileName(0),
      pixels(0)
{
   if (allocate(3 * numCols * numRows))
      glReadPixels(x, y, numCols, numRows, GL_RGB, GL_UNSIGNED_BYTE, pixels);
}

PixelMap::~PixelMap()
{
   delete fileName;
   delete pixels;
}

// Member functions of class PixelMap

bool PixelMap::allocate(unsigned long newSize)
{
   // Attempt to get the memory we need, return if failure.
   unsigned char *mem = new unsigned char[newSize];
   if (mem == 0)
   {
      cuglError = NOT_ENOUGH_MEMORY;
      return false;
   }

   // Update data members.
   delete [] pixels;
   pixels = mem;
   size = newSize;
   return true;
}

void PixelMap::read(const char *bmpFileName)
{
   //ifstream inf(bmpFileName, ios::in | ios::binary | ios::nocreate);
   //in linux, there is no mode of "nocreate"
   ifstream inf(bmpFileName, ios::in | ios::binary );
   if (!inf)
   {
      cuglError = OPEN_FAILED;
      return;
   }

   char b, m;
   inf.get(b);
   inf.get(m);
   if (b != 'B' || m != 'M')
   {
      cuglError = NOT_BMP_FILE;
      inf.close();
      return;
   }

   // Read the header field of the .bmp file.
   // Fields that we do not need are commented out to prevent compiler warnings.
   /* unsigned long  fileSize   = */ getULong(inf);
   /* unsigned short res1       = */ getShort(inf);      // should be 0
   /* unsigned short res2       = */ getShort(inf);      // should be 0
   /* unsigned long  offset     = */ getULong(inf);      // unreliable offset to image
   /* unsigned long  headerSize = */ getULong(inf);      // should be 40
   long           cols          =    getLong(inf);       // # columns
   long           rows          =    getLong(inf);       // # rows
   /* unsigned short planes     = */ getShort(inf);      // usually 1
   unsigned short bpx           =    getShort(inf);      // bits per pixel
   unsigned long  compressed    =    getULong(inf);      // 0 for uncompressed
   /* unsigned long  imageSize  = */ getULong(inf);      // bytes in image (unreliable - may be zero)
   /* long           xPels      = */ getLong(inf);       // may be 0
   /* long           yPels      = */ getLong(inf);       // may be 0
   /* unsigned long  entries    = */ getULong(inf);      // should be 0 or 256
   /* unsigned long  impColours = */ getULong(inf);      // should be 0

   if (bpx != 24)
   {
      cuglError = NOT_24_BITS;
      inf.close();
      return;
   }

   if (compressed != 0)
   {
      cuglError = COMPRESSED_BMP_FILE;
      inf.close();
      return;
   }

   // Deal with the variant of BMP file: if height is negative
   if (rows <= 0)
      rows = -rows;

   // Allocate memory
   if ( ! allocate(3 * rows * cols))
   {
      cuglError = NOT_ENOUGH_MEMORY;
      return;
   }

   delete [] fileName;
   fileName = new char[strlen(bmpFileName) + 1];
   strcpy(fileName, bmpFileName);
   numRows = rows;
   numCols = cols;

   // Transfer data from BMP file to memory.
   long count = 0;
   for (long row = 0; row < rows; row++)
   {
      for (long col = 0; col < cols; col++)
      {
         unsigned char red, green, blue;

         blue = inf.get();
         green = inf.get();
         red = inf.get();
         pixels[count++] = red;
         pixels[count++] = green;
         pixels[count++] = blue;
      }
   }
   inf.close();
}

//void PixelMap::read(GLint x, GLint y, GLsizei w, GLsizei h)
//{
//   numCols = w - w % 4;
//   numRows = h - h % 4;
//   delete [] fileName;
//   if (allocate(3 * numCols * numRows))
//      glReadPixels(x, y, numCols, numRows, GL_RGB, GL_UNSIGNED_BYTE, pixels);
//}

void PixelMap::read(GLint x, GLint y, GLsizei w, GLsizei h, GLenum mode)
{
   numCols = w - w % 4;
   numRows = h - h % 4;
   delete [] fileName;
   if (allocate(3 * numCols * numRows))
   {
      glReadBuffer(mode);
      glReadPixels(x, y, numCols, numRows, GL_RGB, GL_UNSIGNED_BYTE, pixels);
   }
}


void PixelMap::write(const char *bmpFileName)
{
   const unsigned long HEADER_SIZE = 40;
   const unsigned long FILE_HEADER_SIZE = 14;
   const unsigned short PLANES = 1;
   const unsigned short BITS_PER_PIXEL = 24;
   const unsigned long UNCOMPRESSED = 0;
   const long XPELS = 0;
   const long YPELS = 0;
   const unsigned long ENTRIES = 0;
   const unsigned long COLOURS = 0;

   ofstream os(bmpFileName, ios::binary);
   if (!os)
   {
      cuglError = OPEN_FAILED;
      return;
   }

   unsigned long imageSize = numRows * (3 * numCols);
   unsigned long fileSize = imageSize + HEADER_SIZE + FILE_HEADER_SIZE;

   // Write file header
   os.put('B');
   os.put('M');
   putULong(os, fileSize);
   putShort(os, 0);
   putShort(os, 0);
   putULong(os, HEADER_SIZE + FILE_HEADER_SIZE);

   // Write header
   putULong(os, HEADER_SIZE);
   putLong (os, numCols);
   putLong (os, numRows);
   putShort(os, PLANES);
   putShort(os, BITS_PER_PIXEL);
   putULong(os, UNCOMPRESSED);
   putULong(os, imageSize);
   putLong (os, XPELS);
   putLong (os, YPELS);
   putULong(os, ENTRIES);
   putULong(os, COLOURS);

   // Write data
   long count = 0;
   for (unsigned long row = 0; row < numRows; row++)
   {
      for (unsigned long col = 0; col < numCols; col++)
      {
         unsigned char red   = pixels[count++];
         unsigned char green = pixels[count++];
         unsigned char blue  = pixels[count++];
         os.put(blue);
         os.put(green);
         os.put(red);
      }
   }
   os.close();
}

bool compatible(const PixelMap & m1, const PixelMap & m2)
{
   return (m1.getRows() == m2.getRows()) && (m1.getColumns() == m2.getColumns());
}

void mix(const PixelMap & m1, const PixelMap & m2, PixelMap & res, double prop)
{
   double p1 = 1 - prop;
   double p2 = prop;
   long count = 0;
   for (unsigned long row = 0; row < m1.numRows; row++)
   {
      for (unsigned long col = 0; col < m1.numCols; col++)
      {
         // red
         res.pixels[count] = (unsigned char)(p1 * m1.pixels[count]
                                             + p2 * m2.pixels[count]);
         count++;

         // green
         res.pixels[count] = (unsigned char)(p1 * m1.pixels[count]
                                             + p2 * m2.pixels[count]);
         count++;

         // blue
         res.pixels[count] = (unsigned char)(p1 * m1.pixels[count]
                                             + p2 * m2.pixels[count]);
         count++;
      }
   }
}

void PixelMap::select(const PixelMap & src, int xp, int yp, int width, int height)
{
   numCols = width - width % 4;
   numRows = height - height % 4;
   size = 3 * numCols * numRows;
   delete [] pixels;
   pixels = new unsigned char[size];

   long sn = 0;
   long tn = 0;
   for (unsigned int r = 0; r < src.getRows(); ++r)
   {
      unsigned int tr = r - yp;
      for (unsigned int c = 0; c < src.getColumns(); ++c)
      {
         unsigned char r = src.pixels[sn++];
         unsigned char g = src.pixels[sn++];
         unsigned char b = src.pixels[sn++];

         unsigned int tc = c - xp;
         if (0 <= tr && tr < numRows && 0 <= tc && tc < numCols)
         {
            pixels[tn++] = r;
            pixels[tn++] = g;
            pixels[tn++] = b;
         }
      }
   }
}

void PixelMap::draw()
{
   if (pixels == 0)
   {
      cuglError = NO_PIX_MAP;
      return;
   }
   glDrawPixels(GLsizei(numCols), GLsizei(numRows), GL_RGB, GL_UNSIGNED_BYTE, pixels);
}

void PixelMap::setTexture(GLuint name)
{
   if (pixels == 0)
   {
      cuglError = NO_PIX_MAP;
      return;
   }
   glBindTexture(GL_TEXTURE_2D, name);
   glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
   glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
   glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, numCols, numRows, 0, GL_RGB, GL_UNSIGNED_BYTE, pixels);
}

void PixelMap::setMipmaps(GLuint name)
{
   if (pixels == 0)
   {
      cuglError = NO_PIX_MAP;
      return;
   }
   glBindTexture(GL_TEXTURE_2D, name);
   glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_NEAREST );
   gluBuild2DMipmaps(GL_TEXTURE_2D, GL_RGB, numCols, numRows, GL_RGB, GL_UNSIGNED_BYTE, pixels);
}

bool PixelMap::badSize()
{
   return ( ! isPowerOfTwo(numCols) || ! isPowerOfTwo(numRows) );
}

void PixelMap::rescale()
{
   unsigned long cols = isPowerOfTwo(numCols) ? numCols : highBit(numCols);
   unsigned long rows = isPowerOfTwo(numRows) ? numRows : highBit(numRows);
   unsigned char *mem = new unsigned char[3 * rows * cols];
   if (mem == 0)
   {
      cuglError = NOT_ENOUGH_MEMORY;
      return;
   }
   gluScaleImage(GL_RGB, numCols, numRows, GL_UNSIGNED_BYTE, pixels,
                 cols, rows, GL_UNSIGNED_BYTE, mem);
   delete [] pixels;
   pixels = mem;
   numCols = cols;
   numRows = rows;
   size = 3 * rows * cols;
}

ostream & operator<<(ostream & os, const PixelMap & pm)
{
   if (pm.fileName)
      os << pm.fileName;
   os << " (" <<
   pm.numRows << " rows, " <<
   pm.numCols << " columns, " <<
   pm.size << " bytes)";
   return os;
}

// Implementation of normals for triangle strips
void triStripNormals(Point points[], Vector normals[], int numPoints, bool neg)
{
   const int MAXPOINTS = 100;
   Vector sides[MAXPOINTS];
   Vector triangleNormals[MAXPOINTS];
   int i;

   if (numPoints > MAXPOINTS)
   {
      cuglError = TOO_MANY_POINTS;
      return;
   }

   for (i = 0; i < numPoints - 1; i++)
      sides[i] = points[i + 1] - points[i];

   bool even = true;
   for (i = 0; i < numPoints - 2; i++)
   {
      if (even)
         triangleNormals[i] = cross(sides[i], sides[i + 1]);
      else
         triangleNormals[i] = cross(sides[i + 1], sides[i]);
      even = ! even;
   }

   for (i = 0; i < numPoints; i++)
   {
      if (i - 2 >= 0)
         normals[i] += triangleNormals[i-2];
      if (i - 1 >= 0 && i < numPoints - 1)
         normals[i] += triangleNormals[i-1];
      if (i < numPoints - 2)
         normals[i] += triangleNormals[i];
      normals[i].normalize();
      if (neg)
         normals[i] = -normals[i];
   }
}

// Implementation of model functions

void axes(GLfloat size)
{
   const int STACKS = 12;
   GLfloat POINTS[STACKS][2] =
   {
      {
         0,     -0.1f
      },
      {
         0.04f,  0
      },
      {
         0.04f,  0.2f
      },
      {
         0.04f,  0.4f
      },
      {
         0.044f, 0.6f
      },
      {
         0.056f, 0.75f
      },
      {
         0.1f,   0.75f
      },
      {
         0.064f, 0.8f
      },
      {
         0.036f, 0.85f
      },
      {
         0.016f, 0.9f
      },
      {
         0.004f, 0.95f
      },
      {
         0,       1
      }
   };

   GLfloat pts[STACKS][2];
   for (int x = 0; x < STACKS; x++)
      for (int y = 0; y < 2; y++)
         pts[x][y] = size * POINTS[x][y];

   // Store one axis in a call list.
   GLuint list = glGenLists(1);
   glNewList(list, GL_COMPILE);
   revolve(STACKS, pts, STACKS);
   glEndList();

   // X axis
   setMaterial(RED);
   glPushMatrix();
   glRotatef(90, 0, 1, 0);
   glCallList(list);
   glPopMatrix();

   // Y axis
   setMaterial(GREEN);
   glPushMatrix();
   glRotatef(-90, 1, 0, 0);
   glCallList(list);
   glPopMatrix();

   // Z axis
   setMaterial(BLUE);
   glPushMatrix();
   glCallList(list);
   glPopMatrix();

   glDeleteLists(list, 1);
}


GLuint makePlaneList(bool shadow)
{
   GLuint index = glGenLists(1);
   glNewList(index, GL_COMPILE);
   buildPlane(shadow);
   glEndList();
   return index;
}

void buildPlane(bool shadow)
{
   int p;
   // Set up for Bezier surface generation.
   glEnable(GL_MAP2_VERTEX_3);
   glEnable(GL_AUTO_NORMAL);
   if (shadow)
      setMaterial(BLACK);
   else
      setMaterial(METAL);

   // Fuselage
   const int fuWidth = 4;
   const int fuLength = 6;
   const int fuLoops = 20;
   const int fuSlices = 20;
   const GLfloat fuShapeFactor = 0.9f;
   GLfloat fuPoints[fuLength][fuWidth][3];

   struct
   {
      GLfloat len;
      GLfloat size;
   }
   fuParameters[fuLength] =
   {
      {
         -10, 0
      },
      {
         -9.6f, 1.4f
      },
      {
         -9, 1.6f
      },
      {
         8, 1.4f
      },
      {
         9.9f, 1
      },
      {
         10, 0
      }
   };

   for (p = 0; p < fuLength; p++)
   {
      for (int y = 0; y < fuWidth; y++)
         fuPoints[p][y][2] = fuParameters[p].len;
      fuPoints[p][0][0] = 0;
      fuPoints[p][1][0] = fuParameters[p].size;
      fuPoints[p][2][0] = fuParameters[p].size;
      fuPoints[p][3][0] = 0;

      fuPoints[p][0][1] = - fuShapeFactor * fuParameters[p].size;
      fuPoints[p][1][1] = - fuShapeFactor * fuParameters[p].size;
      fuPoints[p][2][1] = fuShapeFactor * fuParameters[p].size;
      fuPoints[p][3][1] = fuShapeFactor * fuParameters[p].size;
   }

   glMap2f(GL_MAP2_VERTEX_3,
           0, 1, 3,           fuWidth,
           0, 1, 3 * fuWidth, fuLength,
           &fuPoints[0][0][0]);
   glMapGrid2f(fuLoops, 0, 1, fuSlices, 0, 1);

   glEvalMesh2(GL_FILL, 0, fuLoops, 0, fuSlices);
   glScalef(-1, 1, 1);
   glEvalMesh2(GL_FILL, 0, fuLoops, 0, fuSlices);

   // Wings
   const int wiLength = 6;
   const GLfloat wiAspectRatio = 0.2f;
   const GLfloat wiHeight = -0.6f;
   const GLfloat wiDihedral = 0.1f;
   GLfloat xVals[wiLength] =
   {
      0.5f, 5, 8, 11, 14, 15
   };
   GLfloat wiPoints[wiLength][4][3];

   for (p = 0; p < wiLength; p++)
   {
      GLfloat x = xVals[p];
      GLfloat y = wiHeight + x * wiDihedral;
      GLfloat zTrailingEdge = (x - 1) / 7 + 2;
      GLfloat zGuide = 8 * (x - 1) / 14 - 7;
      GLfloat yGuide = wiAspectRatio * (zGuide - zTrailingEdge);

      wiPoints[p][0][0] = x;
      wiPoints[p][0][1] = y;
      wiPoints[p][0][2] = zTrailingEdge;

      wiPoints[p][1][0] = x;
      wiPoints[p][1][1] = p == 5 ? y : y + yGuide;
      wiPoints[p][1][2] = zGuide;

      wiPoints[p][2][0] = x;
      wiPoints[p][2][1] = p == 5 ? y: y - yGuide;
      wiPoints[p][2][2] = zGuide;

      wiPoints[p][3][0] = x;
      wiPoints[p][3][1] = y;
      wiPoints[p][3][2] = zTrailingEdge;
   }

   glMap2f(GL_MAP2_VERTEX_3,
           0, 1,  3, 4,
           0, 1, 12, wiLength,
           &wiPoints[0][0][0]);
   glMapGrid2f(fuLoops, 0, 1, fuSlices, 0, 1);

   glEvalMesh2(GL_FILL, 0, fuLoops, 0, fuSlices);
   glScalef(-1, 1, 1);
   glEvalMesh2(GL_FILL, 0, fuLoops, 0, fuSlices);

   // Tailplane
   const int taWidth = 4;
   const int taLength = 6;
   const GLfloat taAspectRatio = 0.3f;
   const GLfloat taHeight = 0.1f;
   const GLfloat taDihedral = 0.3f;
   GLfloat xTa[taLength] =
   {
      0.3f, 1.7f, 2.6f, 3.3f, 4.6f, 5
   };
   GLfloat taPoints[taLength][taWidth][3];

   for (p = 0; p < taLength; p++)
   {
      GLfloat x = xTa[p];
      GLfloat y = taHeight + x * taDihedral;
      GLfloat zTrailingEdge = (x - 1) / 4 + 9.5f;
      GLfloat zGuide = (x - 1) / 2 + 7.5f;
      GLfloat yGuide = taAspectRatio * (zGuide - zTrailingEdge);

      taPoints[p][0][0] = x;
      taPoints[p][0][1] = y;
      taPoints[p][0][2] = zTrailingEdge;

      taPoints[p][1][0] = x;
      taPoints[p][1][1] = p == 5 ? y : y + yGuide;
      taPoints[p][1][2] = zGuide;

      taPoints[p][2][0] = x;
      taPoints[p][2][1] = p == 5 ? y: y - yGuide;
      taPoints[p][2][2] = zGuide;

      taPoints[p][3][0] = x;
      taPoints[p][3][1] = y;
      taPoints[p][3][2] = zTrailingEdge;
   }

   glMap2f(GL_MAP2_VERTEX_3,
           0, 1, 3,           taWidth,
           0, 1, 3 * taWidth, taLength,
           &taPoints[0][0][0]);
   glMapGrid2f(fuLoops, 0, 1, fuSlices, 0, 1);

   glEvalMesh2(GL_FILL, 0, fuLoops, 0, fuSlices);
   glScalef(-1, 1, 1);
   glEvalMesh2(GL_FILL, 0, fuLoops, 0, fuSlices);

   // Rudder
   const int ruWidth = 4;
   const int ruLength = 6;
   const GLfloat ruAspectRatio = 0.2f;
   GLfloat ruPoints[ruLength][ruWidth][3];

   for (p = 0; p < ruLength; p++)
   {
      GLfloat y = 0.7f * p;
      GLfloat zTrailingEdge = y / 3 + 9.5f;
      GLfloat zGuide = y + 6;
      GLfloat xGuide = ruAspectRatio * (zGuide - zTrailingEdge);

      ruPoints[p][0][0] = 0;
      ruPoints[p][0][1] = y;
      ruPoints[p][0][2] = zTrailingEdge;

      ruPoints[p][1][0] = p == 5 ? 0 : - xGuide;
      ruPoints[p][1][1] = y;
      ruPoints[p][1][2] = zGuide;

      ruPoints[p][2][0] = p == 5 ? 0 : xGuide;
      ruPoints[p][2][1] = y;
      ruPoints[p][2][2] = zGuide;

      ruPoints[p][3][0] = 0;
      ruPoints[p][3][1] = y;
      ruPoints[p][3][2] = zTrailingEdge;
   }

   glMap2f(GL_MAP2_VERTEX_3,
           0, 1, 3,           ruWidth,
           0, 1, 3 * ruWidth, ruLength,
           &ruPoints[0][0][0]);
   glMapGrid2f(fuLoops, 0, 1, fuSlices, 0, 1);

   glEvalMesh2(GL_FILL, 0, fuLoops, 0, fuSlices);

   const GLfloat xMotor = 3;
   const GLfloat yMotor = -1.8f;
   const GLfloat zMotor = -3;

   // Motor support
   const int suWidth = 4;
   const int suLength = 4;
   const GLfloat suAspectRatio = 0.2f;
   GLfloat suPoints[ruLength][ruWidth][3];

   for (p = 0; p < suLength; p++)
   {
      GLfloat y = 0.2f * p + 0.7f;
      GLfloat zTrailingEdge = y / 2 + 2.5f;
      GLfloat zGuide = y / 2 + 0.5f;
      GLfloat xGuide = suAspectRatio * (zGuide - zTrailingEdge);

      suPoints[p][0][0] = 0;
      suPoints[p][0][1] = y;
      suPoints[p][0][2] = zTrailingEdge;

      suPoints[p][1][0] = p == 3 ? 0 : - xGuide;
      suPoints[p][1][1] = y;
      suPoints[p][1][2] = zGuide;

      suPoints[p][2][0] = p == 3 ? 0 : xGuide;
      suPoints[p][2][1] = y;
      suPoints[p][2][2] = zGuide;

      suPoints[p][3][0] = 0;
      suPoints[p][3][1] = y;
      suPoints[p][3][2] = zTrailingEdge;
   }

   glMap2f(GL_MAP2_VERTEX_3,
           0, 1, 3, suWidth,
           0, 1, 3 * suWidth, suLength,
           &suPoints[0][0][0]);
   glMapGrid2f(fuLoops, 0, 1, fuSlices, 0, 1);

   glPushMatrix();
   glTranslatef(xMotor, yMotor, zMotor);
   glEvalMesh2(GL_FILL, 0, fuLoops, 0, fuSlices);
   glPopMatrix();

   glPushMatrix();
   glTranslatef(- xMotor, yMotor, zMotor);
   glEvalMesh2(GL_FILL, 0, fuLoops, 0, fuSlices);
   glPopMatrix();

   // Motor
   const double PI = 4 * atan(1.0);
   const int moLength = 5;
   const int moRound = 8;
   const GLfloat zVal[moLength] =
   {
      1, 0, 1, 4, 3
   };
   const GLfloat rad[moLength] =
   {
      0.5, 0.5, 1.5, 0.5, 0.5
   };
   GLfloat moPoints[moLength][moRound][3];

   for (int a = 0; a < moRound; a++)
   {
      const double theta = a * 2 * PI / (moRound - 1);
      const GLfloat sint = GLfloat(sin(theta));
      const GLfloat cost = GLfloat(cos(theta));
      for (int z = 0; z < moLength; z++)
      {
         moPoints[z][a][0] = - rad[z] * sint;
         moPoints[z][a][1] = rad[z] * cost;
         moPoints[z][a][2] = zVal[z];
      }
   }

   glMap2f(GL_MAP2_VERTEX_3,
           0, 1, 3, moRound,
           0, 1, 3 * moRound, moLength,
           &moPoints[0][0][0]);
   glMapGrid2f(20, 0, 1, 20, 0, 1);

   glPushMatrix();
   glTranslatef(xMotor, yMotor, zMotor);
   glEvalMesh2(GL_FILL, 0, 20, 0, 20);
   glPopMatrix();

   glPushMatrix();
   glTranslatef(- xMotor, yMotor, zMotor);
   glEvalMesh2(GL_FILL, 0, 20, 0, 20);
   glPopMatrix();

   GLUquadricObj *quad = gluNewQuadric();
   gluQuadricDrawStyle(quad, GLU_FILL);
   gluQuadricNormals(quad, GLU_SMOOTH);
   setMaterial(BLACK);

   glPushMatrix();
   glTranslatef(xMotor, yMotor + 0.1f, zMotor + 1);
   // Cover front of engine
   gluDisk(quad, 0, 0.45f, 10, 5);
   glTranslatef(0, 0, 2);
   // Cover rear of engine
   gluDisk(quad, 0, 0.35f, 10, 5);
   glPopMatrix();

   glPushMatrix();
   glTranslatef(- xMotor, yMotor + 0.1f, zMotor + 1);
   // Cover front of engine
   gluDisk(quad, 0, 0.45f, 10, 5);
   glTranslatef(0, 0, 2);
   // Cover rear of engine
   gluDisk(quad, 0, 0.35f, 10, 5);
   glPopMatrix();

   gluDeleteQuadric(quad);
}


// The following array describes material properties.
//    ambiance  [4],
//    diffuse   [4],
//    specular  [4],
//    shininess [1].
// 13 values are required to define a material.

// The order of the definitions must correspond with
// the enumeration MATERIAL in the header file.

const int MAX_MATERIALS = 100;

GLfloat materialProperties[][MAX_MATERIALS] =
{
   { // Black
      0, 0, 0, 1,
      0, 0, 0, 1,
      0, 0, 0, 1, 20
   },
   { // White
      1, 1, 1, 1,
      1, 1, 1, 1,
      1, 1, 1, 1, 75
   },
   { // Red
      1, 0, 0, 1,
      1, 0, 0, 1,
      1, 1, 1, 1, 75
   },
   { // Green
      0, 1, 0, 1,
      0, 1, 0, 1,
      1, 1, 1, 1, 75
   },
   { // Blue
      0, 0, 1, 1,
      0, 0, 1, 1,
      1, 1, 1, 1, 75
   },
   { // Metal
      0.2f, 0.2f, 0.2f, 1,
      0.65f, 0.65f, 0.65f, 1,
      1, 1, 1, 1, 75
   },
   { // Glass
      0.2f, 0.2f, 0.2f, 0.2f,
      0.2f, 0.2f, 0.2f, 0.2f,
      1, 1, 1, 1, 120
   },
   { // Brass
      0.329412f, 0.223529f, 0.027451f, 1.000000f,
      0.780392f, 0.568627f, 0.113725f, 1.000000f,
      0.992157f, 0.941176f, 0.807843f, 1.000000f,
      27.897400f
   },
   { // Bronze
      0.212500f, 0.127500f, 0.054000f, 1.000000f,
      0.714000f, 0.428400f, 0.181440f, 1.000000f,
      0.393548f, 0.271906f, 0.166721f, 1.000000f,
      25.600000f
   },
   { // Polished Bronze
      0.250000f, 0.148000f, 0.064750f, 1.000000f,
      0.400000f, 0.236800f, 0.103600f, 1.000000f,
      0.774597f, 0.458561f, 0.200621f, 1.000000f,
      76.800003f
   },
   { // Chrome
      0.250000f, 0.250000f, 0.250000f, 1.000000f,
      0.400000f, 0.400000f, 0.400000f, 1.000000f,
      0.774597f, 0.774597f, 0.774597f, 1.000000f,
      76.800003f
   },
   { // Copper
      0.191250f, 0.073500f, 0.022500f, 1.000000f,
      0.703800f, 0.270480f, 0.082800f, 1.000000f,
      0.256777f, 0.137622f, 0.086014f, 1.000000f,
      12.800000f
   },
   { // Polished Copper
      0.229500f, 0.088250f, 0.027500f, 1.000000f,
      0.550800f, 0.211800f, 0.066000f, 1.000000f,
      0.580594f, 0.223257f, 0.069570f, 1.000000f,
      51.200001f
   },
   { // Gold
      0.247250f, 0.199500f, 0.074500f, 1.000000f,
      0.751640f, 0.606480f, 0.226480f, 1.000000f,
      0.628281f, 0.555802f, 0.366065f, 1.000000f,
      51.200001f
   },
   { // Polished Gold
      0.247250f, 0.224500f, 0.064500f, 1.000000f,
      0.346150f, 0.314300f, 0.090300f, 1.000000f,
      0.797357f, 0.723991f, 0.208006f, 1.000000f,
      83.199997f
   },
   { // Pewter
      0.105882f, 0.058824f, 0.113725f, 1.000000f,
      0.427451f, 0.470588f, 0.541176f, 1.000000f,
      0.333333f, 0.333333f, 0.521569f, 1.000000f,
      9.846150f
   },
   { // Silver
      0.192250f, 0.192250f, 0.192250f, 1.000000f,
      0.507540f, 0.507540f, 0.507540f, 1.000000f,
      0.508273f, 0.508273f, 0.508273f, 1.000000f,
      51.200001f
   },
   { // Polished Silver
      0.231250f, 0.231250f, 0.231250f, 1.000000f,
      0.277500f, 0.277500f, 0.277500f, 1.000000f,
      0.773911f, 0.773911f, 0.773911f, 1.000000f,
      89.599998f
   },
   { // Emerals
      0.021500f, 0.174500f, 0.021500f, 0.550000f,
      0.075680f, 0.614240f, 0.075680f, 0.550000f,
      0.633000f, 0.727811f, 0.633000f, 0.550000f,
      76.800003f
   },
   { // Jade
      0.135000f, 0.222500f, 0.157500f, 0.950000f,
      0.540000f, 0.890000f, 0.630000f, 0.950000f,
      0.316228f, 0.316228f, 0.316228f, 0.950000f,
      12.800000f
   },
   { // Obsidian
      0.053750f, 0.050000f, 0.066250f, 0.820000f,
      0.182750f, 0.170000f, 0.225250f, 0.820000f,
      0.332741f, 0.328634f, 0.346435f, 0.820000f,
      38.400002f
   },
   { // Pearl
      0.250000f, 0.207250f, 0.207250f, 0.922000f,
      1.000000f, 0.829000f, 0.829000f, 0.922000f,
      0.296648f, 0.296648f, 0.296648f, 0.922000f,
      11.264000f
   },
   { // Ruby
      0.174500f, 0.011750f, 0.011750f, 0.550000f,
      0.614240f, 0.041360f, 0.041360f, 0.550000f,
      0.727811f, 0.626959f, 0.626959f, 0.550000f,
      76.800003f
   },
   { // Turquoise
      0.100000f, 0.187250f, 0.174500f, 0.800000f,
      0.396000f, 0.741510f, 0.691020f, 0.800000f,
      0.297254f, 0.308290f, 0.306678f, 0.800000f,
      12.800000f
   },
   { // Black Plastic
      0.000000f, 0.000000f, 0.000000f, 1.000000f,
      0.010000f, 0.010000f, 0.010000f, 1.000000f,
      0.500000f, 0.500000f, 0.500000f, 1.000000f,
      32.000000f
   },
   { // Black Rubber
      0.020000f, 0.020000f, 0.020000f, 1.000000f,
      0.010000f, 0.010000f, 0.010000f, 1.000000f,
      0.400000f, 0.400000f, 0.400000f, 1.000000f,
      10.000000f
   },
};

void setMaterial(const int m, GLenum face)
{
   glMaterialfv(face, GL_AMBIENT,  &(materialProperties[m][0]));
   glMaterialfv(face, GL_DIFFUSE,  &(materialProperties[m][4]));
   glMaterialfv(face, GL_SPECULAR, &(materialProperties[m][8]));
   glMaterialf(face, GL_SHININESS, materialProperties[m][12]);
}

int matIndex = LAST_VALUE;

int addMaterial(
   GLfloat ambR, GLfloat ambG, GLfloat ambB, GLfloat ambA,
   GLfloat difR, GLfloat difG, GLfloat difB, GLfloat difA,
   GLfloat speR, GLfloat speG, GLfloat speB, GLfloat speA,
   GLfloat shine)
{
   if (matIndex < MAX_MATERIALS)
   {
      materialProperties[matIndex][ 0] = ambR;
      materialProperties[matIndex][ 1] = ambG;
      materialProperties[matIndex][ 2] = ambB;
      materialProperties[matIndex][ 3] = ambA;
      materialProperties[matIndex][ 4] = difR;
      materialProperties[matIndex][ 5] = difG;
      materialProperties[matIndex][ 6] = difB;
      materialProperties[matIndex][ 7] = difA;
      materialProperties[matIndex][ 8] = speR;
      materialProperties[matIndex][ 9] = speG;
      materialProperties[matIndex][10] = speB;
      materialProperties[matIndex][11] = speA;
      materialProperties[matIndex][12] = shine;
      return matIndex++;
   }
   else
   {
      cuglError = TOO_MANY_MATERIALS;
      return 0;
   }
}

int addMaterial(GLfloat params[])
{
   if (matIndex < MAX_MATERIALS)
   {
      for (int i = 0; i < 13; i++)
         materialProperties[matIndex][i] = params[i];
      return matIndex++;
   }
   else
   {
      cuglError = TOO_MANY_MATERIALS;
      return 0;
   }
}

}
; // end of namespace


