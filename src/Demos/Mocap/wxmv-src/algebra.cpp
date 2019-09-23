///////////////////////////////////////////////////////////////////////////////
//
// algebra.cpp
//
// Purpose:   Implementation of classes for mathematics of vectors, matrices,
//            quaternions, etc.
//            Classes: Vector, IntVector, Matrix, Quaternion.
//
// Created:   Jaroslav Semancik, 13/10/2003
//
///////////////////////////////////////////////////////////////////////////////

#include <math.h>
#include <iostream>

using namespace std;

#include "base.h"
#include "algebra.h"

///////////////////////////////////////////////////////////////////////////////
//
// union Vector methods
//
///////////////////////////////////////////////////////////////////////////////

Vector& Vector::operator *= (const Matrix& m)
{
    double xx, yy, zz;

    xx = x;
    yy = y;
    zz = z;

    x = xx * m.c[0][0] + yy * m.c[1][0] + zz * m.c[2][0] + m.c[3][0];
    y = xx * m.c[0][1] + yy * m.c[1][1] + zz * m.c[2][1] + m.c[3][1];
    z = xx * m.c[0][2] + yy * m.c[1][2] + zz * m.c[2][2] + m.c[3][2];

    return *this;
}


// adjust vector to unit length, does not affect a zero vector

Vector& Vector::normalize()
{
    double d = length();
    if (ZERO(d)) return *this;

    x /= d;
    y /= d;
    z /= d;

    return *this;
}


// Rotation of the vector by a unit quaternion q. A pure quaternion
// p = (x, y, z, 0) corresponding to the vector is multiplied to
// p' = q * p * q^-1 and the vector is extracted back from p'.
// Note that: q^-1 = q.cnj for a unit quaternion,
//            q must be a unit quaternion to get a correct result!

Vector& Vector::rotate(const Quaternion& q)
{
    Quaternion p(x, y, z, 0.0f);
    Quaternion qc(q);
    qc.conjugate();
    Quaternion pp(q * p * qc);

    x = pp.x;
    y = pp.y;
    z = pp.z;

    return *this;
}


// Calculate length of the vector and rotation angles needed to align
// positive Z-axis onto vector direction. In x_rot and y_rot are angles
// (in radians!) to rotate around x-axis and y-axis respectively.

void Vector::polar(double& d, double& x_rot, double& y_rot) const
{
    d = length();
    double d2 = sqrt(x * x + z * z);
    if (!ZERO(d2))
    {
        x_rot = atan(-y / d2);
        y_rot = asin(x / d2);
        if (z < 0.0f) y_rot = PI - y_rot;
    }
    else
    {
        x_rot = (y > 0.0f) ? -PI/2 : PI/2;
        y_rot = 0.0f;
    }
}


Vector operator + (const Vector& u, const Vector& v)
{
    return Vector(u.x + v.x, u.y + v.y, u.z + v.z);
}


Vector operator - (const Vector& u, const Vector& v)
{
    return Vector(u.x - v.x, u.y - v.y, u.z - v.z);
}


Vector operator - (const Vector& v)
{
    return Vector(-v.x, -v.y, -v.z);
}


Vector operator * (double k, const Vector& v)
{
    return Vector(k * v.x, k * v.y, k * v.z);
}


// multiplication of vector v extended by 4-th homogeneous coordinate = 1
// and matrix M, v' = M * v

Vector operator * (const Matrix& m, const Vector& v)
{
    return Vector(
        v.x * m.c[0][0] + v.y * m.c[1][0] + v.z * m.c[2][0] + m.c[3][0],
        v.x * m.c[0][1] + v.y * m.c[1][1] + v.z * m.c[2][1] + m.c[3][1],
        v.x * m.c[0][2] + v.y * m.c[1][2] + v.z * m.c[2][2] + m.c[3][2]);
}


bool operator == (const Vector& u, const Vector& v)
{
    Vector d(u - v);
    return ZERO(d.x * d.x + d.y * d.y + d.z * d.z);
}


bool operator != (const Vector& u, const Vector& v)
{
    Vector d(u - v);
    return !ZERO(d.x * d.x + d.y * d.y + d.z * d.z);
}


// dot product

double dot(const Vector& u, const Vector& v)
{
    return u.x * v.x + u.y * v.y + u.z * v.z;
}


// cross product

Vector cross(const Vector& u, const Vector& v)
{
    return Vector(u.y * v.z - u.z * v.y,
                  u.z * v.x - u.x * v.z,
                  u.x * v.y - u.y * v.x);
}


// angle of two vectors

