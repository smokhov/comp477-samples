/* Link with these libraries:
 *  libcugl
 *  libglut32
 *  libopengl32
 *  libglu32
 */

#include <iostream>
#include <sstream>
#include <iomanip>
#include <vector>
#include "cugl.h"
#include <GL/glut.h>
#include <windows.h>

using namespace std;
using namespace cugl;

// Current window dimensions
int windowWidth = 800;
int windowHeight = 800;

// Material data
GLfloat red[] = { 0.9, 0.3, 0.3, 1.0 };
GLfloat green[] = { 0.3, 0.9, 0.3, 1.0 };
GLfloat blue[] = { 0.3, 0.3, 0.9, 1.0 };
GLfloat white[] = { 1.0, 1.0, 1.0, 1.0 };
GLfloat shiny[] = { 50 };
GLfloat dir[] = { 0.0, 0.0, 1.0, 0.0 };

class Link
{
public:
   Link(double length, GLfloat *col) : length(length), radius(length/20), col(col)
   {
      bar = gluNewQuadric();
      gluQuadricDrawStyle(bar, GLU_FILL);
      gluQuadricOrientation(bar, GLU_OUTSIDE);
      gluQuadricNormals(bar, GLU_SMOOTH);
   }

   void draw()
   {
      glMaterialfv(GL_FRONT, GL_AMBIENT_AND_DIFFUSE, col);
      glRotated(degrees(angle), 1, 0, 0);
      gluCylinder(bar, radius, radius, length, 20, 20);
      glTranslated(0, 0, length);
      glutSolidSphere(1.5 * radius, 20, 20);
      for (vector<Link*>::const_iterator i = pLinks.begin(); i != pLinks.end(); ++i)
      {
         glPushMatrix();
         (*i)->draw();
         glPopMatrix();
      }
   }

   void addLink(Link *p)
   {
      pLinks.push_back(p);
   }

   void setRot(double newAngle)
   {
      angle = newAngle;
   }

private:
   double length;
   double radius;
   GLfloat *col;
   double angle;
   vector<Link*> pLinks;
   GLUquadricObj *bar;
};

// Upper arm parameters
Link *upperArm;
const double L = 25;
double theta = 0;

// Lower arm parameters
Link *lowerArm;
const double S = 15;
double phi = PI/2;

// Target point
double tX = L;
double tY = S;

// Current position of tip
double x;
double y;

// Distance to move at each update.
double step = 0.01;

// Points passed through on this trip
vector<Point> points;

// Choose a random target for the tip to aim at
void chooseTarget()
{
   double rad = 0, ang = 2 * PI * randReal();
   do
   {
      rad = L - S + 2 * S * randReal();
   }
   while (rad < (L - S) + 1 || rad > (L + S) - 1);
   tX = rad * cos(ang);
   tY = rad * sin(ang);
   points.clear();
}

void initialize()
{
   upperArm = new Link(L, red);
   lowerArm = new Link(S, green);
   upperArm->addLink(lowerArm);
   chooseTarget();
}

void display (void)
{
   glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
   glMatrixMode(GL_MODELVIEW);
   glLoadIdentity();
   glTranslated(0, 0, -120);
   glRotated(90, 0, 1, 0);

   // Show target
   glPushMatrix();
   glMaterialfv(GL_FRONT, GL_AMBIENT_AND_DIFFUSE, blue);
   glTranslated(0, -tY, tX);
   glutSolidSphere(1, 20, 20);
   glPopMatrix();

   // Record point and show trail
   points.push_back(Point(0, -y, x));
   glDisable(GL_LIGHTING);
   glColor3d(1, 1, 1);
   glBegin(GL_POINTS);
   for (vector<Point>::const_iterator i = points.begin(); i != points.end(); ++i)
      i->draw();
   glEnd();
   glEnable(GL_LIGHTING);

   // Draw robot
   glutSolidSphere(3, 20, 20);
   upperArm->draw();

   glutSwapBuffers();

   ostringstream os;
   os << "Step = " << step;
   glutSetWindowTitle(os.str().c_str());
}

void idle()
{
   static double oldDTheta = 0;
   static double oldDPhi = 0;

   // Use relative position of target and tip to find deltaX and deltaY.
   x = L * cos(theta) + S * cos(theta + phi);
   y = L * sin(theta) + S * sin(theta + phi);

   // Find the distance to the target at (tX, tY).
   // If close, choose another target.
   double deltaX = tX - x;
   double deltaY = tY - y;
   double dist = sqrt(deltaX * deltaX + deltaY * deltaY);
   if (dist < 0.1)
   {
      chooseTarget();
      return;
   }

   // Compute components of a small step towards the target.
   double rd = step / dist;
   deltaX *= rd;
   deltaY *= rd;

   // Use inverse Jacobian to find deltaTheta and deltaPhi
   // If close to a singularity, use previous values
   double deltaTheta = fabs(sin(phi)) < 0.01 ? oldDTheta :
                       (deltaX * cos(theta+phi) + deltaY * sin(theta+phi)) / (L * sin(phi));

   double deltaPhi = fabs(sin(phi)) < 0.01 ? oldDPhi :
                     - (deltaX * (L * cos(theta) + S * cos(theta+phi)) +
                        deltaY * (L * sin(theta) + S * sin(theta+phi))) / (S * L * sin(phi));

   oldDTheta = deltaTheta;
   oldDPhi = deltaPhi;

   // Update angles and arm positions
   theta += deltaTheta;
   upperArm->setRot(theta);
   phi += deltaPhi;
   lowerArm->setRot(phi);

   glutPostRedisplay();
}

bool running = true;

void keyboard (unsigned char key, int x, int y)
{
   switch (key)
   {
      case '+':
         step *= 1.1;
         break;
      case '-':
         step /= 1.1;
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
   gluPerspective(40, double(w)/double(h), 1, 200);
   glutPostRedisplay();
}

int main(int argc, char *argv[])
{
   cout <<
        "Inverse kinematics\n\n"
        " +   faster\n"
        " -   slower \n"
        " f   full screen\n"
        " q   quit\n"
        "ESC  quit\n";
   glutInit(&argc, argv);
   glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGB | GLUT_DEPTH);
   glutInitWindowSize(windowWidth, windowHeight);
   glutInitWindowPosition(300, 300);
   glutCreateWindow("Inverse Kinematics");
   glutDisplayFunc(display);
   glutKeyboardFunc(keyboard);
   glutReshapeFunc(reshape);
   glutIdleFunc(idle);
   glEnable(GL_DEPTH_TEST);
   glEnable(GL_LIGHTING);
   glEnable(GL_LIGHT0);
   glLightfv(GL_LIGHT0, GL_POSITION, dir);
   glMaterialfv(GL_FRONT, GL_SPECULAR, white);
   glMaterialfv(GL_FRONT, GL_SHININESS, shiny);
   initialize();
   glutMainLoop();
   return 0;
}
