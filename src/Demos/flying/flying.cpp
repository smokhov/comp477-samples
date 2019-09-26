/** \file flying.cpp
 *  Concordia University Graphics Library
 *
 * \authors Peter Grogono
 * \date November 2005
 */

/**
 * \addtogroup examples
 * @{
 * flying.cpp shows how to use quaternions to "fly" a plane.
 * Aerodynamics are not simulated.
 * @}
 */

#include <cstdlib>
#include <sstream>
#include <iomanip>
#include <GL/glut.h>
#include "cugl.h"
#include "windows.h"

using namespace std;
using namespace cugl;

// Initial size of graphics window.
const int WIDTH  = 900;
const int HEIGHT = 600;

// Current size of window.
int width  = WIDTH;
int height = HEIGHT;

// Bounds of viewing frustum.
const double NEAR_PLANE =  10;
const double FAR_PLANE  = 1500;
const double DISTANCE = -200;

// Viewing angle.
double fovy = 40.0;

// Lighting
GLfloat ambient[] = { 0.4f, 0.4f, 0.4f, 1 };
GLfloat lightPos[] = { 5, 20, 10, 1 };

const double DELTA_TIME = 5; // milliseconds
const double DELTA_TURN = radians(5);
Vector speed(0, 0, -0.02);

// The speed is constant. It is negative because
// of the plane's coordinate system.

Vector velocity;                   // Current velocity.
Vector position;                   // Current position.
Quaternion orientation;            // Current orientation.

// Quaternions for small rotations about each axis ...
const Quaternion climb(I, DELTA_TURN);
const Quaternion leftTurn(J, DELTA_TURN);
const Quaternion roll(K, DELTA_TURN);

// ... and their inverses
const Quaternion climbInv = climb.inv();
const Quaternion leftInv = leftTurn.inv();
const Quaternion rollInv = roll.inv();

GLuint plane;                 // Index for model.

void reset()
// Initializes at start-up and
// in response to 'r' key.
{
   orientation = Quaternion(J, radians(90));
   velocity = orientation.inv().apply(speed);
   position = Vector();
}

void initialize()
// Set up background, lighting, and plane.
{
   glClearColor(0.6, 0.6, 1, 1);
   glEnable(GL_DEPTH_TEST);
   glEnable(GL_LIGHTING);
   glEnable(GL_LIGHT0);
   glLightfv(GL_LIGHT0, GL_AMBIENT, ambient);
   glLightfv(GL_LIGHT0, GL_POSITION, lightPos);
   plane = makePlaneList();
   reset();
}

// Convert an angle in radians to degrees, as an integer in [0,359].
int fix(double angle)
{
   int d = int(degrees(angle));
   if (d < 0)
      d += 360;
   return d;
}

void display ()
{
   // Display Euler angles and DISTANCE in title bar.
   double x, y, z;
   orientation.euler(x, y, z);
   ostringstream os;
   os <<
   "Euler angles: P = " << setw(3) << fix(x) <<
   "   Y = " << setw(3) << fix(y) <<
   "   R = " << setw(3) << fix(z) <<
   "   Distance: " << setw(6) << int((position - Vector(0, 0, -DISTANCE)).length());
   glutSetWindowTitle(os.str().c_str());

   // Display the scene.
   glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
   glMatrixMode(GL_MODELVIEW);
   glLoadIdentity();
   glTranslatef(0, 0, DISTANCE);

   // Move, rotate, and display model.
   position.translate();
   orientation.apply();
   glCallList(plane);

   glutSwapBuffers();
}

double realTime = GetTickCount();
double pathTime = realTime;

void idle ()
// Update position by first-order integration.
{
   realTime = GetTickCount();
   while (pathTime < realTime)
   {
      position += velocity * DELTA_TIME;
      pathTime += DELTA_TIME;
   }
   glutPostRedisplay();
}

void reshapeMainWindow (int newWidth, int newHeight)
{
   width = newWidth;
   height = newHeight;
   glViewport(0, 0, width, height);
   glMatrixMode(GL_PROJECTION);
   glLoadIdentity();
   gluPerspective(fovy, GLfloat(width) / GLfloat(height), NEAR_PLANE, FAR_PLANE);
}

void graphicKeys (unsigned char key, int x, int y)
{
   switch (key)
   {
      case 'r':
         reset();
         break;
      case 'f':
         glutFullScreen();
         break;
      case '+':
         speed[2] *= 1.1;
         velocity = orientation.inv().apply(speed);
         break;
      case '-':
         speed[2] /= 1.1;
         velocity = orientation.inv().apply(speed);
         break;
      case 27:
         exit(0);
   }
}

void functionKeys (int key, int x, int y)
{
   switch (key)
   {
      case GLUT_KEY_UP:
         orientation *= climb;
         break;
      case GLUT_KEY_DOWN:
         orientation *= climbInv;
         break;
      case GLUT_KEY_LEFT:
         orientation *= leftTurn;
         break;
      case GLUT_KEY_RIGHT:
         orientation *= leftInv;
         break;
      case GLUT_KEY_END:
         orientation *= roll;
         break;
      case GLUT_KEY_PAGE_DOWN:
         orientation *= rollInv;
         break;
   }
   velocity = orientation.inv().apply(speed);
}

int main (int argc, char **argv)
{
   glutInit(&argc, argv);
   glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGB);
   glutInitWindowSize(width, height);
   glutCreateWindow("");
   glutDisplayFunc(display);
   glutReshapeFunc(reshapeMainWindow);
   glutKeyboardFunc(graphicKeys);
   glutSpecialFunc(functionKeys);
   glutIdleFunc(idle);
   initialize();
   cout <<
        "Flight simulator\n\n"
        "Keys:\n"
        "Up arrow      climb\n"
        "Down arrow    descend\n"
        "Left arrow    turn leftTurn\n"
        "Right arrow   turn right\n"
        "End           bank leftTurn\n"
        "PgDn          bank right\n"
        "+             fly faster\n"
        "-             fly slower\n"
        "r             reset initial conditions\n"
        "f             full screen\n" << endl;
   glutMainLoop();
   return 0;
}