double angle(const Vector& u, const Vector& v)
{
    double du, dv, dp;

    du = u.length();
    dv = v.length();
    if (ZERO(du) || ZERO(dv)) return 0.0f;

    dp = (u.x * v.x + u.y * v.y + u.z * v.z) / (du * dv);
    if (dp >= 1.0f) return 0.0f;
    if (dp <= -1.0f) return PI;

    return acos(dp);
}


// print a vector

ostream& operator << (ostream& os, const Vector& v)
{
    return os << '(' << v.x << ", " << v.y << ", " << v.z << ')';
}



///////////////////////////////////////////////////////////////////////////////
//
// struct Matrix methods
//
///////////////////////////////////////////////////////////////////////////////

// construction of matrix from an array of values, first 4 values of array d
// fill first column of the matrix, etc. (like in OpenGL)

Matrix::Matrix(const double *d)
{
    int i, j;

    for (i = 0; i < 4; i++)
        for (j = 0; j < 4; j++)
            c[i][j] = d[i << 2 + j];
}


// construction of matrix representing the same rotation as a unit quaternion

Matrix::Matrix(const Quaternion& q)
{
    double s, xs, ys, zs, wx, wy, wz, xx, xy, xz, yy, yz, zz, d;

    d = q.x * q.x + q.y * q.y + q.z * q.z + q.w * q.w;
    if (ZERO(d))
        s = 0.0f;
    else
        s = 2.0f / d;

    xs = q.x * s;   ys = q.y * s;   zs = q.z * s;
    wx = q.w * xs;  wy = q.w * ys;  wz = q.w * zs;
    xx = q.x * xs;  xy = q.x * ys;  xz = q.x * zs;
    yy = q.y * ys;  yz = q.y * zs;  zz = q.z * zs;

    c[0][0] = 1.0f - (yy + zz);
    c[1][0] = xy - wz;
    c[2][0] = xz + wy;
    c[0][1] = xy + wz;
    c[1][1] = 1.0f - (xx + zz);
    c[2][1] = yz - wx;
    c[0][2] = xz - wy;
    c[1][2] = yz + wx;
    c[2][2] = 1.0f - (xx + yy);
    c[3][0] = c[3][1] = c[3][2] = c[0][3] = c[1][3] = c[2][3] = 0.0f;
    c[3][3] = 1.0f;
}


Matrix& Matrix::operator += (const Matrix& m)
{
    int i, j;

    for (i = 0; i < 4; i++)
        for (j = 0; j < 4; j++)
        	c[i][j] += m.c[i][j];

    return *this;
}


Matrix& Matrix::operator -= (const Matrix& m)
{
    int i, j;

    for (i = 0; i < 4; i++)
        for (j = 0; j < 4; j++)
        	c[i][j] -= m.c[i][j];

    return *this;
}


Matrix& Matrix::operator *= (double k)
{
    int i, j;

    for (i = 0; i < 4; i++)
        for (j = 0; j < 4; j++)
        	c[i][j] *= k;

    return *this;
}


// X = X * M

Matrix& Matrix::operator *= (const Matrix& m)
{
    int i, j;
    double tmp[4];

    for (i = 0; i < 4; i++)
    {
        for (j = 0; j < 4; j++)
            tmp[j] = c[0][i] * m.c[j][0] + c[1][i] * m.c[j][1]
                   + c[2][i] * m.c[j][2] + c[3][i] * m.c[j][3];

        for (j = 0; j < 4; j++)
            c[j][i] = tmp[j];
    }
    return *this;
}


Matrix& Matrix::zero()
{
    int i, j;

    for (i = 0; i < 4; i++)
        for (j = 0; j < 4; j++)
            c[i][j] = 0.0f;
            
    return *this;
}


Matrix& Matrix::identity()
{
    int i, j;

    for (i = 0; i < 4; i++)
        for (j = 0; j < 4; j++)
            c[i][j] = 0.0f;

    for (i = 0; i < 4; i++)
        c[i][i] = 1.0f;
        
    return *this;
}


Matrix operator + (const Matrix& a, const Matrix& b)
{
    int i, j;
    Matrix res;

    for (i = 0; i < 4; i++)
        for (j = 0; j < 4; j++)
        	res.c[i][j] = a.c[i][j] + b.c[i][j];

    return res;
}


Matrix operator - (const Matrix& a, const Matrix& b)
{
    int i, j;
    Matrix res;

    for (i = 0; i < 4; i++)
        for (j = 0; j < 4; j++)
        	res.c[i][j] = a.c[i][j] - b.c[i][j];

    return res;
}


Matrix operator - (const Matrix& m)
{
    int i, j;
    Matrix res;

    for (i = 0; i < 4; i++)
        for (j = 0; j < 4; j++)
        	res.c[i][j] = -m.c[i][j];

    return res;
}


