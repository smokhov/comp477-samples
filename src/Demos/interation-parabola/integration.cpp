/* Link with these libraries (omit those unneeded):
 *  libcugl
 *  libglut32
 *  libopengl32
 *  libglu32
 */

#include <iostream>
#include <iomanip>
#include <sstream>
#include <GL/cugl.h>
#include <GL/glut.h>
#include <windows.h>

using namespace std;
using namespace cugl;

// Current window dimensions
int windowWidth = 800;
int windowHeight = 800;

string message = "";

// Display 'message' as a bit-mapped character string.
void displayMessage (GLfloat x, GLfloat y)
{
   glRasterPos2f(x, y);
   size_t len = message.size();
   for (size_t i = 0; i < len; i++)
      glutBitmapCharacter(GLUT_BITMAP_TIMES_ROMAN_24, message[i]);
}

const double DT = 0.01;
const double HH = 5;
const double RAD = 5;
const double GRAV = 2000;
const double DT_FACTOR = 1.1;

double dt = DT;
bool euler = false;
bool heun = false;
bool verlet = false;
bool midpoint = false;
bool gravity = true;
enum Mode { PARABOLA, ORBIT } mode = PARABOLA;

void display (void)
{
   glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
   glMatrixMode(GL_MODELVIEW);
   glLoadIdentity();

   switch (mode)
   {
      case PARABOLA:
         {
            // Analytic
            const int STEPS = 100;
            glColor3d(0, 0.6, 0);
            glBegin(GL_LINE_STRIP);
            for (int i = 0; i <= STEPS; ++i)
            {
               double t = i * (4.0 / STEPS);
               double x = 2 * t - 4;
               double y = 10 * t - 2.5 * sqr(t);
               glVertex2d(x, y);
            }
            glEnd();

            glBegin(GL_POINTS);

            if (euler)
            {
               // First-order Euler
               glColor3d(0, 0, 0.5);
               Vector a(0, -5, 0);
               Vector v(2, 10, 0);
               Vector x(-4, 0, 0);
               glVertex2d(x[0], x[1]);
               do
               {
                  v += dt * a;
                  x += dt * v;
                  glVertex2d(x[0], x[1]);
               }
               while (x[1] > 0);
            }

            if (heun)
            {
               glColor3d(0.5, 0, 0);
               Vector a(0, -5, 0);
               Vector v(2, 10, 0);
               Vector x(-4, 0, 0);
               Vector oldV = v;
               glVertex2d(x[0], x[1]);
               do
               {
                  v += dt * a;
                  x += dt * 0.5 * (v + oldV);
                  oldV = v;
                  glVertex2d(x[0], x[1]);
               }
               while (x[1] > 0);
            }

            if (midpoint)
            {
               glColor3d(0.5, 0.5, 0);
               Vector a(0, -5, 0);
               Vector v(2, 10, 0);
               Vector x(-4, 0, 0);
               glVertex2d(x[0], x[1]);
               do
               {
                  Vector tv = dt * a;
                  x += dt * (v + 0.5 * tv);
                  v += tv;
                  glVertex2d(x[0], x[1]);
               }
               while (x[1] > 0);
            }

            if (verlet)
            {
               glColor3d(0, 0.5, 0.5);
               Vector a(0, -5, 0);
               Vector v(2, 10, 0);
               Vector x(-4, 0, 0);
               Vector x0 = x;
               Vector x1 = x + dt * v;
               glVertex2d(x[0], x[1]);
               do
               {
                  x = 2 * x1 - x0 + sqr(dt) * a;
                  glVertex2d(x[0], x[1]);
                  x0 = x1;
                  x1 = x;
               }
               while (x[1] > 0);
            }
            glEnd();
            break;
         }

      case ORBIT:
         {
            // Analytic
            const int STEPS = 100;
            glColor3d(0, 0.6, 0);
            glBegin(GL_LINE_STRIP);
            for (int i = 0; i <= STEPS; ++i)
            {
               double angle = (2 * PI * i) / STEPS;
               double x = RAD * cos(angle);
               double y = HH + RAD * sin(angle);
               glVertex2d(x, y);
            }
            glEnd();

            glBegin(GL_POINTS);

            if (euler)
            {
               glColor3d(0, 0, 0.5);
               Vector a;
               Vector v(20, 0, 0);
               Vector x(0, RAD, 0);
               double t = 0;
               glVertex2d(x[0], x[1] + HH);
               do
               {
                  a = gravity ? - (GRAV / sqr(x.length())) * x.unit()
                              : - (GRAV / sqr(RAD)) * x.unit();
                  v += dt * a;
                  x += dt * v;
                  t += dt;
                  glVertex2d(x[0], x[1] + HH);
               }
               while (t < 5);
            }

            if (heun)
            {
               glColor3d(0.5, 0, 0);
               Vector a;
               Vector v(20, 0, 0);
               Vector oldV = v;
               Vector x(0, RAD, 0);
               double t = 0;
               glVertex2d(x[0], x[1] + HH);
               do
               {
                  a = gravity ? - (GRAV / sqr(x.length())) * x.unit()
                              : - (GRAV / sqr(RAD)) * x.unit();
                  v += dt * a;
                  x += dt * 0.5 * (v + oldV);
                  oldV = v;
                  t += dt;
                  glVertex2d(x[0], x[1] + HH);
               }
               while (t < 5);
            }

            if (midpoint)
            {
               glColor3d(0.5, 0.5, 0);
               Vector a;
               Vector v(20, 0, 0);
               Vector x(0, RAD, 0);
               double t = 0;
               glVertex2d(x[0], x[1] + HH);
               do
               {
                  a = gravity ? - (GRAV / sqr(x.length())) * x.unit()
                              : - (GRAV / sqr(RAD)) * x.unit();
                  Vector tv = dt * a;
                  x += dt * (v + 0.5 * tv);
                  v += tv;
                  t += dt;
                  glVertex2d(x[0], x[1] + HH);
               }
               while (t < 5);
            }

            if (verlet)
            {
               glColor3d(0, 0.5, 0.5);
               Vector a;
               Vector v(20, 0, 0);
               Vector x(0, RAD, 0);
               Vector x0 = x;
               Vector x1 = x + dt * v;
               double t = 0;
               glVertex2d(x[0], x[1] + HH);
               do
               {
                  a = gravity ? - (GRAV / sqr(x.length())) * x.unit()
                              : - (GRAV / sqr(RAD)) * x.unit();
                  x = 2 * x1 - x0 + sqr(dt) * a;
                  glVertex2d(x[0], x[1] + HH);
                  x0 = x1;
                  x1 = x;
                  t += dt;
               }
               while (t < 5);
            }
            glEnd();
            break;
         }
   }

   glColor3d(0, 0, 0);
   displayMessage(-5, -0.5);
   glutSwapBuffers();
}

