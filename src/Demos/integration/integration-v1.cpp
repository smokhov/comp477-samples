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

// Current mouse positions
GLdouble mx; // 0 <= mx <= windowWidth
GLdouble my; // 0 <= my <= windowHeight

// Material data
GLfloat white[] = { 1.0, 1.0, 1.0, 1.0 };
GLfloat blue[] = { 0.4, 0.4, 1.0, 1.0 };
GLfloat shiny[] = { 30.0 };
GLfloat dir[] = { 0.0, 0.0, 1.0, 0.0 };

char mode = '1';

// All modes
double dt = 0.001;

// Mode  = '1'
double tim = 0;

// Mode > '1'
Vector acc, vel, pos, prevPos, prevVel;

void* font1 = GLUT_BITMAP_TIMES_ROMAN_24;
void* font2 = GLUT_BITMAP_9_BY_15;
string title = "";

// Display 'title' as a bit-mapped character string.
void showString (GLfloat x, GLfloat y)
{
   glRasterPos2f(x, y);
   size_t len = title.size();
   for (size_t i = 0; i < len; i++)
      glutBitmapCharacter(font1, title[i]);
}

void display (void)
{
   glMatrixMode(GL_PROJECTION);
   glLoadIdentity();
   gluPerspective(40.0, double(windowWidth)/double(windowHeight), 1.0, 20.0);
   glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
   glMatrixMode(GL_MODELVIEW);
   glLoadIdentity();
   glTranslated(0.0, 0.0, -5);

   glDisable(GL_LIGHTING);
   glColor3d(1, 1, 0);
   const int NUMP = 100;
   glBegin(GL_LINE_LOOP);
   for (int i = 0; i < NUMP; ++i)
   {
      double a = (2 * PI * i) / NUMP;
      glVertex2d(cos(a), sin(a));
   }
   glEnd();

   glEnable(GL_LIGHTING);
   glMaterialfv(GL_FRONT, GL_AMBIENT_AND_DIFFUSE, blue);
   glMaterialfv(GL_FRONT, GL_SPECULAR, white);
   glMaterialfv(GL_FRONT, GL_SHININESS, shiny);

   pos.translate();
   glutSolidSphere(0.05, 10, 10);

   glMatrixMode(GL_PROJECTION);
   glLoadIdentity();
   glOrtho(0, 3, 0, 1, -1, 1);
   glMatrixMode(GL_MODELVIEW);
   glLoadIdentity();
   glDisable(GL_LIGHTING);
   glColor3d(1, 1, 1);
   showString(0.2, 0.1);
   glutSwapBuffers();
}

Vector midpoint(const Vector & p, Vector f(const Vector &))
{
   Vector a1(f(p));
   Vector a2(f(p + 0.5 * dt * a1));
   return dt * a2;
}

Vector facc(const Vector & p)
{
   double r = pow(sqr(pos[0]) + sqr(pos[1]), 1.5);
   return Vector(-pos[0]/r, -pos[1]/r, 0);
}

void idle()
{
   ostringstream os;
   switch (mode)
   {
      case '1':
         os << "Analytic";
         tim += dt;
         if (tim > 2 * PI)
            tim -= 2 * PI;
         pos = Vector(cos(tim), sin(tim), 0);
         break;

      case '2':
         os << "First-order Euler integration";
         acc = facc(pos);
         vel += dt * acc;
         pos += dt * vel;
         break;

      case '3':
         {
            os << "Verlet integration";
            acc = facc(pos);
            Vector op(pos);
            pos = 2 * pos - prevPos + dt * dt * acc;
            prevPos = op;
         }
         break;

      case '4':
         {
            os << "Heun integration";
            acc = facc(pos);
            vel += dt * acc;
            pos += dt * 0.5 * (vel + prevVel);
            prevVel = vel;
         }
         break;

   }
   glutPostRedisplay();
   os << fixed << setprecision(4) << " with DT = " << dt << "   ";
   title = os.str();
}

void mouse(int x, int y)
{
   mx = double(x) / double(windowWidth);
   my = 1.0 - double(y) / double(windowHeight);
   glutPostRedisplay();
}

void initialize()
{
   tim = 0;
   vel = Vector(0, 1, 0);
   prevVel = vel;
   pos = Vector(1, 0, 0);
   prevPos = Vector(cos(-dt), sin(-dt), 0);
}

void keyboard (unsigned char key, int x, int y)
{
   switch (key)
   {
      case '1':
      case '2':
      case '3':
      case '4':
         initialize();
         mode = key;
         break;

      case '+':
         dt *= 1.5;
         break;

      case '-':
         dt /= 1.5;
         break;

      case 'r':
         initialize();
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
   "Simple integration techniques\n\n"
   " 1   Analytic\n"
   " 2   Euler\n"
   " 3   Verlet\n"
   " 4   Heun\n"
   " +   increase DT\n"
   " -   decrease DT\n"
   " r   reset\n"
   " f   full screen\n"
   " q   quit\n"
   " ESC quit\n";
   glutInit(&argc, argv);
   glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGB | GLUT_DEPTH);
   glutInitWindowSize(windowWidth, windowHeight);
   glutInitWindowPosition(300, 300);
   glutCreateWindow("Simple integration techniques");
   glutDisplayFunc(display);
   glutMotionFunc(mouse);
   glutKeyboardFunc(keyboard);
   glutReshapeFunc(reshape);
   glutIdleFunc(idle);
   glEnable(GL_DEPTH_TEST);
   glEnable(GL_LIGHTING);
   glEnable(GL_LIGHT0);
   glLightfv(GL_LIGHT0, GL_POSITION, dir);
   initialize();
   glutMainLoop();
   return 0;
}
