/* Vibrating membrane
   Link with:
     libcugl
     libglut32
     libopengl32
     libglu32
*/

#include <GL/glut.h>
#include <GL/cugl.h>
#include <cmath>
#include <iostream>
#include <iomanip>
#include <sstream>

using namespace std;
using namespace cugl;

// Menu constants
enum
{
   SMALL,
   MEDIUM,
   LARGE,
   SIDE,
   CENTRE,
   OFFCENTRE,
   SOFT,
   HARD,
   NONE,
   CORNER,
   ONESIDE,
   TWOSIDES,
   FOURSIDES,
   INCREMENT,
   DECREMENT,
   POLISHED,
   DULL,
   BG_BLACK,
   BG_BLUE
};

// Constant settings for motion parameters.
const int MALLETSMALL = 1;
const int MALLETMEDIUM = 3;
const int MALLETLARGE = 10;
const int FORCESOFT = 1;
const int FORCEMEDIUM = 5;
const int FORCEHARD = 10;

// Physics

// Membrane dimensions
const int N = 75;
const int H = N / 2;
const int DIM = N + 1;
typedef double MAT[DIM][DIM];

// Membrane acceleration, velocity, position, and normal vector.
MAT acc, vel, z;
Vector normal[DIM][DIM];

// Time step for integration.
double dt = 0.05;

// Values of motion parameters: chosen by menuMain selection.
int xImpact = H;
int yImpact = H;
int malletRadius = MALLETLARGE;
int amplitude = FORCEHARD;
int clamp = FOURSIDES;

// Strings used to display values
string malletString = "large.  ";
string forceString = "hard.  ";
string impactString = "centre.  ";
string clampString = "four sides.  ";

// Vibration mode.
enum VibrationMode
{
   IMPACT,
   GETTING_Y,
   NORMAL_MODES
} mode = IMPACT;
int xq = 2;
int yq = 3;

// Determines membrane motion.
bool animation = true;

// Utility functions.

//inline double sqr(double x)
//{
//   return x * x;
//}

inline bool odd(int n)
{
   return n % 2 == 1;
}

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

// Useful functions.

// Set membrane to equilibrium (planar) position.
void initialize()
{
   animation = false;
   for (int i = 0; i <= N; i++)
      for (int j = 0; j <= N; j++)
      {
         acc[i][j] = 0;
         vel[i][j] = 0;
         z[i][j] = 0;
         normal[i][j] = Vector(0,0,1);
      }
}

