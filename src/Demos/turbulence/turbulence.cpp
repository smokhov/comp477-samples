/* Link with these libraries (omit those unneeded):
 *  libglut32
 *  libopengl32
 *  libglu32
 */

#include <cmath>
#include <cstdlib>
#include <iostream>
#include <iomanip>
#include <sstream>
#include <GL/glut.h>

using namespace std;

const double PI = 4 * atan(1);
const int GRID_SIZE = 1000;
double noise[GRID_SIZE][GRID_SIZE];

double randReal()
{
   return double(rand()) / double(RAND_MAX);
}

void makeNoise()
{
   for (int i = 0; i < GRID_SIZE; ++i)
      for (int j = 0; j < GRID_SIZE; ++j)
         noise[i][j] = randReal();
}

double interpolate(double u, double v)
{
   int iu0 = u;
   double du = u - iu0;
   iu0 %= GRID_SIZE;
   int iu1 = (iu0 + 1) % GRID_SIZE;

   int iv0 = v;
   double dv = v - iv0;
   iv0 %= GRID_SIZE;
   int iv1 = (iv0 + 1) % GRID_SIZE;

   double bot = noise[iu0][iv0] + du * (noise[iu1][iv0] - noise[iu0][iv0]);
   double top = noise[iu0][iv1] + du * (noise[iu1][iv1] - noise[iu0][iv1]);
   return bot + dv * (top - bot);
}

double turbulence(double u, double v)
{
   double t = 0;
   double s = 512;
   while (s > 1)
   {
      t += s * interpolate(u/s, v/s);
      s /= 2;
   }
   return t;
}

int windowWidth = 600;
int windowHeight = 600;

// Current mouse positions
GLdouble mx; // 0 <= mx <= windowWidth
GLdouble my; // 0 <= my <= windowHeight

// Control from idle function
double xi = 0;

void display (void)
{
   glClear(GL_COLOR_BUFFER_BIT);
   glMatrixMode(GL_MODELVIEW);
   glLoadIdentity();

   glBegin(GL_POINTS);
   for (int i = 0; i < windowWidth; ++i)
      for (int j = 0; j < windowHeight; ++j)
      {
         glTranslated(i, j, 0);
         double r = 0.2 + 0.8 * sin((4 * PI * i) / windowWidth + 0.5 * xi * turbulence(i, j));
         glColor3f(r, r, 0);
         glVertex2i(i, j);
      }
   glEnd();
   glutSwapBuffers();

   ostringstream os;
   os << fixed << setprecision(3);
   os << "Turbulence factor: " << xi;
   glutSetWindowTitle(os.str().c_str());
}

void idle()
{
   static double angle = 0;
   angle += 0.02;
   if (angle > 1 * PI)
      angle -= 2 * PI;
   xi = cos(angle);
   glutPostRedisplay();
}

void mouse(int x, int y)
{
   mx = double(x) / windowWidth;
   my = 1.0 - double(y) / windowHeight;
   glutPostRedisplay();
}

void keyboard (unsigned char key, int x, int y)
{
   switch (key)
   {
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
   gluOrtho2D(0, windowWidth, 0, windowHeight);
   glutPostRedisplay();
}

int main(int argc, char *argv[])
{
   makeNoise();
   glutInit(&argc, argv);
   glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGB);
   glutInitWindowSize(windowWidth, windowHeight);
   glutCreateWindow("Turbulence");
   glutDisplayFunc(display);
   glutMotionFunc(mouse);
   glutIdleFunc(idle);
   glutKeyboardFunc(keyboard);
   glutReshapeFunc(reshape);
   glEnable(GL_POINT_SMOOTH);
   glPointSize(3);
   glutMainLoop();
   return 0;
}
