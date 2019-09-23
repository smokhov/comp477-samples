/* Constant rate animation.
 * Peter Grogono.
 * COMP 477.
 * 21/12/2010.
 *
 * Link with:
 *  libglut32
 *  libopengl32
 *  libglu32
 */

#include <iostream>
#include <vector>
#include <GL/glut.h>
#include <windows.h>
#include <cmath>
#include <sstream>
#include <iomanip>
using namespace std;

// Current window dimensions
int windowWidth = 800;
int windowHeight = 800;

// Material data
GLfloat trans[] = { 0.5, 0.5, 0.5, 0.1 };
GLfloat opaque[] = { 0.2, 0.2, 0.2, 1.0 };
GLfloat white[] = { 1.0, 1.0, 1.0, 1.0 };
GLfloat red[] = { 1.0, 0.6, 0.6, 1.0 };
GLfloat shiny[] = { 50 };
GLfloat dir[] = { -1, 1, 1, 0 };

// Cube data
const double HALF_CUBE_SIDE = 7;

double verts[] =
{
   -HALF_CUBE_SIDE, -HALF_CUBE_SIDE, -HALF_CUBE_SIDE,
   -HALF_CUBE_SIDE,  HALF_CUBE_SIDE, -HALF_CUBE_SIDE,
   -HALF_CUBE_SIDE,  HALF_CUBE_SIDE,  HALF_CUBE_SIDE,
   -HALF_CUBE_SIDE, -HALF_CUBE_SIDE,  HALF_CUBE_SIDE,
   HALF_CUBE_SIDE, -HALF_CUBE_SIDE, -HALF_CUBE_SIDE,
   HALF_CUBE_SIDE,  HALF_CUBE_SIDE, -HALF_CUBE_SIDE,
   HALF_CUBE_SIDE,  HALF_CUBE_SIDE,  HALF_CUBE_SIDE,
   HALF_CUBE_SIDE, -HALF_CUBE_SIDE,  HALF_CUBE_SIDE
};

GLubyte indexes[] =
{
   4,5,6,7,
   1,2,6,5,
   0,1,5,4,
   0,3,2,1,
   0,4,7,3,
   2,3,7,6
};

const double DT = 10; // Time increment (milliseconds)
const double BALL_RADIUS = 0.5;
const double CHECK_DISTANCE = HALF_CUBE_SIDE - BALL_RADIUS; // For collisions with walls of cube
const double K = 0.01; // Scale initial velocity

// Viewing angles, controlled by mouse
double yAng = 0;
double zAng = 0;

// Return a random double in [0,1).
inline double rdv()
{
   return double(rand()) / double(RAND_MAX);
}

inline double sqr(double x)
{
   return x * x;
}

class Ball
{
public:

   // Construct a ball: first is red, others are green.
   Ball(bool first) : xp(0), yp(0), zp(0), xv(K * rdv()), yv(K * rdv()), zv(K * rdv())
   {
      if (first)
      {
         col[0] = 1;
         col[1] = 0;
      }
      else
      {
         col[0] = 0;
         col[1] = 1;
      }
      col[2] = 0;
      col[3] = 1;
   }

   // Update ball position, checking for collision with wall.
   void update()
   {
      xp += DT * xv;
      if (xp < -CHECK_DISTANCE)
      {
         xp = -CHECK_DISTANCE;
         xv = -xv;
      }
      else if (xp > CHECK_DISTANCE)
      {
         xp = CHECK_DISTANCE;
         xv = -xv;
      }

      yp += DT * yv;
      if (yp < -CHECK_DISTANCE)
      {
         yp = -CHECK_DISTANCE;
         yv = -yv;
      }
      else if (yp > CHECK_DISTANCE)
      {
         yp = CHECK_DISTANCE;
         yv = -yv;
      }

      zp += DT * zv;
      if (zp < -CHECK_DISTANCE)
      {
         zp = -CHECK_DISTANCE;
         zv = -zv;
      }
      else if (zp > CHECK_DISTANCE)
      {
         zp = CHECK_DISTANCE;
         zv = -zv;
      }
   }

   void draw()
   {
      glMaterialfv(GL_FRONT, GL_AMBIENT_AND_DIFFUSE, col);
      glPushMatrix();
      glTranslated(xp, yp, zp);
      glutSolidSphere(BALL_RADIUS, 100, 100);
      glPopMatrix();
   }

private:
   GLfloat col[4]; // Colour
   double xp, yp, zp; // Position
   double xv, yv, zv; // Velocity
};