void updateMessage()
{
   ostringstream os;
   if (euler) os << "Euler ";
   if (heun) os << "Heun ";
   if (midpoint) os << "Midpoint ";
   if (verlet) os << "Verlet ";
   if (mode == ORBIT) os << (gravity ? "(gravity) " : "(constant force) ");
   os << "   dt = " << fixed << setprecision(4) << dt;
   message = os.str();
   glutPostRedisplay();
}

void reset()
{
   euler = false;
   heun = false;
   verlet = false;
   midpoint = false;
   dt = DT;
}

void keyboard (unsigned char key, int x, int y)
{
   switch (key)
   {

      case 'e':
         euler = ! euler;
         dt = DT;
         break;

      case 'f':
         glutFullScreen();
         break;

      case 'h':
         heun = ! heun;
         dt = DT;
         break;

      case 'g':
         gravity = ! gravity;
         break;

      case 'm':
         midpoint = ! midpoint;
         dt = DT;
         break;

      case 'o':
         mode = ORBIT;
         break;

      case 'p':
         mode = PARABOLA;
         break;

      case 'r':
         reset();
         break;

      case 'v':
         verlet = ! verlet;
         break;

      case '+':
         dt *= DT_FACTOR;
         break;

      case '-':
         dt /= DT_FACTOR;
         break;

      case 'q':
      case 27:
         exit(0);
         break;

   }
   updateMessage();
}

void reshape (int w, int h)
{
   windowWidth = w;
   windowHeight = h;
   glViewport(0, 0, windowWidth, windowHeight);
   glMatrixMode(GL_PROJECTION);
   glLoadIdentity();
   gluOrtho2D(-6, 6, -1, 11);
   glutPostRedisplay();
}

int main(int argc, char *argv[])
{
   cout <<
        "Simple integration techniques (e, h, m, v)\n\n"
        " o   orbit (central attractive force)\n"
        " p   parabola (constant downward force)\n\n"
        " e   toggle Euler\n"
        " g   gravity or constant force\n"
        " h   toggle Heun\n"
        " m   toggle midpoint\n"
        " v   toggle Verlet\n"
        " r   all off\n\n"
        " +   increase DT\n"
        " -   decrease DT\n\n"
        " f   full screen\n"
        " q   quit\n"
        " ESC quit\n";
   glutInit(&argc, argv);
   glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGB | GLUT_DEPTH);
   glutInitWindowSize(windowWidth, windowHeight);
   glutInitWindowPosition(300, 100);
   glutCreateWindow("Simple integration techniques");
   glutDisplayFunc(display);
   glutKeyboardFunc(keyboard);
   glutReshapeFunc(reshape);
   glClearColor(1, 1, 0.8, 1);
   glPointSize(3);
   updateMessage();
   glutMainLoop();
   return 0;
}
