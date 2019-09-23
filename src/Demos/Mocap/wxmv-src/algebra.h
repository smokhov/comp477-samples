///////////////////////////////////////////////////////////////////////////////
//
// algebra.h
//
// Purpose:   Declaration of classes for mathematics of vectors, matrices,
//            quaternions, etc.
//            Classes: Vector, IntVector, Matrix, Quaternion
//
// Created:   Jaroslav Semancik, 13/10/2003
//
///////////////////////////////////////////////////////////////////////////////

#ifndef ALGEBRA_H
#define ALGEBRA_H


struct Matrix;      // forward reference for matrix-vector multiplication
union Quaternion;   // forward reference for rotation of a vector by a quaternion

///////////////////////////////////////////////////////////////////////////////
//
// union Vector
//
// A class for vector of 3 real components. It can be accesed both as
// a structure of x, y, z coordinates and an array of c[0], c[1], c[2]
// coordinates. Usual operators are overloaded.
//
///////////////////////////////////////////////////////////////////////////////

union Vector
{
    struct { double x, y, z; };
    double c[3];

    Vector() {}                                             // new V
    Vector(double xx, double yy, double zz)
        : x(xx), y(yy), z(zz) {}                    // new V(1.0f, 2.0f, 3.0f)
    ~Vector() {};

    Vector& operator += (const Vector& v)                   // V += V
        { x += v.x; y += v.y; z += v.z; return *this; }

    Vector& operator -= (const Vector& v)                   // V -= V
        { x -= v.x; y -= v.y; z -= v.z; return *this; }

    Vector& operator *= (double k)                          // V *= k
        { x *= k; y *= k; z *= k; return *this; }

    // column vectors, v' = M * v
    Vector& operator *= (const Matrix& m);                  // V *= M

    Vector& zero()                                          // V.zero()
        { x = y = z = 0.0f; return *this; }

    Vector& normalize();                                    // V.normalize()

    // rotation of the vector by a unit quaternion
    Vector& rotate(const Quaternion& q);                    // V.rotate(Q);

    double length() const                                   // d = V.length()
        { return sqrt(x * x + y * y + z * z); }

    void polar(double& d, double& x_rot, double& y_rot) const;  // V.polar(&d, &xr, &yr)

};

Vector operator + (const Vector& u, const Vector& v);       // V = V + V
Vector operator - (const Vector& u, const Vector& v);       // V = V - V
Vector operator - (const Vector& v);                        // V = -V
Vector operator * (double k, const Vector& v);              // V = k * V
Vector operator * (const Matrix& m, const Vector& v);       // V = M * V
bool operator == (const Vector& u, const Vector& v);        // if (V == V)
bool operator != (const Vector& u, const Vector& v);        // if (V != V)
double dot(const Vector& u, const Vector& v);               // d = dot(V, V)
Vector cross(const Vector& u, const Vector& v);             // V = cross(V, V)
double angle(const Vector& u, const Vector& v);             // a = angle(V, V)
ostream& operator << (ostream& os, const Vector& v);        // cout << V



///////////////////////////////////////////////////////////////////////////////
//
// struct Matrix
//
// Class for 4x4 matrix, used for transformations in 3D. Matrix is stored in
// column-major form ie. matrix columns are continuous in memory, ie.
// c[i][j] is an element of the matrix in i-th column and j-th row. Thus
// the Matrix is compatible with matrix representation in OpenGL, especially
// with glGetFloatv(). Such matrices are multiplied by column vectors on the
// right, ie v' = M * v and a composite transformation from T1 then T2 then
// T3 is an M = M3 * M2 * M1.
//
///////////////////////////////////////////////////////////////////////////////

struct Matrix
{
    double c[4][4];

    Matrix() {};                                            // new M
    Matrix(const double* d);                                // new M(&d)
    // rotation matrix from a unit quaternion
    Matrix(const Quaternion& q);                            // new M(Q)
    ~Matrix() {};
    Matrix& operator += (const Matrix& m);                  // M += M
    Matrix& operator -= (const Matrix& m);                  // M -= M
    Matrix& operator *= (double k);                         // M *= k
    // X = X * M
    Matrix& operator *= (const Matrix& m);                  // M *= M
    Matrix& zero();                                         // M.zero()
    Matrix& identity();                                     // M.identity()
};

