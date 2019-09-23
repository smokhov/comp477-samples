///////////////////////////////////////////////////////////////////////////////
//
// base.h
//
// Purpose:   Fundamental data types and function declarations
//
// Created:   Jaroslav Semancik, 8/10/2003
//
///////////////////////////////////////////////////////////////////////////////

#ifndef BASE_H
#define BASE_H

// some helpful math macros

#define EPSILON     0.00000001
#define PI          3.141592653589793
#define RAD2DEG(x)  (180.0 * (x) / PI)      // radians to degrees conversion
#define DEG2RAD(x)  (PI * (x) / 180.0)      // degrees to radians conversion
#define SQR(x)      ((x) * (x))             // square root
#define ZERO(x)     (fabs(x) < EPSILON)     // zero test for floating point numbers
#define EQUAL(x, y) (fabs((x) - (y)) < EPSILON) // test for equality of float numbers
#define ROUND(x)    (floor((x) + 0.5))      // floating point number rounding

// float random number from interval < l ; u >
#define RAND(l,u)   ((float)rand() / RAND_MAX * ((u) - (l)) + (l))


// Function for reporting of errors.
// Implemented as simple console output or a message box under wxWindows.

void ErrorMessage(const string& msg);


// Strips and returns extension from filename. Extension is considered as
// a smallest suffix starting by period.

string StripExtension(string& filename);

#endif

