/* Link with these libraries (omit those unneeded):
 *  libcugl
 *  libglut32
 *  libopengl32
 *  libglu32
 */

#include <iostream>
#include <iomanip>
#include <sstream>
#include "cugl.h"
#include <GL/glut.h>
#include <windows.h>

using namespace std;
using namespace cugl;

// Current window dimensions
int windowWidth = 800;
int windowHeight = 800;

// Material data
GLfloat white[] = { 1.0, 1.0, 1.0, 1.0 };
GLfloat blue[] = { 0.4, 0.4, 1.0, 1.0 };
GLfloat shiny[] = { 30.0 };
GLfloat dir[] = { 0.0, 0.0, 1.0, 0.0 };

const double INIT_DT = 0.01;
const double FACTOR_DT = 1.5;

char mode = '1';
double dt = INIT_DT;
double angle = 0;
bool showVector = false;

Vector acc, vel, pos, prevPos, prevVel;

void* font1 = GLUT_BITMAP_TIMES_ROMAN_24;
void* font2 = GLUT_BITMAP_9_BY_15;
string message = "";

// Display 'message' as a bit-mapped character string.
void displayMessage (GLfloat x, GLfloat y)
{
   glRasterPos2f(x, y);
   size_t len = message.size();
   for (size_t i = 0; i < len; i++)
      glutBitmapCharacter(font1, message[i]);
}

// Return acceleration at current position, assuming speed = 1.
Vector findAcceleration()
{
   double r = pow(sqr(pos[0]) + sqr(pos[1]), 1.5);
   return Vector(-pos[0]/r, -pos[1]/r, 0);
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

   // Display circle.
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

   // Display mioving ball.
   glEnable(GL_LIGHTING);
   glMaterialfv(GL_FRONT, GL_AMBIENT_AND_DIFFUSE, blue);
   glMaterialfv(GL_FRONT, GL_SPECULAR, white);
   glMaterialfv(GL_FRONT, GL_SHININESS, shiny);
   pos.translate();
   glutSolidSphere(0.05, 10, 10);
   glDisable(GL_LIGHTING);

   // Display acceleration vector and, if mode is not analytic or Verlet, velocity vector.
   if (showVector)
   {
      glColor3d(0, 1, 0);
      findAcceleration().draw();
      if (mode == '2' || mode == '4' || mode == '5')
      {
         glColor3d(0, 0, 1);
         vel.draw();
      }
   }

   // Display message.
   glMatrixMode(GL_PROJECTION);
   glLoadIdentity();
   glOrtho(0, 3, 0, 1, -1, 1);
   glMatrixMode(GL_MODELVIEW);
   glLoadIdentity();
   glColor3d(1, 1, 1);
   displayMessage(0.2, 0.1);

   glutSwapBuffers();
}

double realTime = GetTickCount(); // milliseconds
double pathTime = realTime;
double timeStep = 5; // milliseconds

void idle ()
{
   realTime = GetTickCount();
   while (pathTime < realTime)
   {
      // Compute angle for analytic mode only.
      angle += dt;
      if (angle > 2 * PI)
         angle -= 2 * PI;
      // Get acceleration for other modes.
      acc = findAcceleration();

      switch (mode)
      {
            // Analytic
         case '1':
            pos = Vector(cos(angle), sin(angle), 0);
            break;

            // Euler
         case '2':
            vel += dt * acc;
            pos += dt * vel;
            break;

            // Verlet
         case '3':
            {
               Vector savePos(pos);
               pos = 2 * pos - prevPos + dt * dt * acc;
               prevPos = savePos;
            }
            break;

            // Heun
         case '4':
            {
               vel += dt * acc;
               pos += dt * 0.5 * (vel + prevVel);
               prevVel = vel;
            }
            break;

            // Midpoint
         case '5':
            {
               Vector tempVel = dt * acc;
               pos += dt * (vel + 0.5 * tempVel);
               vel += tempVel;
            }
            break;
      }
      glutPostRedisplay();
      pathTime += timeStep;
   }
}

void updateMessage()
{
   ostringstream os;
   switch (mode)
   {
      case '1':
         os << "1 Analytic";
         break;
      case '2':
         os << "2 Euler integration";
         break;
      case '3':
         os << "3 Verlet integration";
         break;
      case '4':
         os << "4 Heun integration";
         break;
      case '5':
         os << "5 Midpoint integration";
         break;
   }
   os << fixed << ".  DT = " << setprecision(4) << dt;
   message = os.str();
}

void initialize()
{
   angle = 0;
   vel = Vector(0, 1, 0);
   prevVel = vel;
   pos = Vector(1, 0, 0);
   prevPos = Vector(cos(-dt), sin(-dt), 0);
   updateMessage();
}

void keyboard (unsigned char key, int x, int y)
{
   switch (key)
   {
      case '1':
      case '2':
      case '3':
      case '4':
      case '5':
         mode = key;
         dt = INIT_DT;
         initialize();
         break;

      case '+':
         dt *= FACTOR_DT;
         initialize();
         break;

      case '-':
         dt /= FACTOR_DT;
         initialize();
         break;

      case 'f':
         glutFullScreen();
         break;

      case 'r':
         initialize();
         break;

      case 'v':
         showVector = !showVector;
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
        " 5   Midpoint\n"
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
//   glutMotionFunc(mouse);
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