Matrix operator + (const Matrix& a, const Matrix& b);       // M = M + M
Matrix operator - (const Matrix& a, const Matrix& b);       // M = M - M
Matrix operator - (const Matrix& m);                        // M = -M
Matrix operator * (double k, const Matrix& m);              // M = k * M
// X = A * B
Matrix operator * (const Matrix& a, const Matrix& b);       // M = M * M
bool operator == (const Matrix& a, const Matrix& b);        // if (M == M)
bool operator != (const Matrix& a, const Matrix& b);        // if (M != M)
ostream& operator << (ostream& os, const Matrix& m);        // cout << V



///////////////////////////////////////////////////////////////////////////////
//
// union Quaternion
//
// Class for quaternions. Components can be accessed both as x, y, z, w or
// c[0]..c[3]. Comprises conversions from axis-angle and matrix
// representations (column major, struct Matrix) of rotation.
//
///////////////////////////////////////////////////////////////////////////////

union Quaternion
{
    struct { double x, y, z, w; };
    double c[4];

    Quaternion() {};                                        // new Q
    Quaternion(double xx, double yy, double zz, double ww)
        : x(xx), y(yy), z(zz), w(ww) {}                     // new Q(1, 2, 3, 4);

    Quaternion(const Vector& axis, double angle);           // new Q(V, a)

    // unit quaternion from a rotation matrix
    Quaternion(const Matrix& m);                            // new Q(M)

    ~Quaternion() {};

    Quaternion& operator += (const Quaternion& q)           // Q += Q
        { x += q.x; y += q.y; z += q.z; w += q.w; return *this; }

    Quaternion& operator -= (const Quaternion& q)           // Q -= Q
        { x -= q.x; y -= q.y; z -= q.z; w -= q.w; return *this; }

    Quaternion& operator *= (double k)                      // Q *= k
        { x *= k; y *= k; z *= k; w *= k; return *this; }

    // x = x * q
    Quaternion& operator *= (const Quaternion q);           // Q *= Q

    Quaternion& zero()                                      // Q.zero()
         { x = y = z = w = 0.0f; return *this; }

    Quaternion& identity()                                  // Q.identity()
         { x = y = z = 0.0f; w = 1.0f; return *this; }

    Quaternion& conjugate()                                 // Q.conjugate()
        { x = -x; y = -y; z = -z; return *this; }

    Quaternion& invert();                                   // Q.invert()
    
    Quaternion& normalize();                                // Q.normalize()

    Quaternion& Quaternion::log();                          // Q.log()

    Quaternion& Quaternion::exp();                          // Q.exp()

    double length() const                                   // d = Q.length()
        { return sqrt(x * x + y * y + z * z + w * w); }
};

Quaternion operator + (const Quaternion& p, const Quaternion& q);   // Q = Q + Q
Quaternion operator - (const Quaternion& p, const Quaternion& q);   // Q = Q - Q
Quaternion operator - (const Quaternion& q);                        // Q = -Q
Quaternion operator * (double k, const Quaternion& q);              // Q = k * Q
// x = p * q
Quaternion operator * (const Quaternion& p, const Quaternion& q);   // Q = Q * Q
bool operator == (const Quaternion& p, const Quaternion& q);        // if (Q == Q)
bool operator != (const Quaternion& p, const Quaternion& q);        // if (Q != Q)
double dot(const Quaternion& p, const Quaternion& q);               // d = dot(Q, Q)
Quaternion slerp(const Quaternion& p, const Quaternion& q, double t); // Q = slerp(Q, Q, k)
ostream& operator << (ostream& os, const Quaternion& q);            // cout << Q



///////////////////////////////////////////////////////////////////////////////
//
// predefined constants
//
///////////////////////////////////////////////////////////////////////////////

extern "C++" Vector zero_vector;
extern "C++" Vector x_vector;
extern "C++" Vector y_vector;
extern "C++" Vector z_vector;

extern "C++" Quaternion zero_quaternion;
extern "C++" Quaternion identity_quaternion;

extern "C++" Matrix zero_matrix;
extern "C++" Matrix identity_matrix;

#endif