// Compute next position of membrane from current position.
void update()
{
   int i, j;

   // Find accelerations

   // Corner points
   acc[0][0] = z[0][1]   + z[1][0]   - 2 * z[0][0];
   acc[N][0] = z[N][1]   + z[N-1][0] - 2 * z[N][0];
   acc[0][N] = z[1][N]   + z[0][N-1] - 2 * z[0][N];
   acc[N][N] = z[N-1][N] + z[N][N-1] - 2 * z[N][N];

   // Side points
   for (i = 1; i < N; i++)
   {
      acc[i][0] = z[i-1][1] + z[i][1] + z[i+1][1] +
                  z[i-1][0]             + z[i+1][0]       - 5 * z[i][0];
      acc[0][i] = z[1][i-1] + z[1][i] + z[1][i+1] +
                  z[0][i-1]             + z[0][i+1]       - 5 * z[0][i];
      acc[i][N] = z[i-1][N-1] + z[i][N-1] + z[i+1][N-1] +
                  z[i-1][N  ]               + z[i+1][N  ] - 5 * z[i][N];
      acc[N][i] = z[N-1][i-1] + z[N-1][i] + z[N-1][i+1] +
                  z[N  ][i-1]               + z[N  ][i+1] - 5 * z[N][i];
   }

   // Middle points
   for (i = 1; i < N; i++)
      for (j = 1; j < N; j++)
         acc[i][j] = z[i-1][j-1] + z[i-1][j] + z[i-1][j+1] +
                     z[i  ][j-1]               + z[i  ][j+1] +
                     z[i+1][j-1] + z[i+1][j] + z[i+1][j+1] - 8 * z[i][j];

   // Integrate acceleration to obtain velocity.
   for (i = 0; i <= N; i++)
      for (j = 0; j <= N; j++)
         vel[i][j] += dt * acc[i][j];

   // Integrate velocity to obtain position
   // without moving clamped points.
   switch (clamp)
   {
      case NONE:
         for (i = 0; i <= N; i++)
            for (j = 0; j <= N; j++)
               z[i][j] += dt * vel[i][j];
         break;
      case CORNER:
         for (i = 0; i <= N; i++)
            for (j = 0; j <= N; j++)
               if (
                  (i != 0 || j != 0) &&
                  (i != 0 || j != N) &&
                  (i != N || j != 0) &&
                  (i != N || j != N) )
                  z[i][j] += dt * vel[i][j];
         break;
      case ONESIDE:
         for (i = 1; i <= N; i++)
            for (j = 0; j <= N; j++)
               z[i][j] += dt * vel[i][j];
         break;
      case TWOSIDES:
         for (i = 1; i <= N-1; i++)
            for (j = 0; j <= N; j++)
               z[i][j] += dt * vel[i][j];
         break;
      case FOURSIDES:
         for (i = 1; i <= N-1; i++)
            for (j = 1; j <= N-1; j++)
               z[i][j] += dt * vel[i][j];
         break;
      case CENTRE:
         for (i = 0; i <= N; i++)
            for (j = 0; j <= N; j++)
               if (i != H || j != H)
                  z[i][j] += dt * vel[i][j];
         break;
   }

   // Calculate normals

   // Corner points
   normal[0][0] = Vector(z[1][0] - z[0][0],   z[0][0] - z[0][1], 1).unit();
   normal[N][0] = Vector(z[N-1][0] - z[N][0], z[N][0] - z[N][1], 1).unit();
   normal[0][N] = Vector(z[1][N] - z[0][N],  z[0][N] - z[0][N-1], 2).unit();
   normal[N][N] = Vector(z[N-1][N] - z[N][N], z[N][N] - z[N][N-1], 2).unit();

   // Sides
   for (i = 1; i <= N-1; i++)
   {
      normal[i][0] = Vector(z[i-1][0] - z[i+1][0], 0, 2).unit();
      normal[i][N] = Vector(z[i-1][N] - z[i+1][N], 0, 2).unit();
   }

   for (j = 1; j <= N-1; j++)
   {
      normal[0][j] = Vector(0, z[0][j-1] - z[0][j+1], 2).unit();
      normal[N][j] = Vector(0, z[N][j-1] - z[N][j+1], 2).unit();
   }

   // Interior points

   /* Fast version.  Less accurate, and not really fast enough to be useful.
      for (i = 1; i <= N-1; i++)
         for (j = 1; j <= N-1; j++)
            normal[i][j] = Vector(z[i-1][j] - z[i+1][j], z[i][j-1] - z[i][j+1], 4).unit();
   */

   // Normals computed using Newell's algorithm and averaging.
   for (i = 1; i <= N-1; i++)
      for (j = 1; j <= N-1; j++)
         normal[i][j] = Vector
                        (
                           z[i-1][j+1] - z[i+1][j+1] + 2*z[i-1][j] - 2*z[i+1][j] + z[i-1][j-1] - z[i+1][j-1],
                           - z[i-1][j+1] - 2*z[i][j+1] - z[i+1][j+1] + z[i-1][j-1] + 2*z[i][j-1] + z[i+1][j-1],
                           8
                        ).unit();
}


// Graphics

// Initial size of graphics window.
const int WIDTH  = 900;
const int HEIGHT = 600;

// Current size of window.
int width  = WIDTH;
int height = HEIGHT;

// Mouse positions, unitd to [0,1].
double xMouse = 0;
double yMouse = 0;

// Bounds of viewing frustum.
double nearPlane = 5;
double farPlane  = 4 * N;

// Viewing angle.
double fovy = 40.0;

// Light properties (controlled by mouse position)

