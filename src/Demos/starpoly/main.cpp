#include <iostream>
#include <iomanip>
#include <GL/glut.h>
#include <GL/cugl.h>

using namespace std;
using namespace cugl;

// Initial size of graphics window.
const int WIDTH  = 600;
const int HEIGHT = 600;

// Current size of window.
int width  = WIDTH;
int height = HEIGHT;

// Bounds of viewing frustum.
GLfloat nearPlane = 10;
GLfloat farPlane  = 100;
GLfloat halfWay = - 0.5 * (nearPlane + farPlane);

// Viewing angle.
GLfloat fovy = 40;

// Current rotation represented as a Quaternion
Quaternion quat;

// Lighting parameters.
GLfloat ambient[] = {0.4f, 0.4f, 0.4f, 1};
GLfloat position[] = {5, 20, 0, 1};

const int SLICES = 20;
const int SLABS = 20;

Point block[SLABS][SLICES];
Point cones[SLABS][SLICES];

double t;
char mode = 'b';

void initialize()
{
   // Block
   double r = 5, x, y;
   for (int iz = 0; iz < SLABS; ++iz)
   {
      double z = (20.0 * iz) / SLABS - 10;
      for (int ia = 0; ia < SLICES; ++ia)
      {
         double a = (2 * PI * ia) / SLICES;
         if (7*PI/4 <= a || a <= PI/4)
         {
            x = r;
            y = r * tan(a);
         }
         else if (PI/4 <= a && a <= 3*PI/4)
         {
            x = r * tan(PI/2 - a);
            y = r;
         }
         else if (3*PI/4 <= a && a <= 5*PI/4)
         {
            x = -r;
            y = r * tan(PI - a);
         }
         else if (5*PI/4 <= a && a <= 7*PI/4)
         {
            x = - r * tan(3*PI/2 - a);
            y = -5;
         }
         block[iz][ia] = Point(x, y, z);
      }
   }

   // Cone
   for (int iz = 0; iz <= SLABS; ++iz)
   {
      double z = (20.0 * iz) / SLABS - 10;
      for (int ia = 0; ia < SLICES; ++ia)
      {
         double a = (2 * PI * ia) / SLICES;
         double r = z >= 0 ? 10 - z : 10 + z;
         double x = r * cos(a);
         double y = r * sin(a);
         cones[iz][ia] = Point(x, y, z);
      }
   }
}

void displayShape(double t)
{
   Point shape[SLABS][SLICES];
   for (int iz = 0; iz < SLABS; ++iz)
   {
      for (int ia = 0; ia < SLICES; ++ia)
      {
         Point p = block[iz][ia];
         Point q = cones[iz][ia];
         shape[iz][ia] = Point(t * p[0] + (1 - t) * q[0], t * p[1] + (1 - t) * q[1], t * p[2] + (1 - t) * q[2]);
      }
   }
   for (int iz = 0; iz < SLABS; ++iz)
   {
      for (int ia = 0; ia < SLICES; ++ia)
      {
         glPushMatrix();
         Point p = shape[iz][ia];
         glTranslated(p[0], p[1], p[2]);
         glutSolidSphere(0.1, 10,10);
         glPopMatrix();
      }
   }
}

void display ()
{
   glClearColor(1, 1, 1, 1);
   glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
   glMatrixMode(GL_MODELVIEW);
   glLoadIdentity();
   glTranslatef(0, 0, halfWay);
   quat.apply();
   axes(2);
   switch (mode)
   {
      case 'b':
         displayShape(1);
         break;
      case 'c':
         displayShape(0);
         break;
      case 'm':
         displayShape(t);
         break;
   }
   glutSwapBuffers();
}

void idle()
{
   static bool increasing = true;
   if (increasing)
   {
      t += 0.01;
      if (t >= 1)
         increasing = false;
   }
   else
   {
      t -= 0.01;
      if (t <= 0)
         increasing = true;
   }

   glutPostRedisplay();
}

void mouseMovement (int xNew, int yNew)
{
   // Avoid a sudden movement at the first mouse movement.
   const int MSTART = -10000;
   static int xOld = MSTART;
   static int yOld = MSTART;
   if (xOld == MSTART && yOld == MSTART)
   {
      xOld = xNew + 1;
      yOld = yNew + 1;
   }

   // Compute a quaternion from the mouse movement
   // and apply it to current quaternion
   quat.trackball(
      float(2 * xOld - width) / float(width),
      float(height - 2 * yOld) / float(height),
      float(2 * xNew - width) / float(width),
      float(height - 2 * yNew) / float(height) );

   // Save previous mouse position.
   xOld = xNew;
   yOld = yNew;

   glutPostRedisplay();
}

void reshapeMainWindow (int newWidth, int newHeight)
{
   width = newWidth;
   height = newHeight;
   glViewport(0, 0, width, height);
   glMatrixMode(GL_PROJECTION);
   glLoadIdentity();
   gluPerspective(fovy, GLfloat(width) / GLfloat(height),
                  nearPlane, farPlane);
}

// Exit when the user hits ESC.
void graphicKeys (unsigned char key, int x, int y)
{
   if (key == 'b' || key == 'c' || key == 'm')
      mode = key;
   else if (key == 27)
      exit(0);
}

int main (int argc, char **argv)
{
   // GLUT initialization.
   glutInit(&argc, argv);
   glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGB);
   glutInitWindowSize(width, height);
   glutCreateWindow("Star polygon");
   glEnable(GL_DEPTH_TEST);
   glEnable(GL_LIGHTING);
   glEnable(GL_LIGHT0);
   glLightfv(GL_LIGHT0, GL_AMBIENT, ambient);
   glLightfv(GL_LIGHT0, GL_POSITION, position);

   glutIdleFunc(idle);
   glutDisplayFunc(display);
   glutReshapeFunc(reshapeMainWindow);
   glutKeyboardFunc(graphicKeys);
   glutPassiveMotionFunc(mouseMovement);

   initialize();
   glutMainLoop();
   return 0;
}
