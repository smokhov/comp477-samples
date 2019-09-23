#include <GL/glut.h>
#include <cstdlib>
#include <iostream>
#include <iomanip>
#include <sstream>

using namespace std;

// Initial size of graphics window.
const int WIDTH  = 600;
const int HEIGHT = 600;

// Current size of window.
int width  = WIDTH;
int height = HEIGHT;

// Mouse positions, normalized to [0,1].
double xMouse = 0.5;
double yMouse = 0.5;

// Bounds of viewing frustum.
double nearPlane =  1;
double farPlane  = 50;
double halfWay = - (farPlane + nearPlane) / 2;

double fovy = 75;
const double INIT_VEL = 50;
double vel = 1;
double ex = 1;
double ey = 1;
double ez = 1;

class Point
{
public:
   Point() : px(0), py(0), pz(0), vx(0), vy(0), vz(0)
   {
      vx = rand() / (INIT_VEL * RAND_MAX);
      vy = rand() / (INIT_VEL * RAND_MAX);
      vz = rand() / (INIT_VEL * RAND_MAX);
      r = rand() / double(RAND_MAX);
      g = rand() / double(RAND_MAX);
      b = rand() / double(RAND_MAX);
   }
   void reset()
   {
      px = 0;
      py = 0;
      pz = 0;
   }
   void move()
   {
      px += vel * vx;
      if (px > 1) px -= 2;
      if (px < -1) px += 2;

      py += vel * vy;
      if (py > 1) py -= 2;
      if (py < -1) py += 2;

      pz += vel * vz;
      if (pz > 1) pz -= 2;
      if (pz < -1) pz += 2;
   }
   void draw()
   {
      glPushMatrix();
      glTranslated(px, py, pz);
      glColor3d(r, g, b);
      glutSolidSphere(0.02, 10, 10);
      glPopMatrix();
   }
private:
   double px, py, pz, vx, vy, vz, r, g, b;
};

const int NUM_POINTS = 25;
Point pts[NUM_POINTS];

bool isVisible(double px, double py, double pz)
{
   GLdouble mm[16], pm[16];
   GLint vp[4];
   double x, y, z;

   glPushMatrix();
   glLoadIdentity();
   gluLookAt(ex, ey, ez,  0, 0, 0,  0, 0, 1);
   glGetDoublev(GL_MODELVIEW_MATRIX, mm);
   glPopMatrix();

   glGetDoublev(GL_PROJECTION_MATRIX, pm);
   glGetIntegerv(GL_VIEWPORT, vp);
   gluProject(px, py, pz, mm, pm, vp, &x, &y, &z);
   return 0 < x && x < width && 0 < y && y < height;
}

bool isCubeVisible(double cx, double cy, double cz)
{
   return
      isVisible(cx - 1, cy - 1, cz - 1) ||
      isVisible(cx - 1, cy - 1, cz + 1) ||
      isVisible(cx - 1, cy + 1, cz - 1) ||
      isVisible(cx - 1, cy + 1, cz + 1) ||
      isVisible(cx + 1, cy - 1, cz - 1) ||
      isVisible(cx + 1, cy - 1, cz + 1) ||
      isVisible(cx + 1, cy + 1, cz - 1) ||
      isVisible(cx + 1, cy + 1, cz + 1);
}


void scene(bool drawCube, bool centreCube)
{
   if (drawCube)
   {
      if (centreCube)
         glColor3f(1, 1, 1);
      else
         glColor3f(0, 0, 0.8);
      glutWireCube(2);
   }
   for (int p = 0; p < NUM_POINTS; ++p)
      pts[p].draw();
}

enum Mode { NO_CUBES, ONE_CUBE, ALL_CUBES } mode = ONE_CUBE;

// This function is called to display the scene.
void display ()
{
   glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
   glMatrixMode(GL_MODELVIEW);
   glLoadIdentity();
   gluLookAt(ex, ey, ez,  0, 0, 0,  0, 0, 1);

   int cubes = 0;
   string msg;
   for (int x = -4; x <= 4; x += 2)
      for (int y = -4; y <= 4; y += 2)
         for (int z = -4; z <= 4; z += 2)
         {
            if (isCubeVisible(x, y, z))
            {
               glPushMatrix();
               glTranslated(x, y, z);
               bool showCubes;
               switch (mode)
               {
                  case NO_CUBES:
                     showCubes = false;
                     msg = "No cubes.";
                     break;
                  case ONE_CUBE:
                     showCubes = x == 0 && y == 0 && z == 0;
                     msg = "One cube.";
                     break;
                  case ALL_CUBES:
                     showCubes = true;
                     msg = "All cubes.";
                     break;
               }
               scene(showCubes, x == 0 && y == 0 && z == 0);
               glPopMatrix();
               ++cubes;
            }
         }

   glutSwapBuffers();

   ostringstream os;
   os << msg << "  ";
   os << fixed << setprecision(1);
//   os << "  EX = " << ex;
//   os << "  EY = " << ey;
//   os << "  EZ = " << ez;
   os << setw(12) << cubes << " cubes.";
   glutSetWindowTitle(os.str().c_str());
}

void idle ()
{
   for (int p = 0; p < NUM_POINTS; ++p)
      pts[p].move();
   glutPostRedisplay();
}

void mouseMovement (int mx, int my)
{
   ex = (5.0 * mx) / width;
   ez = (5.0 * (height - my)) / height;
   glutPostRedisplay();
}

// Respond to window resizing, preserving proportions.
// Parameters give new window size in piyels.
void reshapeMainWindow (int newWidth, int newHeight)
{
   width = newWidth;
   height = newHeight;
   glViewport(0, 0, width, height);
   glMatrixMode(GL_PROJECTION);
   glLoadIdentity();
   gluPerspective(fovy, GLfloat(width) / GLfloat(height), nearPlane, farPlane);
}

void graphicKeys (unsigned char key, int x, int y)
{
   switch (key)
   {
      case 'f':
         glutFullScreen();
         break;

      case 'm':
         {
            int m = mode;
            m = (m + 1) % 3;
            mode = Mode(m);
            break;
         }

      case 'r':
         for (int p = 0; p < NUM_POINTS; ++p)
            pts[p].reset();
         break;


      case '+':
         vel *= 1.1;
         break;

      case '-':
         vel /= 1.1;
         break;

      case 27:
         exit(0);
   }
}

int main (int argc, char **argv)
{
   cout <<
        "Torus Viewer\n\n"
        " +   faster\n"
        " -   slower\n"
        " f  full screen\n"
        " m   step mode\n"
        " r   move all points to origin\n";

   // GLUT initialization.
   glutInit(&argc, argv);
   glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGB);
   glutInitWindowSize(width, height);
   glutCreateWindow("");

   // Register call backs.
   glutDisplayFunc(display);
   glutReshapeFunc(reshapeMainWindow);
   glutMotionFunc(mouseMovement);
   glutKeyboardFunc(graphicKeys);
   glutIdleFunc(idle);
   glutMainLoop();
   return 0;
}