GLfloat lightPosition[] = { H, N, 0, 1 };
GLfloat lightDiffuse[]  = { 1, 1, 1, 1 };
GLfloat lightAmbient[]  = { 1, 1, 1, 1 };
GLfloat lightSpecular[] = { 1, 1, 1, 1 };

// Constants for quaternion rotation.
const double DELTA = PI/100;
const double SINDELTA = sin(DELTA);
const double COSDELTA = cos(DELTA);
const Quaternion UP = Quaternion(COSDELTA, I * SINDELTA);
const Quaternion DOWN = UP.inv();
const Quaternion LEFT = Quaternion(COSDELTA, K * SINDELTA);
const Quaternion RIGHT = LEFT.inv();

// Variable for quaternion rotation.
Quaternion q(-1, 0, 0);

// Flag for diaplying normals.
bool showNormals = false;


// Display the membrane.
void display ()
{
   glMatrixMode(GL_PROJECTION);
   glLoadIdentity();
   gluPerspective(fovy, GLfloat(width) / GLfloat(height), nearPlane, farPlane);
   glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
   glMatrixMode(GL_MODELVIEW);
   glLoadIdentity();
   glTranslatef(0, 0, - 3 * H);
   glEnable(GL_LIGHTING);
   glLightfv(GL_LIGHT0, GL_POSITION, lightPosition);
   q.apply();
   glTranslatef(-H, -H, 0);

   // Display membrane
   glBegin(GL_QUADS);
   for (int i = 0; i < N; i++)
      for (int j = 0; j < N; j++)
      {
         normal[i][j].drawNormal();
         glVertex3f(i, j, z[i][j]);

         normal[i+1][j].drawNormal();
         glVertex3f(i+1,j, z[i+1][j]);

         normal[i+1][j+1].drawNormal();
         glVertex3f(i+1, j+1, z[i+1][j+1]);

         normal[i][j+1].drawNormal();
         glVertex3f(i, j+1, z[i][j+1]);
      }
   glEnd();

   if (showNormals)
   {
      glDisable(GL_LIGHTING);
      glColor3f(1,1,1);
      glBegin(GL_LINES);
      for (int i = 0; i <= N; i++)
         for (int j = 0; j <= N; j++)
         {
            Point p(i, j, z[i][j]);
            Point q = p + normal[i][j];
            p.draw();
            q.draw();
         }
      glEnd();
      glEnable(GL_LIGHTING);
   }

   glMatrixMode(GL_PROJECTION);
   glLoadIdentity();
   glOrtho(0, 5, -0.1, 1, -1.0, 1.0);
   glMatrixMode(GL_MODELVIEW);
   glLoadIdentity();
   glDisable(GL_LIGHTING);
   glColor3d(1, 1, 1);
   showString(0, 0);
   glutSwapBuffers();
}

// Move membrane and redisplay it.
void idle ()
{
   if (animation)
   {
      update();
      glutPostRedisplay();
   }
}

// Update title bar of graphics window.
void setTitle()
{
   ostringstream os;
   os << fixed << setprecision(2);
   switch (mode)
   {
      case IMPACT:
         os <<
         "Mallet: " << malletRadius << ' ' << malletString <<
         "Impact: " << impactString <<
         "Force: " << amplitude << ' ' << forceString;
         break;

      case GETTING_Y:
         os <<
         "X mode: " << xq << ".  Enter Y value." << ends;
         break;

      case NORMAL_MODES:
         os <<
         "X mode: " << xq <<
         ".   Y mode: " << yq <<
         ".   Amplitude: " << forceString;
         break;
   }

   os <<
   "Clamping: " << clampString <<
   "dt = " << setprecision(4) << dt <<
   ".   Light at: (" << setprecision(1) <<
   lightPosition[0] << ", " <<
   lightPosition[1] << ", " <<
   lightPosition[2] << ")." <<
   ends;
   title = os.str();
}

// Move light in response to mouse movement.
void mouseMovement (int mx, int my)
{
   lightPosition[0] = N * (double(mx) / double(width) - 0.5);
   lightPosition[2] = 3 * N * (double(my) / double(height) - 0.5);
   setTitle();
   glutPostRedisplay();
}

