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
int windowWidth = 600;
int windowHeight = 600;

// Material data
GLfloat white[] = { 1.0, 1.0, 1.0, 1.0 };
GLfloat blue[] = { 0.3, 0.3, 0.9, 1.0 };
GLfloat shiny[] = { 30.0 };
GLfloat dir[] = { -0.3, 1.5, 1.0, 0.0 };

double r = 0;

void display (void)
{
   glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
   glMatrixMode(GL_MODELVIEW);
   glLoadIdentity();
   glLightfv(GL_LIGHT0, GL_POSITION, dir);
   glMaterialfv(GL_FRONT, GL_AMBIENT_AND_DIFFUSE, blue);
   glMaterialfv(GL_FRONT, GL_SPECULAR, white);
   glMaterialfv(GL_FRONT, GL_SHININESS, shiny);

   glTranslated(0, 0, -2.5);
   glColor3d(0,0,0);

   glPushMatrix();
   glRotated(90,1,1,0);
   Matrix start(GL_MODELVIEW_MATRIX);
   glPopMatrix();

   glPushMatrix();
   glRotated(-90,1,1,0);
   Matrix end(GL_MODELVIEW_MATRIX);
   glPopMatrix();

   Matrix m = (1 - r) * start + r * end;
   m.apply();
   glutSolidTeapot(1);
   glutSwapBuffers();
}

bool moving = false;
void idle()
{
   if (moving)
   {
      r += 0.001;
      if (r > 1)
         moving = false;
   }
   glutPostRedisplay();
}

void keyboard (unsigned char key, int x, int y)
{
   switch (key)
   {
      case ' ':
         r = 0;
         moving = true;
         break;
      case 'f':
         glutFullScreen();
         glutPostRedisplay();
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
   gluPerspective(40, double(w)/double(h), 1, 40);
   glutPostRedisplay();
}

int main(int argc, char *argv[])
{
   glutInit(&argc, argv);
   glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGB | GLUT_DEPTH);
   glutInitWindowSize(windowWidth, windowHeight);
   glutInitWindowPosition(300, 300);
   glutCreateWindow("Matrix interpolation: press space to rotate.");
   glutDisplayFunc(display);
   glutKeyboardFunc(keyboard);
   glutReshapeFunc(reshape);
   glutIdleFunc(idle);
   glEnable(GL_DEPTH_TEST);
   glEnable(GL_LIGHTING);
   glEnable(GL_LIGHT0);
   glEnable(GL_NORMALIZE);
   glClearColor(1,1,1,0);
   glutMainLoop();
   return 0;
}
