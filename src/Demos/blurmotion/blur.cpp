#include <GL/glut.h>
#include <cmath>
#include <iostream>
#include <iomanip>
#include <sstream>
#include <windows.h>
using namespace std;

// Current window dimensions
int windowWidth = 1200;
int windowHeight = 800;

double xpos = 10;
double dir = 1;
double maxdist = 20;
int steps = 3;
double blurdist = -0.2;

// Material data
GLfloat white[] = { 1.0, 1.0, 1.0, 1.0 };
GLfloat blue[] = { 0.6, 0.6, 1.0, 1.0 };
GLfloat shiny[] = { 70 };
GLfloat lightdir[] = { 0.0, 1.0, 1.0, 0.0 };

void title()
{
   ostringstream os;
   os << "Blurring.  Steps = " << steps << ".  Distance = " << fixed << setprecision(3) << -blurdist;
   glutSetWindowTitle(os.str().c_str());
}

void display (void)
{
   glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
   glMatrixMode(GL_MODELVIEW);
   glLoadIdentity();
   glTranslated(xpos, 0, -50);
   for (int i = 0; i < steps; ++i)
   {
      glutSolidSphere(4, 25, 250);
      if (i == 0)
         glAccum(GL_LOAD, 0.9);
      else
         glAccum(GL_ACCUM, 0.05);
      glTranslated(blurdist * dir, 0, 0);
   }
   glAccum(GL_RETURN, 1);
   glutSwapBuffers();
}

double curTime = GetTickCount(); // milliseconds
double simTime = curTime;
int cycles;

void idle()
{
   double curTime = GetTickCount();
   while (simTime < curTime)
   {
      xpos += 0.1 * dir;
      if (fabs(xpos) > maxdist)
         dir = -dir;
      simTime += 3;
      glutPostRedisplay();
   }
}

void keyboard (unsigned char key, int x, int y)
{
   switch (key)
   {
      case 'f':
      glutFullScreen();
      break;

      case 'q':
      case 27:
         exit(0);
         break;
   }
   title();
}

void functionKeys (int key, int x, int y)
{
   switch (key)
   {
      case GLUT_KEY_UP:
         ++steps;
         break;
      case GLUT_KEY_DOWN:
         if (steps > 0) --steps;
         break;
      case GLUT_KEY_LEFT:
         blurdist /= 1.2;
         break;
      case GLUT_KEY_RIGHT:
         blurdist *= 1.2;
         break;
   }
   title();
}

void reshape (int w, int h)
{
   windowWidth = w;
   windowHeight = h;
   glViewport(0, 0, windowWidth, windowHeight);
   glMatrixMode(GL_PROJECTION);
   glLoadIdentity();
   gluPerspective(40, double(w)/double(h), 1, 100);
   glutPostRedisplay();
}

int main(int argc, char *argv[])
{
   cout << "Blur demonstration\n"
   " up    more blurring steps\n"
   " down  fewer blurring steps\n"
   " left  reduce blurring distance\n"
   " right increase blurring distance\n";

   glutInit(&argc, argv);
   glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGB | GLUT_DEPTH | GLUT_ACCUM);
   glutInitWindowSize(windowWidth, windowHeight);
   glutCreateWindow("Accumulation Buffer");
   glutDisplayFunc(display);
   glutIdleFunc(idle);
   glutKeyboardFunc(keyboard);
   glutSpecialFunc(functionKeys);
   glutReshapeFunc(reshape);
   glClearColor(0, 0, 0, 0);
   glEnable(GL_DEPTH_TEST);
   glEnable(GL_LIGHTING);
   glEnable(GL_LIGHT0);
   glLightfv(GL_LIGHT0, GL_POSITION, lightdir);
   glMaterialfv(GL_FRONT, GL_AMBIENT_AND_DIFFUSE, blue);
   glMaterialfv(GL_FRONT, GL_SPECULAR, white);
   glMaterialfv(GL_FRONT, GL_SHININESS, shiny);
   title();
   glutMainLoop();
   return 0;
}