// Respond to window resizing, preserving proportions.
// Parameters give new window size in pixels.
void reshapeMainWindow (int newWidth, int newHeight)
{
   width = newWidth;
   height = newHeight;
   glViewport(0, 0, width, height);
}

// Move membrane to initial position.
// Called when space bar struck.
void setMembrane()
{
   int i, j;
   initialize();
   switch (mode)
   {
      case IMPACT:
         for (i = xImpact - malletRadius; i <= xImpact + malletRadius; i++)
            if (0 <= i && i <= N)
               for (j = yImpact - malletRadius; j <= yImpact + malletRadius; j++)
                  if (0 <= j && j <= N)
                  {
                     double sq = sqr(malletRadius) - sqr(i - xImpact) - sqr(j - yImpact);
                     if (sq >= 0)
                     {
                        double h = amplitude - malletRadius + sqrt(sq);
                        if (h > 0)
                           z[i][j] = h;
                     }
                  }
         break;

      case GETTING_Y:
         break;

      case NORMAL_MODES:
         switch (clamp)
         {
            case NONE:
               if (xq == 0)
                  for (i = 0; i <= N; i++)
                     for (j = 0; j <= N; j++)
                        z[i][j] = amplitude * cos(j * yq * PI / N);
               else if (yq == 0)
                  for (i = 0; i <= N; i++)
                     for (j = 0; j <= N; j++)
                        z[i][j] = amplitude * cos(i * xq * PI / N);
               else
                  for (i = 0; i <= N; i++)
                     for (j = 0; j <= N; j++)
                        z[i][j] = amplitude * cos(i * xq * PI / N) * cos(j * yq * PI / N);
               break;

            case ONESIDE:
            case TWOSIDES:
               if (xq == 0)
                  for (i = 0; i <= N; i++)
                     for (j = 0; j <= N; j++)
                        z[i][j] = amplitude * cos(j * yq * PI / N);
               else if (yq == 0)
                  for (i = 0; i <= N; i++)
                     for (j = 0; j <= N; j++)
                        z[i][j] = amplitude * sin(i * xq * PI / N);
               else
                  for (i = 0; i <= N; i++)
                     for (j = 0; j <= N; j++)
                        z[i][j] = amplitude * sin(i * xq * PI / N) * cos(j * yq * PI / N);
               break;

            case FOURSIDES:
               for (i = 0; i <= N; i++)
                  for (j = 0; j <= N; j++)
                     z[i][j] = amplitude * sin(i * xq * PI / N) * sin(j * yq * PI / N);
               break;

            case CENTRE:
               if (odd(xq) || odd(yq))
                  for (i = 0; i <= N; i++)
                     for (j = 0; j <= N; j++)
                        z[i][j] = amplitude * cos(i * xq * PI / N) * cos(j * yq * PI / N);
               break;
         }
         break;
   }
}

// Respond to graphic character keys.
void graphicKeys (unsigned char key, int x, int y)
{
   switch (key)
   {

      case 13:
      case ' ':
         setMembrane();
         setTitle();
         animation = true;
         break;

      case '+':
         dt /= 0.8;
         setTitle();
         break;

      case '-':
         dt *= 0.8;
         setTitle();
         break;

      case 'n':
         showNormals = ! showNormals;
         glutPostRedisplay();
         break;

      case 'p':
         animation = ! animation;
         break;

      case 'r':
         q = Quaternion(-1, 0, 0);
         glutPostRedisplay();
         break;

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
         switch (mode)
         {
            case NORMAL_MODES:
            case IMPACT:
               xq = key - '0';
               mode = GETTING_Y;
               break;
            case GETTING_Y:
               yq = key - '0';
               mode = NORMAL_MODES;
               break;
         }
         setTitle();
         break;

      case 'q':
      case 27:
         exit(0);
   }
}

// Respond to function keys.
void functionKeys (int key, int x, int y)
{
   switch (key)
   {

      case GLUT_KEY_UP:
         q *= UP;
         glutPostRedisplay();
         break;

      case GLUT_KEY_DOWN:
         q *= DOWN;
         glutPostRedisplay();
         break;

      case GLUT_KEY_LEFT:
         q *= LEFT;
         glutPostRedisplay();
         break;

      case GLUT_KEY_RIGHT:
         q *= RIGHT;
         glutPostRedisplay();
         break;
   }
}

