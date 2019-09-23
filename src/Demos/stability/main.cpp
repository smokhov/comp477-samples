#include <iostream>
#include <iomanip>
#include <sstream>
#include <GL/cugl.h>
#include <GL/glut.h>

using namespace std;
using namespace cugl;

int width  = 800;
int height = 600;

Matrix mi, m, dm;
Quaternion qi, q, dq;
int rots = 0;
const double delta = 0.01 * (PI + 0.001);

void display ()
{
   glClearColor(1, 1, 1, 1);
   glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
   glMatrixMode(GL_MODELVIEW);
   glLoadIdentity();
   glTranslatef(0, 0, -15);

   glPushMatrix();
   glTranslated(-5, 0, 0);
   glRotated(45, 1, 1, 0);
   mi.apply();
   glutWireSphere(2, 20, 20);
   glPopMatrix();

   glPushMatrix();
   glTranslated(0, 0, 0);
   m.apply();
   glutWireSphere(2, 20, 20);
   glPopMatrix();

   glPushMatrix();
   glTranslated(5, 0, 0);
   q.apply();
   glutWireSphere(2, 20, 20);
   glPopMatrix();

   glutSwapBuffers();
   ostringstream os;
   os << rots << " rotations applied";
   glutSetWindowTitle(os.str().c_str());
}

void idle()
{
   const int mul = 10000;
   for (int i = 0; i < mul; ++i)
   {
      m *= dm;
      q *= dq;
   }
   q.normalize();
   rots += mul;
   glutPostRedisplay();
}

void reshape(int newWidth, int newHeight)
{
   width = newWidth;
   height = newHeight;
   glViewport(0, 0, width, height);
   glMatrixMode(GL_PROJECTION);
   glLoadIdentity();
   glOrtho(-8, 8, -6, 6, 1, 20);
}

void graphicKeys (unsigned char key, int x, int y)
{
   switch (key)
   {
      case 27:
         exit(0);
   }
}

int main (int argc, char **argv)
{
   glutInit(&argc, argv);
   glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGB);
   glutInitWindowSize(width, height);
   glutCreateWindow("Matrix versus quaternion.");
   glEnable(GL_DEPTH_TEST);
   glutDisplayFunc(display);
   glutIdleFunc(idle);
   glutReshapeFunc(reshape);
   glutKeyboardFunc(graphicKeys);
   glColor3d(0, 0, 0);
   Vector axis(1, 1, 1);
   axis.normalize();
   dm = Matrix(axis, delta);
   dq = Quaternion(axis, delta);

   cout <<
   "Comparison of quaternion and matrix calculation.\n\n"
   "The left sphere is fixed at an angle to the vertical.\n"
   "The middle sphere is rotated using a matrix.\n"
   "The right sphere is rotated using a quaternion.\n\n"
   "Rotations are computed cumulatively, which can often be avoided in practice.\n";

   glutMainLoop();
   return 0;
}
