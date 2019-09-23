/* Link with these libraries:
 *  libcugl
 *  libglut32
 *  libopengl32
 *  libglu32
 */

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
   Link(double length, double radius, GLfloat *col) : length(length), radius(radius), col(col)
   {
      bar = gluNewQuadric();
      gluQuadricDrawStyle(bar, GLU_FILL);
      gluQuadricOrientation(bar, GLU_OUTSIDE);
      gluQuadricNormals(bar, GLU_SMOOTH);
   }

   void draw()
   {
      glMaterialfv(GL_FRONT, GL_AMBIENT_AND_DIFFUSE, col);
      ori.apply();
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

   void setRot(Quaternion newOri)
   {
      ori = newOri;
   }

private:
   double length;
   double radius;
   GLfloat *col;
   Quaternion ori;
   vector<Link*> pLinks;
   GLUquadricObj *bar;
};

Link *upperArm;
Link *lowerArm;
Link *finger;
Link *thumb;

void build()
{
   upperArm = new Link(25, 1.5, red);
   lowerArm = new Link(15, 0.75, green);
   upperArm->addLink(lowerArm);
   finger = new Link(6, 0.3, blue);
   lowerArm->addLink(finger);
   thumb = new Link(4, 0.5, blue);
   lowerArm->addLink(thumb);
}

void display (void)
{
   glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
   glMatrixMode(GL_MODELVIEW);
   glLoadIdentity();
   glTranslated(-15, -10, -100);
   glRotated(-90, 1, 0, 0);

   glutSolidSphere(5, 20, 20);
   upperArm->draw();
   glutSwapBuffers();
}

Quaternion upperQuat = Quaternion(I, 0.3);
Quaternion lowerQuat = Quaternion(J, 0.7);
Quaternion fingerQuat = Quaternion(K, 0.5);
Quaternion thumbQuat = Quaternion(J, 1.6);
double sp = 0.001;

void idle()
{
   upperQuat *= Quaternion(Vector(1, 0, 1).unit(), sp);
   upperArm->setRot(upperQuat);
   lowerQuat *= Quaternion(Vector(0, 1, 0), 3 * sp);
   lowerArm->setRot(lowerQuat);
   fingerQuat *= Quaternion(Vector(0, 1, 1).unit(), 8 * sp);
   finger->setRot(fingerQuat);
   thumbQuat *= Quaternion(Vector(1, 0, 0), 5 * sp);
   thumb->setRot(thumbQuat);

   upperQuat.normalize();
   lowerQuat.normalize();
   fingerQuat.normalize();
   thumbQuat.normalize();

   glutPostRedisplay();
}

void keyboard (unsigned char key, int x, int y)
{
   switch (key)
   {
      case 'f':
         glutFullScreen();
         break;
      case '+':
         sp *= 1.1;
         break;
      case '-':
         sp /= 1.1;
         break;
      case 27:
      case 'q':
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
        "Forward kinematics\n\n"
        "+  speed up\n"
        "-  slow down\n"
        "f  full screen\n"
        "ESC  quit\n";
   glutInit(&argc, argv);
   glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGB | GLUT_DEPTH);
   glutInitWindowSize(windowWidth, windowHeight);
   glutInitWindowPosition(300, 300);
   glutCreateWindow("Forward Kinematics");
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
   build();
   glutMainLoop();
   return 0;
}