Matrix operator * (double k, const Matrix& m)
{
    int i, j;
    Matrix res;

    for (i = 0; i < 4; i++)
        for (j = 0; j < 4; j++)
        	res.c[i][j] = k * m.c[i][j];

    return res;
}


// X = A * B

Matrix operator * (const Matrix& a, const Matrix& b)
{
    int i, j;
    Matrix res;

    for (i = 0; i < 4; i++)
        for (j = 0; j < 4; j++)
            res.c[j][i] = a.c[0][i] * b.c[j][0] + a.c[1][i] * b.c[j][1]
                        + a.c[2][i] * b.c[j][2] + a.c[3][i] * b.c[j][3];
    return res;
}


bool operator == (const Matrix& a, const Matrix& b)
{
    int i, j;

    for (i = 0; i < 4; i++)
        for (j = 0; j < 4; j++)
            if (!EQUAL(a.c[i][j], b.c[i][j])) return false;

    return true;
}


bool operator != (const Matrix& a, const Matrix& b)
{
    int i, j;

    for (i = 0; i < 4; i++)
        for (j = 0; j < 4; j++)
            if (!EQUAL(a.c[i][j], b.c[i][j])) return true;

    return false;
}


// print a formated matrix

ostream& operator << (ostream& os, const Matrix& m)
{
    return os << endl << m.c[0][0] << "\t" << m.c[1][0] << "\t" << m.c[2][0] << '\t' << m.c[3][0]
              << endl << m.c[0][1] << "\t" << m.c[1][1] << "\t" << m.c[2][1] << '\t' << m.c[3][1]
              << endl << m.c[0][2] << "\t" << m.c[1][2] << "\t" << m.c[2][2] << '\t' << m.c[3][2]
              << endl << m.c[0][3] << "\t" << m.c[1][3] << "\t" << m.c[2][3] << '\t' << m.c[3][3]
              << endl;
}



///////////////////////////////////////////////////////////////////////////////
//
// union Quaternion methods
//
///////////////////////////////////////////////////////////////////////////////

// construction of quaternion from an axis and angle

Quaternion::Quaternion(const Vector& axis, double angle)
{
    double d, s, theta;

    d = axis.length();
    if (ZERO(d))
    {
    	identity();
    	return;
    }

    theta = 0.5f * angle;
    s = sin(theta) / d;
    x = s * axis.x;
    y = s * axis.y;
    z = s * axis.z;
    w = cos(theta);
}


// construction of unit quaternion from a 4x4 rotation matrix

Quaternion::Quaternion(const Matrix& m)
{
    double tr, s;
    int i, j, k;
    static int nxt[3] = {1, 2, 0};

    tr = m.c[0][0] + m.c[1][1] + m.c[2][2];
    if (tr > 0.0f)
    {
        s = sqrt(tr + 1.0f);
        w = s * 0.5f;
        s = 0.5f / s;
        x = (m.c[1][2] - m.c[2][1]) * s;
        y = (m.c[2][0] - m.c[0][2]) * s;
        z = (m.c[0][1] - m.c[1][0]) * s;
    }

    else {
        i = 0;
        if (m.c[1][1] > m.c[0][0]) i = 1;
        if (m.c[2][2] > m.c[i][i]) i = 2;
        j = nxt[i];
        k = nxt[j];
        s = sqrt((m.c[i][i] - (m.c[j][j] + m.c[k][k])) + 1.0f);
        c[i] = s * 0.5f;

        if (ZERO(s)) identity();
        else {
            s = 0.5f / s;
            w = (m.c[j][k] - m.c[k][j]) * s;
            c[j] = (m.c[i][j] + m.c[j][i]) * s;
            c[k] = (m.c[i][k] + m.c[k][i]) * s;
        }
    }
}


Quaternion& Quaternion::operator *= (const Quaternion q)
{
    double xx, yy, zz, ww;

    xx = x;
    yy = y;
    zz = z;
    ww = w;

    x = ww * q.x + xx * q.w + yy * q.z - zz * q.y;
    y = ww * q.y + yy * q.w + zz * q.x - xx * q.z;
    z = ww * q.z + zz * q.w + xx * q.y - yy * q.x;
    w = ww * q.w - xx * q.x - yy * q.y - zz * q.z;

    return *this;
}


// inverse quaternion

Quaternion& Quaternion::invert()
{
    double k;

    k = x * x + y * y + z * z + w * w;
    if (ZERO(k))
    {
        identity();
        return *this;
    }

    k = 1.0f / k;
    x *= -k;
    y *= -k;
    z *= -k;
    w *= k;
        
    return *this;
}


// adjust quaternion to unit length

Quaternion& Quaternion::normalize()
{
    double d, dd;

    d = length();
    if (ZERO(d))
    {
    	identity();
        return *this;
    }

	dd = 1.0f / d;
    x *= dd;
    y *= dd;
    z *= dd;
    w *= dd;

    return *this;
}


