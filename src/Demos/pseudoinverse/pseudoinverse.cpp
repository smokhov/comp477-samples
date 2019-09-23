// Inverse kinematics using pseudo-inverse of Jacobian matrix

// Link with libcugl libglut32 libopengl32 libglu32

#include <iostream>
#include <sstream>
#include <iomanip>
#include <vector>
#include <GL/cugl.h>
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

// The arm has three components
Link *upper;
const double L = 25;
double theta = -PI/4;

Link *middle;
const double M = 15;
double phi = -PI/4;

Link *lower;
const double S = 10;
double psi = -PI/4;

// Target point - moved when the arm has reached it
double tX = L;
double tY = S;

// Current position of tip
double x;
double y;

// Control step size
double step = 0.01;

// Points passed through on this trip
vector<Point> points;

// Choose a random target for the tip to aim at
void chooseTarget()
{
   double rad = 0, ang = 2 * PI * randReal();
   rad = 2 + (L + M + S - 3) * randReal();
   tX = rad * cos(ang);
   tY = rad * sin(ang);
   points.clear();
}

// Initialize arm and target
void initialize()
{
   upper = new Link(L, red);
   middle = new Link(M, green);
   lower = new Link(S, blue);
   upper->addLink(middle);
   middle->addLink(lower);
   chooseTarget();
}

void display (void)
{
   glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
   glMatrixMode(GL_MODELVIEW);
   glLoadIdentity();
   glTranslated(0, 0, -150);
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

   // Draw robot arm
   glutSolidSphere(3, 20, 20);
   upper->draw();

   glutSwapBuffers();
}

void idle()
{
   static double oldDTheta = 0;
   static double oldDPhi = 0;
   static double oldDPsi = 0;

   // Current position of tip
   x = L * cos(theta) + M * cos(theta + phi) + S * cos(theta + phi + psi);
   y = L * sin(theta) + M * sin(theta + phi) + S * sin(theta + phi + psi);

   // Position of tip relative to target
   double deltaX = tX - x;
   double deltaY = tY - y;

   // If tip is close to target, move the tartget
   double dist = sqrt(deltaX * deltaX + deltaY * deltaY);
   if (dist < 0.1)
   {
      chooseTarget();
      return;
   }

   // Scale deltas according to distance
   double rd = step / dist;
   deltaX *= rd;
   deltaY *= rd;

   // Find partial derivatives
   double dx_dpsi = - S * sin(theta + phi + psi);
   double dx_dphi = - M * sin(theta + phi) + dx_dpsi;
   double dx_dtht = - L * sin(theta) + dx_dphi;

   double dy_dpsi = S * cos(theta + phi + psi);
   double dy_dphi = M * cos(theta + phi) + dy_dpsi;
   double dy_dtht = L * cos(theta) + dy_dphi;

   // Set up Jacobian J
   double j[2][3];
   j[0][0] = dx_dtht;
   j[0][1] = dx_dphi;
   j[0][2] = dx_dpsi;
   j[1][0] = dy_dtht;
   j[1][1] = dy_dphi;
   j[1][2] = dy_dpsi;

   // Compute transposed Jacobian J^T
   double jt[3][2];
   for (int r = 0; r < 3; ++r)
      for (int c = 0; c < 2; ++c)
         jt[r][c] = j[c][r];

   // Compute product J J^T
   double jjt[2][2];
   for (int r = 0; r < 2; ++r)
      for (int c = 0; c < 2; ++c)
      {
         jjt[r][c] = 0;
         for (int k = 0; k < 3; ++k)
            jjt[r][c] += j[r][k] * jt[k][c];
      }

   // Set angle increments with old values in case there is a singularity
   double deltaTheta = oldDTheta;
   double deltaPhi = oldDPhi;
   double deltaPsi = oldDPsi;

   // Computer determinant
   double det = jjt[0][0] * jjt[1][1] - jjt[0][1] * jjt[1][0];
   if (det == 0)
   {
      cerr << "Singularity!\n";
   }
   else
   {
      // Determinant is non-zero, so compute inverse
      double jjti[2][2];
      jjti[0][0] =   jjt[1][1] / det;
      jjti[0][1] = - jjt[0][1] / det;
      jjti[1][0] = - jjt[1][0] / det;
      jjti[1][1] =   jjt[0][0] / det;

      // Pseudoinverse = transpose * inverse
      double psi[3][2];
      for (int r = 0; r < 3; ++r)
         for (int c = 0; c < 2; ++c)
         {
            psi[r][c] = 0;
            for (int k = 0; k < 2; ++k)
               psi[r][c] += jt[r][k] * jjti[k][c];
         }

      // Obtain angle increments from pseudo-inverse
      deltaTheta = psi[0][0] * deltaX + psi[0][1] * deltaY;
      deltaPhi =   psi[1][0] * deltaX + psi[1][1] * deltaY;
      deltaPsi =   psi[2][0] * deltaX + psi[2][1] * deltaY;

      // Save increments in case we need them
      oldDTheta = deltaTheta;
      oldDPhi = deltaPhi;
      oldDPsi = deltaPsi;
   }

   // Update angles and arm positions
   theta += deltaTheta;
   upper->setRot(theta);

   phi += deltaPhi;
   middle->setRot(phi);

   psi += deltaPsi;
   lower->setRot(psi);

   glutPostRedisplay();
}

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
      case 't':
         chooseTarget();
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
   "Pseudoinverse demonstration\n\n"
   " +   faster\n"
   " -   slower\n"
   " t   choose new target\n"
   " f   full screen\n"
   " q   quit\n"
   "ESC  quiit\n";
   glutInit(&argc, argv);
   glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGB | GLUT_DEPTH);
   glutInitWindowSize(windowWidth, windowHeight);
   glutInitWindowPosition(300, 300);
   glutCreateWindow("Inverse Kinematics using the Pseudoinverse");
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
