#include <iostream>
#include <cmath>
#include <GL/glut.h>

#ifndef CALLBACK
#define CALLBACK
#endif
using namespace std;

const double PI = 4 * atan(1);
double xang = 0;
double yang = 0;

int width = 500;
int height = 500;

GLfloat ctlpoints[5][7][3];
int showPoints = 0;
double time = 0;

GLUnurbsObj *theNurb;

void init_surface(void)
{
   for (int i = 0; i < 5; i++)
   {
      for (int j = 0; j < 7; j++)
      {
         GLfloat x = 0.5 * PI * (i/2.0 - 1);
         GLfloat y = 0.5 * PI * (j/3.0 - 1);
         ctlpoints[i][j][0] = x;
         ctlpoints[i][j][1] = y;
         ctlpoints[i][j][2] = 2 * sin(time) * sin(x) * sin(y);
      }
   }
}

void nurbsError(GLenum errorCode)
{
   const GLubyte *estring;

   estring = gluErrorString(errorCode);
   cerr << "Nurbs Error: %s\n" << estring;
   exit (0);
}

/*  Initialize material property and depth buffer.
 */
void init(void)
{
   GLfloat mat_diffuse[] = { 0.7, 0.7, 0.7, 1.0 };
   GLfloat mat_specular[] = { 1.0, 1.0, 1.0, 1.0 };
   GLfloat mat_shininess[] = { 100.0 };

   glClearColor (0.0, 0.0, 0.0, 0.0);
   glMaterialfv(GL_FRONT, GL_DIFFUSE, mat_diffuse);
   glMaterialfv(GL_FRONT, GL_SPECULAR, mat_specular);
   glMaterialfv(GL_FRONT, GL_SHININESS, mat_shininess);

   glEnable(GL_LIGHTING);
   glEnable(GL_LIGHT0);
   glEnable(GL_DEPTH_TEST);
   glEnable(GL_AUTO_NORMAL);
   glEnable(GL_NORMALIZE);

   theNurb = gluNewNurbsRenderer();
   gluNurbsProperty(theNurb, GLU_SAMPLING_TOLERANCE, 25.0);
   gluNurbsProperty(theNurb, GLU_DISPLAY_MODE, GLU_FILL);
//   gluNurbsCallback(theNurb, GLU_ERROR, nurbsError);
}

void display(void)
{
   GLfloat uknots[9]  = {0.0, 0.0, 0.2, 0.4,  0.5,             0.6, 0.8, 1.0, 1.0};
   GLfloat vknots[11] = {0.0, 0.0, 0.1, 0.2,  0.3, 0.4, 0.7,   0.8, 0.9, 1.0, 1.0};

//   GLfloat uknots[9]  = {0.0, 0.0, 0.0, 0.0,  0.5,             1.0, 1.0, 1.0, 1.0};
//   GLfloat vknots[11] = {0.0, 0.0, 0.0, 0.0,  0.3, 0.5, 0.7,   1.0, 1.0, 1.0, 1.0};

   glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
   glMatrixMode(GL_MODELVIEW);
   glLoadIdentity();
   glTranslatef(0, 0, -20);

   glRotated(xang, 1, 0, 0);
   glRotated(yang, 0, 1, 0);

   glPushMatrix();

   init_surface();
   gluBeginSurface(theNurb);
   gluNurbsSurface(theNurb,
                   9, uknots, 11, vknots,
                   5 * 3, 3, &ctlpoints[0][0][0],
                   4, 4, GL_MAP2_VERTEX_3);
   gluEndSurface(theNurb);

   //if (showPoints)
   {
      glPointSize(2);
      glDisable(GL_LIGHTING);
      glColor3f(1.0, 1.0, 0.0);
      glBegin(GL_POINTS);
      for (int i = 0; i < 5; i++)
      {
         for (int j = 0; j < 7; j++)
         {
            glVertex3f(ctlpoints[i][j][0], ctlpoints[i][j][1], ctlpoints[i][j][2]);
         }
      }
      glEnd();
      glEnable(GL_LIGHTING);
   }
   glPopMatrix();
   glFlush();
}

void idle()
{
   time += 0.01;
   if (time > 2 * PI)
      time -= 2 * PI;
   glutPostRedisplay();
}

void mouseMovement (int mx, int my)
{
   xang = 180.0 * (1 - double(my) / double(height));
   yang = 180.0 * double(mx) / double(height);
   glutPostRedisplay();
}

void reshape(int w, int h)
{
   glViewport(0, 0, (GLsizei) w, (GLsizei) h);
   glMatrixMode(GL_PROJECTION);
   glLoadIdentity();
   gluPerspective (45.0, (GLdouble)w/(GLdouble)h, 5, 50);
}

void keyboard(unsigned char key, int x, int y)
{
   switch (key)
   {
      case 'c':
      case 'C':
         showPoints = !showPoints;
         glutPostRedisplay();
         break;
      case 27:
         exit(0);
         break;
      default:
         break;
   }
}

int main(int argc, char** argv)
{
   glutInit(&argc, argv);
   glutInitDisplayMode(GLUT_SINGLE | GLUT_RGB | GLUT_DEPTH);
   glutInitWindowSize (width, height);
   glutCreateWindow(argv[0]);
   init();
   glutReshapeFunc(reshape);
   glutDisplayFunc(display);
   glutIdleFunc(idle);
   glutKeyboardFunc (keyboard);
   glutMotionFunc(mouseMovement);
   glutMainLoop();
   return 0;
}
