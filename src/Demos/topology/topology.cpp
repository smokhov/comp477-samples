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
#include <GL/cugl.h>
#include <GL/glut.h>

using namespace std;
using namespace cugl;

// Current window dimensions
int windowWidth = 800;
int windowHeight = 600;

// Randomized matrix to distort teapot
Matrix mat;

void display (void)
{
   glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
   glMatrixMode(GL_MODELVIEW);
   glLoadIdentity();
   glTranslated(0, 0, -10);
   mat.apply();
   glutWireTeapot(1);
   glutSwapBuffers();
}

// Randomize, run 500 cycles, and repeat.
int cycles = 0;
void idle ()
{
   static int i, j;
   static bool bigger;
   if (cycles == 0)
   {
      i = randInt(3);
      j = randInt(3);
      bigger = randReal() > 0.5;
      cycles = 500;
   }
   mat(i, j) = bigger ? mat(i, j) + 0.01 * randReal() : mat(i, j) - 0.01 * randReal();
   --cycles;
   glutPostRedisplay();
}

void keyboard (unsigned char key, int x, int y)
{
   switch (key)
   {
      case 'r':
         mat = Matrix();
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
   gluPerspective(40, GLfloat(w) / GLfloat(h), 1, 20);
}

int main(int argc, char *argv[])
{
   cout <<
   "Distorting the teapot without changing topology\n\n"
   " r    reset to original shape\n"
   " f    full screen\n"
   " q    quit\n"
   " ESC  quit\n";

   glutInit(&argc, argv);
   glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGB | GLUT_DEPTH);
   glutInitWindowSize(windowWidth, windowHeight);
   glutInitWindowPosition(300,0);
   glutCreateWindow("Adaptation");
   glutDisplayFunc(display);
   glutKeyboardFunc(keyboard);
   glutReshapeFunc(reshape);
   glutIdleFunc(idle);
   glutMainLoop();
   return 0;
}
