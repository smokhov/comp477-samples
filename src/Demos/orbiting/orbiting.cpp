/* Link with these libraries (omit those unneeded):
 *  libcugl
 *  libglui32
 *  libglut32
 *  libopengl32
 *  libglu32
 */

#include <io.h>
#include <iostream>
#include <iomanip>
#include <sstream>
#include <GL/cugl.h>
#include <GL/glut.h>
#include <windows.h>

using namespace std;
using namespace cugl;

// Initial window dimensions
int windowWidth = 1800;
int windowHeight = 800;

// Pixel array and quadric object for the sphere.
PixelMap pm;
GLUquadricObj *quad;

enum TextureMode { DECAL, MODULATE, REPLACE } texMode = MODULATE;
enum ClimbingMode { LEVEL, ASCEND, DESCEND } climbing = LEVEL;

const double TIME_STEP = 10; // milliseconds
const double TIMER_INTERVAL = 250; // milliseconds
const double EARTH_RADIUS = 6370; // kilometres
const double EARTH_CIRCUMFERENCE = 2 * PI * EARTH_RADIUS; // kilometres
const double MODEL_RADIUS = 10; // Radius of earth
const double MARKER_RADIUS = 1.05 * MODEL_RADIUS;
const double DIR_STEP = 0.01;

double height = 0.1 * MODEL_RADIUS; // Height above surface
double cameraTilt = radians(41);
double earthRotation = 0;
double rotRate = 0.0002; // radians per frame
Vector axis; // Vector pointing at viewer in earth's frame
Quaternion ori; // Orientation of the earth
int steps = 0; // Count idle steps for speed calculation.
double speed; // Measured speed

// Save values for extremely far view
bool high = false;
double saveHeight;
double saveCameraTilt;

// Names of texture files.
const int MAXNAMES = 200;
char *names[MAXNAMES];
int numNames = 0;

void timer(int code)
{
   // Speed calculation:
   //   1000 milliseconds per second
   //   steps = number of times orientation has been incremented
   //   rotRate = radians per step
   speed = (1000 * steps * rotRate * EARTH_CIRCUMFERENCE) / (2 * PI * TIMER_INTERVAL);
   steps = 0;
   glutTimerFunc(TIMER_INTERVAL, timer, 0);
}

void changeDir(double dir)
{
   // Rotate the earth about an axis pointing towards the viewer
   axis = Vector(0, sin(earthRotation), cos(earthRotation));
   Quaternion d(axis, DIR_STEP * dir);
   ori = d * ori;
}

void showTitle(string message = "")
{
   if (message.size() > 0)
      glutSetWindowTitle(message.c_str());
   else
   {
      ostringstream os;
      os << fixed << setprecision(0);
      os <<
      "Speed: " << speed <<
      " km/s.  Height: " << (EARTH_RADIUS * height / MODEL_RADIUS) <<
      " km.  Camera: " << degrees(cameraTilt) << " degrees.  ";
      switch (texMode)
      {
         case DECAL:
            os << "Decal.";
            break;
         case MODULATE:
            os << "Modulate.";
            break;
         case REPLACE:
            os << "Replace.";
            break;
      }
      glutSetWindowTitle(os.str().c_str());
   }
}

// GLUT callbacks.

void display (void)
{
   glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
   glMatrixMode(GL_MODELVIEW);
   glLoadIdentity();

   // Orient and position camera
   glRotated(degrees(cameraTilt), -1, 0, 0);
   glTranslated(0, 0, -(MODEL_RADIUS + height));

   // Move over the earth
   glRotated(degrees(earthRotation), 1, 0, 0);

   // Show marker over current position
   if (MODEL_RADIUS + height > 1.2 * MARKER_RADIUS)
   {
      glPushMatrix();
      glTranslated(0,  MARKER_RADIUS * sin(earthRotation), MARKER_RADIUS * cos(earthRotation));
      glColor3d(1, 1, 1);
      glDisable(GL_TEXTURE_2D);
      glutSolidSphere(0.02, 25, 25);
      glPopMatrix();
   }

   // Rotate the earth to give direction and display it
   ori.apply();
   glEnable(GL_TEXTURE_2D);
   gluSphere(quad, MODEL_RADIUS, 5000, 5000);

   glutSwapBuffers();
}

void idle()
{
   steps++;
   earthRotation += rotRate;
   switch (climbing)
   {
      case ASCEND:
         height += 0.0001 * MODEL_RADIUS;
         showTitle();
         break;
      case LEVEL:
         break;
      case DESCEND:
         height -= 0.0001 * MODEL_RADIUS;
         if (height < 0.1) height = 0.1;
         showTitle();
         break;
   }
   glutPostRedisplay();
}