// The model consists of a set of balls.
class Model
{
public:

   // Initial model has one red ball.
   void initialize()
   {
      balls.push_back(Ball(true));
   }

   // Balls can be added to, or removed from, the model.
   void change(char key)
   {
      size_t newSize;
      switch (key)
      {
         case '+':
            newSize = 2 * balls.size();
            while (balls.size() < newSize)
               balls.push_back(false);
            break;
         case '-':
            newSize = balls.size() / 2;
            if (newSize < 1)
               newSize = 1;
            while (balls.size() > newSize)
               balls.pop_back();
            break;
      }
   }

   void draw()
   {
      for (vector<Ball>::iterator i = balls.begin(); i != balls.end(); ++i)
         i->draw();
   }

   void update()
   {
      for (vector<Ball>::iterator i = balls.begin(); i != balls.end(); ++i)
         i->update();
   }

   unsigned int size()
   {
      return balls.size();
   }

private:
   vector<Ball> balls;
};

Model model;
int frameCount = 0;
int updateCount = 0;

void display (void)
{
   glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
   glMatrixMode(GL_MODELVIEW);
   glLoadIdentity();
   glTranslated(0, 0, -30);

   // Rotate using angles set by mouse.
   glRotated(yAng, 0, 1, 0);
   glRotated(zAng, 0, 0, 1);

   // Display balls.
   model.draw();

   // Display transparent cube.
   glMaterialfv(GL_FRONT, GL_AMBIENT_AND_DIFFUSE, trans);
   glMaterialfv(GL_FRONT, GL_SPECULAR, white);
   glEnableClientState(GL_VERTEX_ARRAY);
   glVertexPointer(3, GL_DOUBLE, 0, verts);
   glDrawElements(GL_QUADS, 24, GL_UNSIGNED_BYTE, indexes);

   glutSwapBuffers();
   ++frameCount;
}

void idle()
{
   static double prevTime = GetTickCount();
   static double accumulatedTime = 0;
   int cycles = 0;
   double currTime = GetTickCount();
   double elapsedTime = (currTime - prevTime);
   if (elapsedTime > 0)
   {
      accumulatedTime += elapsedTime;
      while (accumulatedTime >= DT)
      {
         model.update();
         ++updateCount;
         accumulatedTime -= DT;
         ++cycles;
      }
      prevTime = currTime;
   }
   glutPostRedisplay();
}

void report(int)
{
   ostringstream os;
   os <<
   "Balls: " << model.size() <<
   "    Updates: " << updateCount <<
   "    Frames/sec: " << frameCount;
   glutSetWindowTitle(os.str().c_str());
   updateCount = 0;
   frameCount = 0;
   glutTimerFunc(1000, report, 0);
}

void mouse(int x, int y)
{
   double mx = double(x) / double(windowWidth);
   double my = 1.0 - double(y) / double(windowHeight);
   yAng = 180 * (mx - 0.5);
   zAng = 180 * (my - 0.5);
   glutPostRedisplay();
}

void keyboard (unsigned char key, int x, int y)
{
   switch (key)
   {
      case '+':
      case '-':
         model.change(key);
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
   gluPerspective(40, double(w)/double(h), 1, 200);
   glutPostRedisplay();
}

int main(int argc, char *argv[])
{
   cout <<
        "Timing demonstration\n\n"
        " '+'   add ball\n"
        " '-'   remove ball\n"
        " 'f'   full screen\n"
        " Use mouse to rotate cube\n"
        " ESC quits\n";

   glutInit(&argc, argv);
   glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGB | GLUT_DEPTH);
   glutInitWindowSize(windowWidth, windowHeight);
   glutInitWindowPosition(300, 300);
   glutCreateWindow("Bouncing Ball");
   glutDisplayFunc(display);
   glutIdleFunc(idle);
   glutMotionFunc(mouse);
   glutKeyboardFunc(keyboard);
   glutReshapeFunc(reshape);
   glutTimerFunc(1000, report, 0);
   glEnable(GL_DEPTH_TEST);
   glEnable(GL_LIGHTING);
   glEnable(GL_LIGHT0);
   glEnable(GL_BLEND);
   glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
   glShadeModel(GL_SMOOTH);
   glMaterialfv(GL_FRONT, GL_SHININESS, shiny);
   glClearColor(1,1,1,1);
   glLightfv(GL_LIGHT0, GL_POSITION, dir);
   for (int i = 0; i < 10000; ++i)
      rand(); // Avoid boring random numbers.
   model.initialize();
   glutMainLoop();
   return 0;
}