// logarithm of quaternion

Quaternion& Quaternion::log()
{
    double d, k, theta;

    d = sqrt(x * x + y * y + z * z);
    theta = atan2(d, w);

    if (ZERO(d))
        k = 1.0f;
    else
        k = theta / d;

    x *= k;
    y *= k;
    z *= k;
    w = 0.0f;

    return *this;
}


// exponentiation of quaternion

Quaternion& Quaternion::exp()
{
    double k, theta;

    theta = sqrt(x * x + y * y + z * z);
    if (ZERO(theta))
        k = 1.0f;
    else
        k = sin(theta) / theta;

    x *= k;
    y *= k;
    z *= k;
    w = cos(theta);

    return *this;
}


Quaternion operator + (const Quaternion& p, const Quaternion& q)
{
    return Quaternion(p.x + q.x, p.y + q.y, p.z + q.z, p.w + q.w);
}


Quaternion operator - (const Quaternion& p, const Quaternion& q)
{
    return Quaternion(p.x - q.x, p.y - q.y, p.z - q.z, p.w - q.w);
}


Quaternion operator - (const Quaternion& q)
{
    return Quaternion(-q.x, -q.y, -q.z, -q.w);
}


Quaternion operator * (double k, const Quaternion& q)
{
    return Quaternion(k * q.x, k * q.y, k * q.z, k * q.w);
}


Quaternion operator * (const Quaternion& p, const Quaternion& q)
{
    return Quaternion(
        p.w * q.x + p.x * q.w + p.y * q.z - p.z * q.y,
        p.w * q.y + p.y * q.w + p.z * q.x - p.x * q.z,
        p.w * q.z + p.z * q.w + p.x * q.y - p.y * q.x,
        p.w * q.w - p.x * q.x - p.y * q.y - p.z * q.z);
}


bool operator == (const Quaternion& p, const Quaternion& q)
{

    Quaternion d = p - q;
    return ZERO(d.x * d.x + d.y * d.y + d.z * d.z + d.w * d.w);
}


bool operator != (const Quaternion& p, const Quaternion& q)
{
    Quaternion d = p - q;
    return !ZERO(d.x * d.x + d.y * d.y + d.z * d.z + d.w * d.w);
}


// dot product of two quaternions

double dot(const Quaternion& p, const Quaternion& q)
{
    return p.x * q.x + p.y * q.y + p.z * q.z + p.w * q.w;
}


// what is the small cosine to change from spherical to linear interpolation 
#define SLERP_EPSILON   0.05f

Quaternion slerp(const Quaternion& p, const Quaternion& q, double t)
{
    double theta, sintheta, costheta, pscl, qscl = 1.0f;
    
    costheta = p.x * q.x + p.y * q.y + p.z * q.z + p.w * q.w;

    // if angle bewteeen quaternions is > PI/2, negate q, to choose
    // the shorter one of two paths for interpolation
    if (costheta < 0.0f)
    {
        qscl = -1.0f;
        costheta = -costheta;
    }
    
    // for close quaternions use linear interpolation instead
    if ((1 - costheta) < SLERP_EPSILON)
    {
        pscl = 1.0f - t;
        qscl *= t;
    }
    // otherwise actually do spherical interpolation
    else
    {
        theta = acos(costheta);
        sintheta = sin(theta);
        pscl = sin((1.0f - t) * theta) / sintheta;
        qscl *= sin(t * theta) / sintheta;
    }

    return Quaternion(pscl * p.x + qscl * q.x,
                      pscl * p.y + qscl * q.y,
                      pscl * p.z + qscl * q.z,
                      pscl * p.w + qscl * q.w);
}


// print a quaternion

ostream& operator << (ostream& os, const Quaternion& q)
{
    return os << '(' << q.x << ", " << q.y << ", " << q.z << ", " << q.w << ')';
}



///////////////////////////////////////////////////////////////////////////////
//
// predefined constants
//
///////////////////////////////////////////////////////////////////////////////

Vector zero_vector(0.0, 0.0, 0.0),
       x_vector(1.0, 0.0, 0.0),
       y_vector(0.0, 1.0, 0.0),
       z_vector(0.0, 0.0, 1.0);

Quaternion zero_quaternion(0.0, 0.0, 0.0, 0.0),
           identity_quaternion(0.0, 0.0, 0.0, 1.0);

double _zm[16] = { 0.0 };
double _im[16] = { 1.0, 0.0, 0.0, 0.0,
                   0.0, 1.0, 0.0, 0.0,
                   0.0, 0.0, 1.0, 0.0,
                   0.0, 0.0, 0.0, 1.0 };

Matrix zero_matrix(_zm),
       identity_matrix(_im);

