/* Libraries
    libcugl
    libglut32
    libopeng32
    libglu32
*/

#include <cassert>
#include <cmath>
#include <iostream>
#include <iomanip>
#include <sstream>
#include <vector>
#include "cugl.h"
#include <GL/glut.h>

using namespace std;
using namespace cugl;

// Current window dimensions
int windowWidth = 800;
int windowHeight = 600;

// GLUT fonts
void* font1 = GLUT_BITMAP_TIMES_ROMAN_24;
void* font2 = GLUT_BITMAP_9_BY_15;

// Global variables
double a1 = 0.1 * PI; // start angle
double a2 = 2.5 * PI; // finish angle
vector<Point> points; // points to plot
double criterion = 1; // curvature criterion for bisection
string message;       // report current values

// Display 'message' at (x, y).
void showString (GLfloat x, GLfloat y)
{
   glRasterPos2f(x, y);
   size_t len = message.size();
   for (size_t i = 0; i < len; i++)
      glutBitmapCharacter(font1, message[i]);
}

void display (void)
{
   glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
   glMatrixMode(GL_MODELVIEW);
   glLoadIdentity();

   // Display the desired curve
   glColor3d(0, 0, 1);
   glBegin(GL_LINE_STRIP);
   for (int i = 0; i < 100; ++i)
   {
      double x = a1 + i * 0.01 * (a2 - a1);
      double y = cos(x);
      glVertex3d(x, y, 0);
   }
   glEnd();

   // Display the selected points on the curve
   glColor3d(1, 1, 1);
   glBegin(GL_POINTS);
   for (vector<Point>::const_iterator it = points.begin(); it < points.end(); ++it)
      it->draw();
   glEnd();

   // Display current values
   showString(0.5 * (a1 + a2), -1);
   glutSwapBuffers();
}

// Construct points from 'here' to 'there'
vector<Point> adapt(const Point & here, const Point & there)
{
   vector<Point> result;
   double curvature = -cos(here[0]);
   double distance = dist(here, there);
   if (abs(curvature * distance) < criterion)
   {
      // Curvature is small: return end points
      result.push_back(here);
      result.push_back(there);
   }
   else
   {
      // Curvature is large: create a midpoint and recurse
      double x = 0.5*(here[0] + there[0]);
      double y = cos(x);
      Point mid = Point(x, y, 0);
      result = adapt(here, mid);
      vector<Point> ps = adapt(mid, there);
      for (vector<Point>::const_iterator it = ps.begin(); it < ps.end(); ++it)
         result.push_back(*it);
   }
   return result;
}

// Plot points and format report
void update()
{
   points = adapt(Point(a1, cos(a1), 0), Point(a2, cos(a2), 0));
   ostringstream os;
   os << "Min C * D = " << fixed << setprecision(4) << criterion << "    Points = " << points.size();
   message = os.str();
   glutPostRedisplay();
}

void keyboard (unsigned char key, int x, int y)
{
   switch (key)
   {
      case '+':
         criterion *= 0.8;
         update();
         break;
      case '-':
         criterion /= 0.8;
         update();
         break;
      case 'f':
         glutFullScreen();
         break;
      case 'q':
      case 27:
         exit(0);
         break;
   }
}

void reshape (int w, int h)
{
   cout <<
   "Adaptive bisection\n\n"
   "+      Increase points\n"
   "-      Decrease points\n"
   "f      Full screen\n"
   "SC, q  Quit\n";

   windowWidth = w;
   windowHeight = h;
   glViewport(0, 0, windowWidth, windowHeight);
   glMatrixMode(GL_PROJECTION);
   glLoadIdentity();
   gluOrtho2D(a1 - 0.1, a2 + 0.1, -1.1, 1.1);
   glutPostRedisplay();
}

int main(int argc, char *argv[])
{
   glutInit(&argc, argv);
   glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGB | GLUT_DEPTH);
   glutInitWindowSize(windowWidth, windowHeight);
   glutInitWindowPosition(300,0);
   glutCreateWindow("Adaptation");
   glutDisplayFunc(display);
   glutKeyboardFunc(keyboard);
   glutReshapeFunc(reshape);
   glEnable(GL_DEPTH_TEST);
   glPointSize(4);
   update();
   glutMainLoop();
   return 0;
}
