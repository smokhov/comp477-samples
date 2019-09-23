/** \file gimbal.cpp
*  Concordia University Graphics Library
*
* \authors Peter Grogono
* \date November 2005
*/

/**
* \addtogroup examples
* @{
* gimbal.cpp demonstrates the problem of "gimbal locking" that arises when Euler angles are used for rotation.
* @}
*/

#include <cstdlib>
#include <sstream>
#include <iomanip>
#include <io.h>
#include <GL/glut.h>
#include <GL/cugl.h>

using namespace std;
using namespace cugl;

// Initial size of graphics window.
const int WIDTH  = 600;
const int HEIGHT = 600;

// Current size of window.
int width  = WIDTH;
int height = HEIGHT;

// Bounds of viewing frustum.
GLfloat nearPlane = 10;
GLfloat farPlane  = 60;
GLfloat viewingDistance = -50;

// Viewing angle.
GLfloat fovy = 40;

// Lighting parameters.
GLfloat ambient[] = {0.4f, 0.4f, 0.4f, 1};
GLfloat position[] = {5, 20, 0, 1};

// Rotation angles (degrees)
int xr = 0;
int yr = 0;
int zr = 0;

// This function is called to display the scene.
void display ()
{
   glClearColor(0.3f, 0.3f, 1, 1);
   glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
   glMatrixMode(GL_MODELVIEW);
   glLoadIdentity();
   glTranslatef(0, 0, viewingDistance);

   glPushMatrix();
   glRotatef(xr, 1, 0, 0);
   glRotatef(yr, 0, 1, 0);
   glRotatef(zr, 0, 0, 1);
   buildPlane();
   glPopMatrix();

   glPushMatrix();
   glTranslatef(8, 6, -5);
   glRotatef(xr, 1, 0, 0);
   glRotatef(yr, 0, 1, 0);
   glRotatef(zr, 0, 0, 1);
   axes(3);
   glPopMatrix();

   glutSwapBuffers();

   ostringstream os;
   os << setprecision(0) <<
   setw(4) << xr <<
   setw(4) << yr <<
   setw(4) << zr;
   glutSetWindowTitle(os.str().c_str());
}

// Respond to window resizing, preserving proportions.
// Parameters give new window size in pixels.
void reshapeMainWindow (int newWidth, int newHeight)
{
   width = newWidth;
   height = newHeight;
   glViewport(0, 0, width, height);
   glMatrixMode(GL_PROJECTION);
   glLoadIdentity();
   gluPerspective(fovy, GLfloat(width) / GLfloat(height), nearPlane, farPlane);
}

void graphicKeys (unsigned char key, int x, int y)
{
   switch (key)
   {
      case 'f':
         glutFullScreen();
         glutPostRedisplay();
         break;
      case 'r':
         xr = 0;
         yr = 0;
         zr = 0;
         glutPostRedisplay();
         break;
      case 'x':
         xr += 30;
         glutPostRedisplay();
         break;
      case 'y':
         yr += 30;
         glutPostRedisplay();
         break;
      case 'z':
         zr += 30;
         glutPostRedisplay();
         break;
      case 27:
         exit(0);
   }
}

int main (int argc, char **argv)
{
   cout <<
        "This program demonstrates 'gimbal locking'.\n\n"
        "Keys have the following effect:\n"
        "  x  rotate 30 degrees about the X-axis.\n"
        "  y  rotate 30 degrees about the Y-axis.\n"
        "  z  rotate 30 degrees about the Z-axis.\n"
        "  r  set all rotations to 0 degrees.\n"
        "  f  full screen mode.\n\n"
        "Turn the plane through 90 degrees by pressing ryyy.\n"
        "In this state, the keys x and z have the same effect.\n\n"
        "The rotation angles are displayed at the top of the window."
        << endl;

   // GLUT initialization.
   glutInit(&argc, argv);
   glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGB);
   glutInitWindowSize(width, height);
   glutCreateWindow("Gimbal lock demonstration.");
   glEnable(GL_DEPTH_TEST);
   glEnable(GL_LIGHTING);
   glEnable(GL_LIGHT0);
   glLightfv(GL_LIGHT0, GL_AMBIENT, ambient);
   glLightfv(GL_LIGHT0, GL_POSITION, position);

   // Register call backs.
   glutDisplayFunc(display);
   glutReshapeFunc(reshapeMainWindow);
   glutKeyboardFunc(graphicKeys);

   // Enter GLUT loop.
   glutMainLoop();
   return 0;
}
