/* Link with these libraries (omit those unneeded):
 *  libcugl
 *  libglut32
 *  libopengl32
 *  libglu32
 */

#include <iostream>
#include <iomanip>
#include <sstream>
#include <cassert>
#include <GL/glut.h>
#include "cugl.h"
#include <windows.h>

using namespace std;
using namespace cugl;

// Current window dimensions
int windowWidth = 1200;
int windowHeight = 800;

// Current mouse positions
GLdouble mx; // 0 <= mx <= 1
GLdouble my; // 0 <= my <= 1

GLfloat ambient[] = {0.4f, 0.4f, 0.4f, 1};
GLfloat position[] = {5, 20, 0, 1};

const int NUMPOINTS = 1000;
double totalTime = 10000; // milliseconds
double startTime = GetTickCount();
int oldCurrPos = 0;

bool showAxes = true;
bool showModel = false;

void* font1 = GLUT_BITMAP_TIMES_ROMAN_24;
void* font2 = GLUT_BITMAP_9_BY_15;
string title = "Fixed frame";

// Display 'title' as a bit-mapped character string.
void showString (GLfloat x, GLfloat y)
{
   glRasterPos2f(x, y);
   size_t len = title.size();
   for (size_t i = 0; i < len; i++)
      glutBitmapCharacter(font1, title[i]);
}

Point pos(double t)
{
   return Point(0.2 * t, cos(t), sin(t));
}

Vector posVec(double t)
{
   return Vector(0.2 * t, cos(t), sin(t));
}

Vector velVec(double t)
{
   return Vector(0.2, -sin(t), cos(t));
}

Vector accVel(double t)
{
   return Vector(0, -cos(t), -sin(t));
}

void drawFrame(Vector x, Vector y, Vector z)
{
   Matrix ori(
      x.unit()[0], y.unit()[0], z.unit()[0], 0,
      x.unit()[1], y.unit()[1], z.unit()[1], 0,
      x.unit()[2], y.unit()[2], z.unit()[2], 0,
      0, 0, 0, 1);
   ori.apply();
   glEnable(GL_LIGHTING);
   if (showAxes)
   {
      axes(0.2);
   }
   if (showModel)
   {
      glRotated(90, 0, -1, 0);
      glRotated(90, 0, 0, -1);
      glScaled(0.05, 0.05, 0.05);
      buildPlane();
   }
}

// Globals used by idle function
int currPos = -NUMPOINTS;
char mode = '1';

void display (void)
{
   glMatrixMode(GL_PROJECTION);
   glLoadIdentity();
   gluPerspective(40.0, double(windowWidth)/double(windowHeight), 1.0, 20.0);
   glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
   glMatrixMode(GL_MODELVIEW);
   glLoadIdentity();
   glTranslated(0, 0, -6);
   glRotated(180 * mx, 0, 1, 0);

   glEnable(GL_LIGHTING);
   axes(0.2);

   glDisable(GL_LIGHTING);
   glColor3d(1, 1, 1);
   glBegin(GL_LINE_STRIP);
   for (int i = -NUMPOINTS; i <= NUMPOINTS; ++i)
   {
      double t = (4 * PI * i) / NUMPOINTS;
      Point p = pos(t);
      p.draw();
   }
   glEnd();

   double tp = (4 * PI * currPos) / NUMPOINTS;
   Point p = pos(tp);
   p.translate();

//   Draw velocity vector
//   velVec(tp).draw();

   switch (mode)
   {
      default:
      case '1':
         drawFrame(I, J, K);
         break;

      case '2':
         {
            Vector t = velVec(tp);
            Vector b = (t * accVel(tp));
            Vector n = (b * t);
            drawFrame(t, b, n);
         }
         break;

      case '3':
         {
            Vector t = velVec(tp);
            Vector b = (t * J);
            Vector n = (b * t);
            drawFrame(t, b, n);
         }
         break;

      case '4':
         {
            Vector t = Point() - p;
            Vector b = (t * J);
            Vector n = (b * t);
            drawFrame(t, b, n);
         }
         break;

      case '5':
         {
            Vector t = velVec(tp).unit();
            Quaternion q(t, I);
            drawFrame(q.apply(I), q.apply(J), q.apply(K));
         }
         break;
   }

   glMatrixMode(GL_PROJECTION);
   glLoadIdentity();
   glOrtho(0, 3, 0, 1, -1, 1);
   glMatrixMode(GL_MODELVIEW);
   glLoadIdentity();
   glDisable(GL_LIGHTING);
   glColor3d(1, 1, 1);
   showString(0.5, 0.1);
   glutSwapBuffers();
}

void idle()
{
   double pathTime = GetTickCount() - startTime;
   if (pathTime > totalTime)
   {
      startTime = GetTickCount();
      pathTime = 0;
   }
   double r = pathTime / totalTime;
   currPos = NUMPOINTS * (2 * r - 1);
   if (currPos != oldCurrPos)
   {
      oldCurrPos = currPos;
      glutPostRedisplay();
   }
}

void mouse(int x, int y)
{
   mx = double(x) / double(windowWidth);
   my = 1.0 - double(y) / double(windowHeight);
   glutPostRedisplay();
}

void keyboard (unsigned char key, int x, int y)
{
   switch (key)
   {
      case '1':
         title = "Fixed frame";
         mode = key;
         break;

      case '2':
         title = "Frenet frame";
         mode = key;
         break;

      case '3':
         title = "Using UP vector";
         mode = key;
         break;

      case '4':
         title = "Looking towards origin";
         mode = key;
         break;

      case '5':
         title = "Using quaternion";
         mode = key;
         break;

      case 'a':
         showAxes = ! showAxes;
         break;

      case 'f':
         glutFullScreen();
         break;

      case 'm':
         showModel = ! showModel;
         break;

      case 'r':
         startTime = GetTickCount();
         break;

      case '+':
         totalTime /= 1.1;
         break;

      case '-':
         totalTime *= 1.1;
         break;

      case 27:
         exit(0);
         break;
   }
}

void reshape (int w, int h)
{
   windowWidth = w;
   windowHeight = h;
   glViewport(0, 0, windowWidth, windowHeight);
   glMatrixMode(GL_PROJECTION);
   glLoadIdentity();
   gluPerspective(40.0, double(w)/double(h), 1.0, 20.0);
   glutPostRedisplay();
}

int main(int argc, char *argv[])
{
   cout <<
        "Curve following demonstration.\n\n"
        "Select mode:\n"
        "  1  Fixed frame.\n"
        "  2  Frenet frame.\n"
        "  3  UP vector.\n"
        "  4  Looking towards origin.\n"
        "  5  Quaternion.\n\n"
        "Speed:\n"
        "  +  Faster\n"
        "  -  Slower\n\n"
        "Show:\n"
        "  a  Axes.\n"
        "  m  Plane.\n"
        "  r  Reset.\n";

   glutInit(&argc, argv);
   glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGB | GLUT_DEPTH);
   glutInitWindowSize(windowWidth, windowHeight);
   glutInitWindowPosition(300, 300);
   glutCreateWindow("Path following");
   glutDisplayFunc(display);
   glutMotionFunc(mouse);
   glutKeyboardFunc(keyboard);
   glutReshapeFunc(reshape);
   glutIdleFunc(idle);
   glEnable(GL_DEPTH_TEST);
   glEnable(GL_NORMALIZE);
   glEnable(GL_LIGHT0);
   glLightfv(GL_LIGHT0, GL_AMBIENT, ambient);
   glLightfv(GL_LIGHT0, GL_POSITION, position);
   glutMainLoop();
   return 0;
}
