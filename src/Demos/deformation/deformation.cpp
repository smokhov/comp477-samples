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

int windowWidth = 800;
int windowHeight = 600;

// World coordinates
const double xFrame = 3.5;
const double yFrame = 3;

string message;       // report current values

// Display global 'message' at (x, y).
void showString (GLfloat x, GLfloat y)
{
   glRasterPos2f(x, y);
   size_t len = message.size();
   for (size_t i = 0; i < len; i++)
      glutBitmapCharacter(GLUT_BITMAP_TIMES_ROMAN_24, message[i]);
}

// Local grid
const int X_LOC_SIZE = 11;
const int Y_LOC_SIZE = 3;
Point grid[X_LOC_SIZE][Y_LOC_SIZE];

// A model coordinate relative to local grid
class Coord
{
public:
   Coord() {}

   // Find grid rectangle and store relative position of model point (x, y).
   Coord(double x, double y)
   {
      for (int ix = 0; ix < X_LOC_SIZE - 1; ++ix)
         for (int iy = 0; iy < Y_LOC_SIZE - 1; ++iy)
         {
            Point psw = grid[ix][iy];
            Point pne = grid[ix+1][iy+1];
            if (psw[0] <= x && x <= pne[0] && psw[1] <= y && y <= pne[1])
            {
               mx = ix;
               my = iy;
               u = (x - psw[0]) / (pne[0] - psw[0]);
               v = (y - psw[1]) / (pne[1] - psw[1]);
            }
         }
   }

   // Draw by interpolating within local grid quad.
   void draw()
   {
      Point p = (1 - v) * ((1 - u) * grid[mx][my] + u * grid[mx+1][my]) +
                v * ((1 - u) * grid[mx][my+1] + u * grid[mx+1][my+1]);
      p.draw();
   }

private:
   int mx, my;  // grid position
   double u, v; // grid offsets
};


// Model
const int MODEL_SIZE = 500;
Point model[MODEL_SIZE];      // World coordinates
Coord localized[MODEL_SIZE];  // Grid-relative coordinates

void display (void)
{
   glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
   glMatrixMode(GL_MODELVIEW);
   glLoadIdentity();

   // Draw local grid
   glColor3d(0, 0, 1);
   for (int iy = 0; iy < Y_LOC_SIZE; ++iy)
   {
      glBegin(GL_LINE_STRIP);
      for (int ix = 0; ix < X_LOC_SIZE; ++ix)
         grid[ix][iy].draw();
      glEnd();
   }
   for (int ix = 0; ix < X_LOC_SIZE; ++ix)
   {
      glBegin(GL_LINE_STRIP);
      for (int iy = 0; iy < Y_LOC_SIZE; ++iy)
         grid[ix][iy].draw();
      glEnd();
   }

   // Draw model in green
   glColor3d(0, 1, 0);
   glBegin(GL_LINE_STRIP);
   for (int ip = 0; ip < MODEL_SIZE; ++ip)
      model[ip].draw();
   glEnd();

   // Draw distorted model in red
   glColor3d(1, 0, 0);
   glBegin(GL_LINE_STRIP);
   for (int ip = 0; ip < MODEL_SIZE; ++ip)
      localized[ip].draw();
   glEnd();

   glColor3d(1, 1, 1);
   showString(-0.9 * xFrame, -0.9 * yFrame);
   glutSwapBuffers();
}

void makeModel()
{
   for (int ip = 0; ip < MODEL_SIZE; ++ip)
   {
      double ang = (2 * PI * ip) / MODEL_SIZE;
      double x = 3 * cos(ang);
      double y = 0.8 * sin(ang) + 0.2 * sin(40 * ang);
      model[ip] = Point(x, y, 0);
      localized[ip] = Coord(x, y);
   }
}

double distortion = 0;

// Reconstruct local grid with distortion
void redraw()
{
   for (int ix = 0; ix < X_LOC_SIZE; ++ix)
   {
      double x = PI * ( 2 * ix / double(X_LOC_SIZE - 1) - 1);
      for (int iy = 0; iy < Y_LOC_SIZE; ++iy)
      {
         double y = 2 * iy / double(Y_LOC_SIZE - 1) - 1;
         if (y > 0) y += distortion * cos(x);
         if (y < 0) y -= distortion * cos(x);
         grid[ix][iy] = Point(x, y, 0);
      }
   }

   ostringstream os;
   os << "Distortion = " << distortion;
   message = os.str();
}

enum MOVING { STOP, UP, DOWN } moving = STOP;
double dk = 0.0001;

void idle ()
{
   switch (moving)
   {
      case UP:
         if (distortion >= 1)
            moving = DOWN;
         else
            distortion += dk;
         break;
      case DOWN:
         if (distortion <= -1)
            moving = UP;
         else
            distortion -= dk;
         break;
      case STOP:
      default:
         break;
   }
   redraw();
   glutPostRedisplay();
}

void keyboard (unsigned char key, int x, int y)
{
   switch (key)
   {
      case '+':
         if (distortion < 0.9) distortion += 0.1;
         redraw();
         glutPostRedisplay();
         break;

      case '-':
         if (distortion > -0.9) distortion -= 0.1;
         redraw();
         glutPostRedisplay();
         break;

      case 'm':
         switch (moving)
         {
            case UP:
               moving = STOP;
               break;
            case DOWN:
               moving = STOP;
               break;
            case STOP:
               moving = UP;
               break;
         }
      case 'r':
         distortion = 0;
         redraw();
         glutPostRedisplay();
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
   gluOrtho2D(-xFrame, xFrame, -yFrame, yFrame);
   glutPostRedisplay();
}

int main(int argc, char *argv[])
{
   cout <<
        "Deformation using a local grid\n\n"
        " +    increase distortion\n"
        " -    decrease distortion\n"
        " r    no distortion\n"
        " m    move, changing distortion continuously\n"
        " f    full screen\n"
        " q    quit\n"
        " ESC  quit\n";

   glutInit(&argc, argv);
   glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGB | GLUT_DEPTH);
   glutInitWindowSize(windowWidth, windowHeight);
   glutInitWindowPosition(300,0);
   glutCreateWindow("Deformation");
   glutDisplayFunc(display);
   glutIdleFunc(idle);
   glutKeyboardFunc(keyboard);
   glutReshapeFunc(reshape);
   redraw();
   makeModel();
   glutMainLoop();
   return 0;
}
