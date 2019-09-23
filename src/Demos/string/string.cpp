#include <GL/glut.h>
#include <iostream>
#include <sstream>
#include <cmath>
#include <cstdlib>

using namespace std;

// Initial size of graphics window.
const int WIDTH = 800;
const int HEIGHT = 400;

// Number of beads on beadString.
const int LENGTH = 200;

const double PI = 3.14159;
double DT = 0.001;
double K = 1000.0;

void* font1 = GLUT_BITMAP_TIMES_ROMAN_24;
void* font2 = GLUT_BITMAP_9_BY_15;
string title = "";

// Display 'title' as a bit-mapped character string.
void showString (GLfloat x, GLfloat y)
{
   glRasterPos2f(x, y);
   size_t len = title.size();
   for (size_t i = 0; i < len; i++)
      glutBitmapCharacter(font1, title[i]);
}


class Bead
{
public:
   Bead ()
   {
      x = 0.0;
      y = 0.0;
      v = 0.0;
      a = 0.0;
   }
   void update ()
   {
      v += a * DT;
      y += v * DT;
   }
   void set (double nx, double ny)
   {
      x = nx;
      y = ny;
      v = 0.0;
   }
   void setAcc (double na)
   {
      a = na;
   }
   double getX ()
   {
      return x;
   }
   double getY ()
   {
      return y;
   }
   friend ostream & operator<< (ostream & os, Bead b);
private:
   double x;
   double y;
   double v;
   double a;
};

ostream & operator<< (ostream & os, Bead b)
{
   os << "(" << b.x << "," << b.y << ") " << b.v;
   return os;
}

Bead beadString[LENGTH + 1];

// Set beadString to fundamental (n = 1) or harmonic (2 <= n <= 10)
void harmonic (int n)
{
   ostringstream os;
   os << "Harmonic " << n-1;
   title = os.str();
   beadString[0].set(0.0, 0.0);
   for (int i = 1; i < LENGTH; i++)
   {
      double x = double(i) / double(LENGTH);
      beadString[i].set(x, sin(n * PI * x));
   }
   beadString[LENGTH].set(1.0, 0.0);
}


// Set beadString to a shape determined by f(x).
void shape (double f (double))
{
   beadString[0].set(0.0, 0.0);
   for (int i = 1; i < LENGTH; i++)
   {
      double x = double(i) / double(LENGTH);
      beadString[i].set(x, f(x));
   }
   beadString[LENGTH].set(1.0, 0.0);
}


// Functions for various shapes on (0,1).

double flat (double x)
{
   title = "Harmonic (n) (T)riangle (S)awtooth s(Q)uare (I)mpulse (H)ump";
   return 0.0;
}

double triangle (double x)
{
   title = "Triangle";
   return x <= 0.5 ? 2.0 * x : 2.0 - 2.0 * x;
}

double sawtooth (double x)
{
   title = "Sawtooth";
   return x;
}

double square (double x)
{
   title = "Square";
   return x <= 0.5 ? 1.0 : -1.0;
}

double impulse (double x)
{
   title = "Impulse";
   return 0.495 < x && x < 0.505 ? 2.0 : 0.0;
}

double hump (double x)
{
   title = "Hump";
   double h = 20.0 * (x - 0.5);
   return 1.5 * exp(- (h*h));
}


// Display the beads.
void display ()
{
   glMatrixMode(GL_MODELVIEW);
   glLoadIdentity();
   glClear(GL_COLOR_BUFFER_BIT);
   glBegin(GL_LINE_STRIP);
   for (int i = 0; i <= LENGTH; i++)
   {
      if (i % 20 == 0)
         glColor3f(1.0, 1.0, 0.5);
      else
         glColor3f(0.0, 0.8, 1.0);
      glVertex2f(beadString[i].getX(), beadString[i].getY());
   }
   glEnd();

   glColor3f(1.0, 1.0, 0.5);
   showString(-0.08 ,1.3);
   glutSwapBuffers();
}


// Update velocity and position of each bead.
void idle ()
{
   for (int i = 1; i < LENGTH; i++)
      beadString[i].setAcc(K * (beadString[i-1].getY() + beadString[i+1].getY() - 2 * beadString[i].getY()));
   for (int i = 1; i < LENGTH; i++)
      beadString[i].update();
   glutPostRedisplay();
}


// Kepp beadString visible in window.
void reshape (int w, int h)
{
   glViewport(0, 0, w, h);
   glMatrixMode(GL_PROJECTION);
   glLoadIdentity();
   glOrtho(-0.1, 1.1, -1.5, 1.5, -1.0, 1.0);
}


// Set initial bead positions.
void keyboard (unsigned char key, int x, int y)
{
   switch (key)
   {

      case '0':
      case '1':
      case '2':
      case '3':
      case '4':
      case '5':
      case '6':
      case '7':
      case '8':
      case '9':
         harmonic(key + 1 - '0');
         break;

      case 'f':
         glutFullScreen();
         break;

      case 'h':
      case 'H':
         shape(hump);
         break;

      case 'i':
      case 'I':
         shape(impulse);
         break;

      case 'q':
      case 'Q':
         shape(square);
         break;

      case 's':
      case 'S':
         shape(sawtooth);
         break;

      case 't':
      case 'T':
         shape(triangle);
         break;

      case '-':
         DT *= 0.8;
         break;

      case '+':
         DT /= 0.8;
         break;

      case '?':
         shape(flat);
         break;

      case 27:
         exit(0);
   }
}


// Explain key effects.
void help ()
{
   cout << "Vibrating String Simulation" << endl << endl;
   cout << "Press one of the following keys to set the initial shape of the beadString:" << endl << endl;
   cout << "0   Fundamental" << endl;
   cout << "1   First harmonic" << endl;
   cout << "2   Second harmonic " << endl;
   cout << "3   Third harmonic " << endl;
   cout << "4   Fourth harmonic " << endl;
   cout << "h   Hump (travelling wave)" << endl;
   cout << "i   Impulse" << endl;
   cout << "q   Square" << endl;
   cout << "s   Sawtooth" << endl;
   cout << "t   Triangle" << endl;
   cout << "?   Flat with instructions" << endl;
   cout << "-   Slow down" << endl;
   cout << "+   Speed up" << endl;
}


// OpenGL initialization.
int main (int argc, char **argv)
{
   glutInit(&argc, argv);
   glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGB);
   glutInitWindowSize(int(WIDTH), int(HEIGHT));
   glutCreateWindow("Vibrating String");
   glutDisplayFunc(display);
   glutReshapeFunc(reshape);
   glutKeyboardFunc(keyboard);
   glutIdleFunc(idle);
   glPointSize(10.0);
   shape(flat);
   help();
   glutMainLoop();
   return 0;
}
