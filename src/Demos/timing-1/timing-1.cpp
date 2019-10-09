/* Constant speed animation.
 * Peter Grogono
 * COMP 477
 * 21 Decemeber 2010
 *
 * Link with:
 *  libglut32
 *  libopengl32
 *  libglu32
 */

#include <GL/glut.h>
#include <windows.h>
#include <iostream>
#include <sstream>
#include <iomanip>
using namespace std;

int windowWidth = 800;
int windowHeight = 800;

const double DT = 1; // Time increment in milliseconds
const double DEGREES_PER_REVOLUTION = 360;
const double MILI = 0.001;

class Model
{
public:

   Model() : time(0), detail(10) {}

   void change(char key)
   {
      switch (key)
      {

         case '+':
            detail *= 1.5;
            break;
         case '-':
            if (detail >= 10)
               detail /= 1.5;
            break;
      }
   }

   void draw()
   {
      double angle = DEGREES_PER_REVOLUTION * MILI * time;
      if (angle > DEGREES_PER_REVOLUTION)
         angle -= DEGREES_PER_REVOLUTION;
      glRotated(angle, 0, 1, 0);
      for (double x = -4; x <= 4; x += 1)
         for (double y = -4; y <= 4; y += 1)
         {
            glPushMatrix();
            glTranslated(x, y, 0);
            glRotated(-angle, 0, 1, 0); // Balls do not rotate
            glutWireSphere(0.4, int(detail), int(detail));
            glPopMatrix();
         }
   }

   void update()
   {
      time += DT;
   }

   int getFaces()
   {
      return int(detail * detail);
   }

private:
   double time;
   double detail;
};

Model model;

void display (void)
{
   glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
   glMatrixMode(GL_MODELVIEW);
   glLoadIdentity();
   glTranslated(0, 0, -20);
   model.draw();
   glutSwapBuffers();
}

void idle()
{
   static double prevTime = GetTickCount();
   static double accumulatedTime = 0;
   double currTime = GetTickCount();
   double elapsedTime = currTime - prevTime;
   if (elapsedTime > 0)
   {
      accumulatedTime += elapsedTime;
      while (accumulatedTime >= DT)
      {
         model.update();
         accumulatedTime -= DT;
      }
      prevTime = currTime;
   }
   glutPostRedisplay();

   ostringstream os;
   os << fixed << setprecision(3) <<
   "Displaying about " << model.getFaces() << " triangles.";
   glutSetWindowTitle(os.str().c_str());
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
        "Constant speed animation.\n\n"
        "Press:\n"
        "   '+' to increase detail.\n"
        "   '-' to decrease detail.\n"
        "   'f' for full screen.\n"
        "   ESC to quit.";

   glutInit(&argc, argv);
   glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGB | GLUT_DEPTH);
   glutInitWindowSize(windowWidth, windowHeight);
   glutInitWindowPosition(300, 300);
   glutCreateWindow("");
   glutDisplayFunc(display);
   glutIdleFunc(idle);
   glutKeyboardFunc(keyboard);
   glutReshapeFunc(reshape);
   glutMainLoop();
   return 0;
}
