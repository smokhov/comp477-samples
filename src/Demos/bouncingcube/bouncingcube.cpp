// Bouncing cube demonstration.

// Link with: opengl32.lib, glu32.lib, glut32.lib, cugl.lib.

#define GLUT_DISABLE_ATEXIT_HACK

#include <cassert>
#include <cmath>
#include <iostream>
#include <sstream>
#include <iomanip>
#include <GL/glut.h>
#include <GL/cugl.h>

using namespace std;
using namespace cugl;

// Initial size of graphics window.
const int WIDTH  = 900;
const int HEIGHT = 600;

// Current size of window.
int width  = WIDTH;
int height = HEIGHT;

// Viewing frustum.
double nearPlane =  5;
double farPlane  = 15;
double distance = 10;
double fovy = 40.0;

// Lighting and shadow.
GLfloat AMBIENT[] = { 0.4f, 0.4f, 0.4f, 1 };
GLfloat LIGHT_POSITION[] = { 10, 15, 1, 1 };
Plane SHADOW_PLANE = Plane(0, 1, 0, -0.01);
Matrix shadowMat(LIGHT_POSITION, SHADOW_PLANE);

// Motion parameters.
const Vector ACCELERATION(0, -1, 0); // Effect of gravity
Vector prevPosition, position;
Vector prevVelocity, velocity;
Quaternion prevOrientation, orientation;
Vector angvel;
double dt;

// Displacements of cube vertexes from centre of cube.
Vector vts[8] =
{
   Vector(-0.5, -0.5, +0.5),
   Vector(-0.5, +0.5, +0.5),
   Vector(+0.5, +0.5, +0.5),
   Vector(+0.5, -0.5, +0.5),
   Vector(-0.5, -0.5, -0.5),
   Vector(-0.5, +0.5, -0.5),
   Vector(+0.5, +0.5, -0.5),
   Vector(+0.5, -0.5, -0.5)
};

// Reset initial conditions.
void initialize()
{
   prevPosition = Vector(0, 5, 0);
   position = prevPosition;
   prevVelocity = Vector(0, 0, 0);
   velocity = prevVelocity;
   angvel = Vector(0, 0, 0);
   prevOrientation = Quaternion(Vector(randReal(), randReal(), randReal()), randReal()).unit();
   orientation = prevOrientation;
   dt = 0.0002;
}

// Update graphics window.
void display ()
{
   glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
   glMatrixMode(GL_MODELVIEW);
   glLoadIdentity();
   gluLookAt(0, 5, -10,  0, 0, 10,  0, 1, 0);

   // Draw the floor.
   setMaterial(METAL);
   glBegin(GL_QUADS);
   glNormal3f(0, 1, 0);
   glVertex3f(+10, 0, -100);
   glVertex3f(+10, 0, +100);
   glVertex3f(-10, 0, +100);
   glVertex3f(-10, 0, -100);
   glEnd();

   // Draw the cube.
   glPushMatrix();
   position.translate();
   orientation.apply();
   setMaterial(GOLD);
   glutSolidCube(1);
   glPopMatrix();

   // Draw the cube's shadow.
   shadowMat.apply();
   position.translate();
   orientation.apply();
   setMaterial(BLACK);
   glutSolidCube(1);

   glutSwapBuffers();
}

// Inverse inertia tensor for a cube
//const GLfloat DIAG = 5.0 / 22.0;
//const GLfloat OFFD = 3.0 / 22.0;

const GLfloat DIAG = 1.0 / 6.0;
const GLfloat OFFD = 0.0;
Matrix mi(
   DIAG, OFFD, OFFD, 0.0f,
   OFFD, DIAG, OFFD, 0.0f,
   OFFD, OFFD, DIAG, 0.0f,
   0.0f, 0.0f, 0.0f, 1.0f);

Matrix invmi = mi.inv();

double linearEnergy(Vector & v)
{
   return 0.5 * dot(v, v);
}

double angularEnergy(Vector & w)
{
   return w.norm() / 6;
//   const double T = 1.0 / 6.0;
//   const double Q = 0.0;
//   return
//      w[0] * (+ T * w[0] - Q * w[1] - Q * w[2]) +
//      w[1] * (- Q * w[0] + T * w[1] - Q * w[2]) +
//      w[2] * (- Q * w[0] - Q * w[1] + T * w[2]);
}