void keyboard (unsigned char key, int x, int y)
{
   switch (key)
   {

      case 'a':
         climbing = ASCEND;
         break;

      case 'd':
         climbing = DESCEND;
         break;

      case 'l':
         climbing = LEVEL;
         break;

      case 'c': // close
         climbing = LEVEL;
         height = 0.1 * MODEL_RADIUS;
         cameraTilt = radians(41);
         break;

      case 'v': // very close
         climbing = LEVEL;
         height = 0.01 * MODEL_RADIUS;
         cameraTilt = 1;
         break;

      case 'q': // quicker
         rotRate *= 1.1;
         break;

      case 's': // slower
         rotRate /= 1.1;
         break;

      case 't':
         switch (texMode)
         {
            case DECAL:
               texMode = MODULATE;
               break;
            case MODULATE:
               texMode = REPLACE;
               break;
            case REPLACE:
               texMode = DECAL;
               break;
         }
         glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, texMode);
         break;

      case 'x': // far
         climbing = LEVEL;
         if (high)
         {
            height = saveHeight;
            cameraTilt = saveCameraTilt;
            high = false;
         }
         else
         {
            saveHeight = height;
            saveCameraTilt = cameraTilt;
            height = 1.5 * MODEL_RADIUS;
            cameraTilt = 0.0;
            high = true;
         }
         break;

      case 27:
         exit(0);
         break;
   }
   glutPostRedisplay();
   showTitle();
}

void funcKeys (int key, int x, int y)
{
   switch (key)
   {
      case GLUT_KEY_UP:
         cameraTilt += 0.02;
         break;
      case GLUT_KEY_DOWN:
         cameraTilt -= 0.02;
         break;
      case GLUT_KEY_LEFT:
         changeDir(-1);
         break;
      case GLUT_KEY_RIGHT:
         changeDir(1);
         break;
   }
   showTitle();
}

void reshape (int w, int h)
{
   windowWidth = w;
   windowHeight = h;
   glViewport(0, 0, windowWidth, windowHeight);
   glMatrixMode(GL_PROJECTION);
   glLoadIdentity();
   gluPerspective(50, double(w)/double(h), 0.1, 50);
   glutPostRedisplay();
}

void menu(int arg)
{
   showTitle("Please wait while map loads ...");
   pm.read(names[arg]);
   checkCUGLStatus();
   if (pm.badSize())
   {
      showTitle("BAD SIZE!  This map cannot be used as a texture.");
      return;
   }
   cout << pm << endl;
   GLuint name;
   glGenTextures(1, &name);
   pm.setTexture(name);
   showTitle();
}

int main(int argc, char *argv[])
{
   glutInit(&argc, argv);
   glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGB | GLUT_DEPTH);
   glutInitWindowSize(windowWidth, windowHeight);
   glutCreateWindow("Flying");
   glutDisplayFunc(display);
   glutKeyboardFunc(keyboard);
   glutSpecialFunc(funcKeys);
   glutReshapeFunc(reshape);
   glutIdleFunc(idle);
   glutTimerFunc(TIMER_INTERVAL, timer, 0);
   glEnable(GL_DEPTH_TEST);
   glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, texMode);

   quad = gluNewQuadric();
   gluQuadricTexture(quad, GL_TRUE);

   // Look for .bmp files in the working directory.
   struct _finddata_t fd;
   long dir = _findfirst("*.bmp", &fd);
   while (true)
   {
      names[numNames] = new char[strlen(fd.name) + 1];
      strcpy(names[numNames], fd.name);
      numNames++;
      if (_findnext(dir, &fd))
         break;
   }
   if (numNames == 0)
   {
      cerr << "There are no BMP files in the current directory.\n";
      return 1;
   }

   // Menu for names.
   glutCreateMenu(menu);
   for (int i = 0; i < numNames; i++)
      glutAddMenuEntry(names[i], i);
   glutAttachMenu(GLUT_RIGHT_BUTTON);

   // Instructions for users.
   cout <<
        "Flying simulation\n\n"
        " a  Ascend\n"
        " d  Descend\n"
        " l  fly Level\n"
        " c  Close to ground (about 650 km)\n"
        " v  Very close to ground (about 64 km)\n"
        " q  go Quicker\n"
        " s  go Slower\n"
        " t  change Texture mode\n"
        " x  toggle eXtreme far view\n\n"
        "Arrows:\n"
        " up     tilt camera up\n"
        " down   tilt camera down\n"
        " left   steer left\n"
        " right  steer right\n\n"
        "Click right mouse button to select a file." <<
        endl;

   glutMainLoop();
   return 0;
}