// Menu functions

// Dummy function for top-level menuMain.
void menuMain(int choice)
{
}

void menuRadius(int choice)
{
   switch (choice)
   {
      case SMALL:
         malletRadius = MALLETSMALL;
         malletString = "small.   ";
         break;
      case MEDIUM:
         malletRadius = MALLETMEDIUM;
         malletString = "medium.  ";
         break;
      case LARGE:
         malletRadius = MALLETLARGE;
         malletString = "large.   ";
         break;
      case INCREMENT:
         malletRadius += 1;
         break;
      case DECREMENT:
         if (malletRadius > 1)
            malletRadius -= 1;
         break;
   }
   mode = IMPACT;
   initialize();
   setTitle();
}

void menuImpactPosition(int choice)
{
   switch (choice)
   {
      case CORNER:
         xImpact = N;
         yImpact = N;
         impactString = "corner.      ";
         break;
      case SIDE:
         xImpact = H;
         yImpact = 0;
         impactString = "side.        ";
         break;
      case OFFCENTRE:
         xImpact = H / 2;
         yImpact = H;
         impactString = "off-centre.  ";
         break;
      case CENTRE:
         xImpact = H;
         yImpact = H;
         impactString = "centre.      ";
         break;
   }
   mode = IMPACT;
   initialize();
   setTitle();
}

void menuForce(int choice)
{
   switch (choice)
   {
      case SOFT:
         amplitude = FORCESOFT;
         forceString = "soft.    ";
         break;
      case MEDIUM:
         amplitude = FORCEMEDIUM;
         forceString = "medium.  ";
         break;
      case HARD:
         amplitude = FORCEHARD;
         forceString = "hard.    ";
         break;
      case INCREMENT:
         amplitude += 1;
         break;
      case DECREMENT:
         if (amplitude > 1)
            amplitude -= 1;
         break;
   }
   initialize();
   setTitle();
}

void menuClampMode(int choice)
{
   clamp = choice;
   switch (choice)
   {
      case NONE:
         clampString = "none.        ";
         break;
      case CORNER:
         clampString = "corners.     ";
         break;
      case ONESIDE:
         clampString = "one side.    ";
         break;
      case TWOSIDES:
         clampString = "two sides.   ";
         break;
      case FOURSIDES:
         clampString = "four sides.  ";
         break;
      case CENTRE:
         clampString = "centre.      ";
         break;
   }
   initialize();
   setTitle();
}

void menuDrawMode(int choice)
{
   glPolygonMode(GL_FRONT_AND_BACK, choice);
}

void menuShowNormals(int choice)
{
   switch (choice)
   {
      case GL_FALSE:
         showNormals = false;
         break;
      case GL_TRUE:
         showNormals = true;
         break;
   }
}

void menuMaterials(int choice)
{
   switch (choice)
   {
      case POLISHED:
         setMaterial(POLISHED_GOLD, GL_FRONT);
         setMaterial(POLISHED_SILVER, GL_BACK);
         break;
      case DULL:
         setMaterial(BRONZE, GL_FRONT);
         setMaterial(METAL, GL_BACK);
         break;
   }
}

void menuBackground(int choice)
{
   switch (choice)
   {
      case BG_BLACK:
         glClearColor(0, 0, 0, 0);
         break;
      case BG_BLUE:
         glClearColor(0.7, 0.8, 1, 1);
         break;
   }
}

// End of functions.