void collision(int vi)
{
   const double K = 10000;

   // Backup to previous position
   velocity = prevVelocity;
   position = prevPosition;
   orientation = prevOrientation;

   double sdt = 0.1 * dt;

   // Loop while corner is above ground
   while (true)
   {
      double height = (position + orientation.apply(vts[vi]))[1];
      if (height < 0)
         break;

      // Update position and orientation of cube.
      velocity += ACCELERATION * sdt;
      position += velocity * sdt;
      orientation.integrate(angvel, sdt);
   }

   // Loop while corner is below ground
   while (true)
   {
      double height = (position + orientation.apply(vts[vi]))[1];
      if (height > 0)
         break;
      Vector force(0, -K * height, 0);
      velocity += force * sdt;
      position += velocity * sdt;

      Vector torque = cross(orientation.apply(vts[vi]), force);
      Vector angacc = torque / 12;
      angvel += angacc * sdt;
      orientation.integrate(angvel, sdt);
   }
}

void report(string desc)
{
      double linKE = linearEnergy(velocity);
      double angKE = angularEnergy(angvel);
      double totKE = linKE + angKE;
      ostringstream os;
      os.setf(ios::fixed, ios::floatfield);
      os.precision(2);
      os << desc << setw(7) << linKE << setw(7) << angKE << setw(7) << totKE;
      os.precision(5);
      os << setw(10) << angvel;
      const char *msg = os.str().c_str();
      cerr << msg << endl;
}

// Background calculations.
void idle ()
{
   static int numCols = 0;

   // Save values from last step
   prevVelocity = velocity;
   prevPosition = position;
   prevOrientation = orientation;

   // Update position and orientation of cube.
   velocity += ACCELERATION * dt;
   position += velocity * dt;
   orientation.integrate(angvel, dt);

   // Find lowest vertex.
   int minVindex = 0;
   double minHeight = 10;
   for (int i = 0; i < 8; i++)
   {
      Vector minVertex;
      Vector vertex = orientation.apply(vts[i]);
      double height = (position + vertex)[1];
      if (minHeight > height)
      {
         minHeight = height;
         minVertex = vertex;
         minVindex = i;
      }
   }

   // If it's below the floor, perform collision calculation.
   if (velocity[1] < 0 && minHeight < 0)
   {
      numCols++;
      report("Before: ");
      collision(minVindex);
      report("After:  ");
   }
   glutPostRedisplay();
}

// Callback for reshaping graphics window.
void reshapeMainWindow (int newWidth, int newHeight)
{
   width = newWidth;
   height = newHeight;
   glViewport(0, 0, width, height);
   glMatrixMode(GL_PROJECTION);
   glLoadIdentity();
   gluPerspective(fovy, GLfloat(width) / GLfloat(height), nearPlane, farPlane);
}

// Respond to graphic keys.
void graphicKeys (unsigned char key, int x, int y)
{
   switch (key)
   {
      case ' ':
         initialize();
         break;
      case '+':
         dt *= 1.1;
         break;
      case '-':
         dt /= 1.1;
         break;
      case 27:
         exit(0);
   }
}

int main (int argc, char **argv)
{
   cout <<
        "Bouncing cube demonstration." << endl <<
        "Press these keys:" << endl <<
        " ' ' (space)  restore initial conditions" << endl <<
        " '+'          speed up" << endl <<
        " '-'          slow down" << endl <<
        " ESC          quit" << endl;

   glutInit(&argc, argv);
   glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGB);
   glutInitWindowSize(width, height);
   glutInitWindowPosition(800,0);
   glutCreateWindow("Bouncing Cube");
   glutDisplayFunc(display);
   glutReshapeFunc(reshapeMainWindow);
   glutKeyboardFunc(graphicKeys);
   glutIdleFunc(idle);
   glClearColor(0.6, 0.6, 1, 1);
   glEnable(GL_DEPTH_TEST);
   glEnable(GL_LIGHTING);
   glEnable(GL_LIGHT0);
   glLightfv(GL_LIGHT0, GL_AMBIENT_AND_DIFFUSE, AMBIENT);
   glLightfv(GL_LIGHT0, GL_POSITION, LIGHT_POSITION);
   initialize();
   glutMainLoop();
   return 0;
}