int main (int argc, char **argv)
{
   cout <<
        "Elastic Membrane simulation\n\n"
        "SP/CR    hit membrane\n"
        "Arrows   tilt membrane\n"
        "Numbers  harmonic motion\n"
        " n       toggle normals\n"
        " p       pause motion\n"
        " r       reset orientation\n"
        " +       faster\n"
        " -       slower\n"
        "Click rightfor menu\n";

   // Initialize membrane;
   initialize();

   // Initialize GLUT
   glutInit(&argc, argv);
   glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGB);
   glutInitWindowSize(width, height);
   glutCreateWindow("");

   // Register callbacks.
   glutDisplayFunc(display);
   glutReshapeFunc(reshapeMainWindow);
   glutKeyboardFunc(graphicKeys);
   glutSpecialFunc(functionKeys);
   glutMotionFunc(mouseMovement);
   glutIdleFunc(idle);

   // Set up menus.
   int mRadius = glutCreateMenu(menuRadius);
   glutAddMenuEntry("Small", SMALL);
   glutAddMenuEntry("Medium", MEDIUM);
   glutAddMenuEntry("Large", LARGE);
   glutAddMenuEntry("Increase", INCREMENT);
   glutAddMenuEntry("Decrease", DECREMENT);

   int mImpactPos = glutCreateMenu(menuImpactPosition);
   glutAddMenuEntry("Corner", CORNER);
   glutAddMenuEntry("Side", SIDE);
   glutAddMenuEntry("Off-centre", OFFCENTRE);
   glutAddMenuEntry("Centre", CENTRE);

   int mForce = glutCreateMenu(menuForce);
   glutAddMenuEntry("Soft", SOFT);
   glutAddMenuEntry("Medium", MEDIUM);
   glutAddMenuEntry("Hard", HARD);
   glutAddMenuEntry("Increase", INCREMENT);
   glutAddMenuEntry("Decrease", DECREMENT);

   int mClamp = glutCreateMenu(menuClampMode);
   glutAddMenuEntry("No clamping", NONE);
   glutAddMenuEntry("Corners", CORNER);
   glutAddMenuEntry("One side", ONESIDE);
   glutAddMenuEntry("Two sides", TWOSIDES);
   glutAddMenuEntry("Four sides", FOURSIDES);
   glutAddMenuEntry("Centre", CENTRE);

   int mDraw = glutCreateMenu(menuDrawMode);
   glutAddMenuEntry("Fill", GL_FILL);
   glutAddMenuEntry("Lines", GL_LINE);
   glutAddMenuEntry("Points", GL_POINT);

   int mShowNormals = glutCreateMenu(menuShowNormals);
   glutAddMenuEntry("Hide normals", GL_FALSE);
   glutAddMenuEntry("Show normals", GL_TRUE);

   int mMaterials = glutCreateMenu(menuMaterials);
   glutAddMenuEntry("Polished", POLISHED);
   glutAddMenuEntry("Dull", DULL);

   int mBackground = glutCreateMenu(menuBackground);
   glutAddMenuEntry("Black", BG_BLACK);
   glutAddMenuEntry("Blue", BG_BLUE);

   glutCreateMenu(menuMain);
   glutAddSubMenu("Mallet radius", mRadius);
   glutAddSubMenu("Impact position", mImpactPos);
   glutAddSubMenu("Force", mForce);
   glutAddSubMenu("Clamping", mClamp);
   glutAddSubMenu("Drawing", mDraw);
   glutAddSubMenu("Normals", mShowNormals);
   glutAddSubMenu("Materials", mMaterials);
   glutAddSubMenu("Background", mBackground);

   glutAttachMenu(GLUT_RIGHT_BUTTON);

   menuShowNormals(GL_FALSE);
   menuMaterials(POLISHED);
   menuBackground(BG_BLACK);

   // Set up lighting.
   glEnable(GL_DEPTH_TEST);
   glEnable(GL_LIGHT0);
   glEnable(GL_LIGHTING);
   glLightfv(GL_LIGHT0, GL_DIFFUSE, lightDiffuse);
   glLightfv(GL_LIGHT0, GL_AMBIENT, lightAmbient);
   glLightfv(GL_LIGHT0, GL_SPECULAR, lightSpecular);
   glLightModeli(GL_LIGHT_MODEL_TWO_SIDE, GL_TRUE);

   // Define material properties for membrane.
   setMaterial(POLISHED_GOLD, GL_FRONT);
   setMaterial(POLISHED_SILVER, GL_BACK);

   // Set main window title and start GLUT event loop.
   setTitle();
   glutMainLoop();
   return 0;
}
