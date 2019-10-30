/* Water waves
   Link with:
     libcugl
     libglut32
     libopengl32
     libglu32
*/

#include <GL/glut.h>
#include "cugl.h"
#include <cmath>
#include <fstream>
#include <iostream>
#include <iomanip>
#include <sstream>

using namespace std;
using namespace cugl;

// Membrane dimensions
const int N = 100;
const int H = N / 2;
const int DIM = N + 1;
double z[DIM][DIM];       // Height array
Vector normal[DIM][DIM];  // Normal vector for each point

double time_ = 0; // Time and step for integration.

const int WIDTH  = 900;  // Initial width of graphics window.
const int HEIGHT = 600;  // Initial height of graphics window.

int width  = WIDTH;   // Current width of graphics window.
int height = HEIGHT;  // Current height of graphics window.

double nearPlane = 5;      // Distance to closest visible objects.
double farPlane  = 4 * N;  // Distance to furthest visible objects.

GLfloat sun[]  = { 1, 1, 0.8, 1 };           // Bright yellow sunlight
GLfloat ambient[]  = { 0.2, 0.2, 0.16, 1 };  // Ambient = dim sun
GLfloat diffuse[]  = { 0, 0.2, 0.4, 1 };     // Approximate sea-water colour
GLfloat specular[] = { 1, 1, 1, 1 };         // Highlights match sun
GLfloat shininess[] = { 120 };               // Very shiny!

const char MAX_MODE = '4';   // Display modes
const int MAX_PRESETS = 10;  // Stored parameter settings
const int MAX_SAMPLES = 50;  // Components for complex waves

// Parameters for menu selection
const int P_NONE = 0;
const int P_AMP = 1;
const int P_AMP_RATIO = 2;
const int P_LEN = 3;
const int P_LEN_RATIO = 4;
const int P_ANGLE = 5;
const int P_DT = 6;
const int P_SAMPLES = 7;
const int P_FOV = 8;
const int P_MODE = 9;

const int TITLE_DISPLAY_TIME = 3000; // Display title for 3 seconds.
const string fileName = "params.txt";
const double FACTOR = 1.02;          // Ratio for changes

bool fillPolys = true;      // Polygon or line display
bool showNormals = false;   // Normal display
bool showTitle = false;     // On-screen title display
string title;               // Current title

int param = P_NONE;         // Current parameter
double fovy = 20;           // Vertical field-of-view
double dir[MAX_SAMPLES] = { 0 }; // Wave directions (radians)

void clearTitleBar (int)
{
   glutSetWindowTitle("");
}

// Display a message in the title bar for a short time_.
void setTitleBar(string message)
{
   glutSetWindowTitle(message.c_str());
   glutTimerFunc(TITLE_DISPLAY_TIME, clearTitleBar, 0);
}

// Recompute sample directions when number of samples changes.
void resample(int numSamples)
{
   for (int n = 0; n < numSamples; ++n)
      dir[n] = 2 * PI * randReal();
}

// Storage for a block of parameters that can be read, written, or applied.
struct Parameters
{
   // Construct a parameter block with reasonable values.
   Parameters()
   {
      mode = '0';
      samples = 12;
      eyeHeight = H;
      eyeDistance = 1;
      lightPosition[0] = 0;
      lightPosition[1] = 1;
      lightPosition[2] = 0.25;
      lightPosition[3] = 0;
      dt = 0.05;
      angle = PI/4;
      amplitude = 0.4;
      amplitudeRatio = 0.95;
      wavelength = 0.8;
      wavelengthRatio = 0.7;
      resample(samples);
   }

   // Read a file of parameter blocks, writing each to stderr.
   void read(Parameters presets[])
   {
      ifstream ifs(fileName.c_str());
      int index = 0;
      while (ifs && index < MAX_PRESETS)
      {
         ifs >>
         presets[index].mode >>
         presets[index].samples >>
         presets[index].eyeHeight >>
         presets[index].eyeDistance >>
         presets[index].lightPosition[0] >>
         presets[index].lightPosition[1] >>
         presets[index].lightPosition[2] >>
         presets[index].dt >>
         presets[index].angle >>
         presets[index].amplitude >>
         presets[index].amplitudeRatio >>
         presets[index].wavelength >>
         presets[index].wavelengthRatio;
         presets[index].lightPosition[3] = 0;
         if (ifs)
            ++index;
      }
      ostringstream os;
      os << index << " presets loaded.";
      setTitleBar(os.str());
   }

   // Append current parameters to file as a block.
   void write()
   {
      ofstream ofs(fileName.c_str(), ios::app);
      ofs << *this;
      ofs.close();
   }

   // Format parameters as string.
   friend ostream & operator<<(ostream & os, const Parameters & param)
   {
      os <<
      param.mode << ' ' <<
      setw(3) << param.samples <<
      fixed << setprecision(4) <<
      setw(10) << param.eyeHeight <<
      setw(10) << param.eyeDistance <<
      setw(10) << param.lightPosition[0] <<
      setw(10) << param.lightPosition[1] <<
      setw(10) << param.lightPosition[2] <<
      setw(10) << param.dt <<
      setw(10) << param.angle <<
      setw(10) << param.amplitude <<
      setw(10) << param.amplitudeRatio <<
      setw(10) << param.wavelength <<
      setw(10) << param.wavelengthRatio <<
      endl;
      return os;
   }

   char mode;        // Mode = '0', '1', etc.
   int samples;      // Number of samples for complex waves.
   double eyeHeight;
   double eyeDistance;
   GLfloat lightPosition[4];
   double dt;        // Time step.
   double angle;     // Angle for directional simple waves.
   double amplitude;
   double amplitudeRatio;
   double wavelength;
   double wavelengthRatio;
};

Parameters curr;                  // Current parameter block.
Parameters presets[MAX_PRESETS];  // Stored parameter blocks.

// Compute normal at a point by averaging adjoining quads.
void computeNormals()
{
   // Corner points
   normal[0][0] = Vector(z[1][0] - z[0][0],   z[0][0] - z[0][1], 1).unit();
   normal[N][0] = Vector(z[N-1][0] - z[N][0], z[N][0] - z[N][1], 1).unit();
   normal[0][N] = Vector(z[1][N] - z[0][N],  z[0][N] - z[0][N-1], 2).unit();
   normal[N][N] = Vector(z[N-1][N] - z[N][N], z[N][N] - z[N][N-1], 2).unit();

   // Sides
   for (int i = 1; i <= N-1; i++)
   {
      normal[i][0] = Vector(z[i-1][0] - z[i+1][0], 0, 2).unit();
      normal[i][N] = Vector(z[i-1][N] - z[i+1][N], 0, 2).unit();
   }

   for (int j = 1; j <= N-1; j++)
   {
      normal[0][j] = Vector(0, z[0][j-1] - z[0][j+1], 2).unit();
      normal[N][j] = Vector(0, z[N][j-1] - z[N][j+1], 2).unit();
   }

   // Interior points using Newell's algorithm and averaging.
   for (int i = 1; i <= N-1; i++)
      for (int j = 1; j <= N-1; j++)
         normal[i][j] = Vector
                        (
                           z[i-1][j+1] - z[i+1][j+1] + 2*z[i-1][j] - 2*z[i+1][j] + z[i-1][j-1] - z[i+1][j-1],
                           - z[i-1][j+1] - 2*z[i][j+1] - z[i+1][j+1] + z[i-1][j-1] + 2*z[i][j-1] + z[i+1][j-1],
                           8
                        ).unit();
}

// Update the wave surface.
void update()
{
   double vel = 0.1 * sqrt(curr.wavelength);
   switch (curr.mode)
   {
         // Flat surface
      case '0':
         for (int i = 0; i <= N; i++)
            for (int j = 0; j <= N; j++)
            {
               z[i][j] = 0;
               normal[i][j] = Vector(0,0,1);
            }
         break;

         // Waves in direction X
      case '1':
         for (int i = 0; i <= N; i++)
         {
            double x = (2.0 * PI * i) / N;
            for (int j = 0; j <= N; j++)
            {
               z[i][j] = curr.amplitude * cos((x - vel * time_) / curr.wavelength);

            }
         }
         computeNormals();
         break;

         // Waves in direction 'dir'
      case '2':
         for (int i = 0; i <= N; i++)
         {
            double x = (2.0 * PI * i) / N;
            for (int j = 0; j <= N; j++)
            {
               double y = (2.0 * PI * j) / N;
               z[i][j] = curr.amplitude * cos((x * cos(dir[0]) + y * sin(dir[0]) - vel * time_) / curr.wavelength);

            }
         }
         computeNormals();
         break;

         // Complex waves
      case '3':
         {
            for (int i = 0; i <= N; i++)
               for (int j = 0; j <= N; j++)
                  z[i][j] = 0;
            double vamp = curr.amplitude;
            double vlen = curr.wavelength;
            vel = 0.1 * sqrt(curr.wavelength);
            for (int n = 0; n < curr.samples; ++n)
            {
               {
                  for (int i = 0; i <= N; i++)
                  {
                     double x = (2.0 * PI * i) / N;
                     for (int j = 0; j <= N; j++)
                     {
                        double y = (2.0 * PI * j) / N;
                        z[i][j] += vamp * cos((x * cos(dir[n]) + y * sin(dir[n]) - vel * time_) / vlen);
                     }
                  }
               }
               vamp *= curr.amplitudeRatio;
               vlen *= curr.wavelengthRatio;
               vel = 0.1 * sqrt(curr.wavelength);
            }
            computeNormals();
         }
         break;

         // Circular ripples
      case '4':
         for (int i = 0; i <= N; i++)
         {
            double x = (2.0 * PI * i) / N;
            for (int j = 0; j <= N; j++)
            {
               double y = (2.0 * PI * j) / N;
               double r = sqrt(sqr(x - PI) + sqr(y - PI));
               z[i][j] = curr.amplitude * cos((r - vel * time_) / curr.wavelength) / (1 + sqr(r));
            }
         }
         computeNormals();
         break;
   }
}

// Display the surface and (optionally) parameter controls.
void display ()
{
   glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
   glMatrixMode(GL_MODELVIEW);
   glLoadIdentity();
   glEnable(GL_LIGHTING);
   glEnable(GL_DEPTH_TEST);
   glLightfv(GL_LIGHT0, GL_POSITION, curr.lightPosition);
   gluLookAt
   (
      H, curr.eyeHeight, curr.eyeDistance, //eye
      H, -H/4, 0,                          // model
      0, 0, 1                              // up
   );

   // Display surface.
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

   // Display normals as short, white lines.
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

   // Display parameter adjustments.
   if (showTitle)
   {
      glMatrixMode(GL_PROJECTION);
      glLoadIdentity();
      gluOrtho2D(0, 0, 6, 3);
      glMatrixMode(GL_MODELVIEW);
      glLoadIdentity();
      glDisable(GL_DEPTH_TEST);
      glDisable(GL_LIGHTING);
      glColor3d(1, 1, 1);
      glRasterPos2f(-0.9, 0.9);
      const char *p = title.c_str();
      while (*p)
         glutBitmapCharacter(GLUT_BITMAP_TIMES_ROMAN_24, *p++);
      glMatrixMode(GL_PROJECTION);
      glLoadIdentity();
      gluPerspective(fovy, GLfloat(width) / GLfloat(height), nearPlane, farPlane);
   }
   glutSwapBuffers();
}

// Update and display new configuration.
void idle ()
{
   time_ += curr.dt;
   update();
   glutPostRedisplay();
}

// Update parameter display.
void setTitle()
{
   ostringstream os;
   os << fixed << setprecision(0);
   switch (curr.mode)
   {
      case '0':
         os << "Flat surface.  ";
         break;
      case '1':
         os << "Wave direction = 0.  ";
         break;
      case '2':
         os << "Wave direction = " << degrees(dir[0]) << ".  ";
         break;
      case '3':
         os << "Complex waves.  ";
         break;
      case '4':
         os << "Circular waves.  ";
         break;
   }
   os << setprecision(param == P_ANGLE ? 0 : 3);
   switch (param)
   {
      case P_AMP:
         os << "Amplitude = " << curr.amplitude;
         break;
      case P_AMP_RATIO:
         os << "Amplitude ratio = " << curr.amplitudeRatio;
         break;
      case P_LEN:
         os << "Wavelength = " << curr.wavelength;
         break;
      case P_LEN_RATIO:
         os << "Wavelength ratio = " << curr.wavelengthRatio;
         break;
      case P_ANGLE:
         os << "Angle = " << degrees(dir[0]);
         break;
      case P_DT:
         os << "DT = " << curr.dt;
         break;
      case P_SAMPLES:
         os << "Samples = " << curr.samples << setprecision(0);
         break;
      case P_FOV:
         os << "Field of view = " << fovy;
         break;
      case P_MODE:
         os << "Mode = " << curr.mode;
      default:
         break;
   }
   title = os.str();
   glutPostRedisplay();
}

// Move light in response to mouse movement.
void mouseMovement (int mx, int my)
{
   double a = PI * double(mx) / double(width);
   double b = PI * double(my) / double(height);
   curr.lightPosition[0] = cos(a) * sin(b);
   curr.lightPosition[1] = sin(a) * sin(b);
   curr.lightPosition[2] = cos(b);
   glutPostRedisplay();
//   ostringstream os;
//   os << fixed <<
//   setprecision(0) << degrees(a) << ' ' << degrees(b) << " (" <<
//   setprecision(3) << curr.lightPosition[0] << ", " << curr.lightPosition[1] << ", " << curr.lightPosition[2] << ", " << curr.lightPosition[3] << ")";
//   glutSetWindowTitle(os.str().c_str());
}

// Respond to window resizing, preserving proportions.
void reshapeMainWindow (int newWidth, int newHeight)
{
   width = newWidth;
   height = newHeight;
   glViewport(0, 0, width, height);
   glMatrixMode(GL_PROJECTION);
   glLoadIdentity();
   gluPerspective(fovy, GLfloat(width) / GLfloat(height), nearPlane, farPlane);
}

// Respond to graphic character keys.
void graphicKeys (unsigned char key, int x, int y)
{
   switch (key)
   {
         // Increase current parameter, or DT if no parameter selected.
      case '+':
         switch (param)
         {
            case P_AMP:
               curr.amplitude *= FACTOR;
               break;
            case P_AMP_RATIO:
               curr.amplitudeRatio *= FACTOR;
               if (curr.amplitudeRatio > 1) curr.amplitudeRatio = 1;
               break;
            case P_LEN:
               curr.wavelength *= FACTOR;
               break;
            case P_LEN_RATIO:
               curr.wavelengthRatio *= FACTOR;
               if (curr.wavelengthRatio > 1) curr.wavelengthRatio = 1;
               break;
            case P_ANGLE:
               dir[0] += 0.1;
               break;
            case P_NONE:
            case P_DT:
               curr.dt *= FACTOR;
               break;
            case P_SAMPLES:
               if (curr.samples + 1 < MAX_SAMPLES)
               {
                  ++curr.samples;
                  resample(curr.samples);
               }
               break;
            case P_FOV:
               fovy += 1;
               glMatrixMode(GL_PROJECTION);
               glLoadIdentity();
               gluPerspective(fovy, GLfloat(width) / GLfloat(height), nearPlane, farPlane);
               break;
            case P_MODE:
               if (curr.mode < MAX_MODE)
                  ++curr.mode;
               break;
         }
         setTitle();
         break;

         // Decrease current parameter, or DT if no parameter selected.
      case '-':
         switch (param)
         {
            case P_AMP:
               curr.amplitude /= FACTOR;
               break;
            case P_AMP_RATIO:
               curr.amplitudeRatio /= FACTOR;
               break;
            case P_LEN:
               curr.wavelength /= FACTOR;
               break;
            case P_LEN_RATIO:
               curr.wavelengthRatio /= FACTOR;
               break;
            case P_ANGLE:
               dir[0] -= 0.1;
               break;
            case P_NONE:
            case P_DT:
               curr.dt /= FACTOR;
               break;
            case P_SAMPLES:
               if (curr.samples > 1)
                  --curr.samples;
               break;
            case P_FOV:
               fovy -= 1;
               glMatrixMode(GL_PROJECTION);
               glLoadIdentity();
               gluPerspective(fovy, GLfloat(width) / GLfloat(height), nearPlane, farPlane);
               break;
            case P_MODE:
               if (curr.mode > 0)
                  --curr.mode;
         }
         setTitle();
         break;

         // Toggle normal display.
      case 'n':
         showNormals = ! showNormals;
         break;

         // Toggle surface/line display.
      case 'f':
         if (fillPolys)
            glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
         else
            glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
         fillPolys = ! fillPolys;
         break;

         // Write current parameter block to file.
      case 'w':
         {
            curr.write();
            ostringstream os;
            os << "Preset written.";
            setTitleBar(os.str());
         }
         break;

         // Select stored parameter block.
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
         {
            curr = presets[key - '0'];
            dir[0] = curr.angle;
            ostringstream os;
            os << "Preset " << key << '.';
            setTitleBar(os.str());
         }
         break;

         // Cancel paramter adjustment (+/- will update DT).
      case ' ':
         param = P_NONE;
         showTitle = false;
         break;

      case 27:
         exit(0);
   }
   glutPostRedisplay();
}

// Respond to function keys.
void functionKeys (int key, int x, int y)
{
   switch (key)
   {
      case GLUT_KEY_UP:
         curr.eyeDistance += 0.02 * H;
         break;

      case GLUT_KEY_DOWN:
         curr.eyeDistance -= 0.02 * H;
         break;

      case GLUT_KEY_LEFT:
         curr.eyeHeight -= 0.02 * H;
         if (curr.eyeHeight < 0)
            curr.eyeHeight = 0;
         break;

      case GLUT_KEY_RIGHT:
         curr.eyeHeight += 0.02 * H;
         break;
   }
//   ostringstream os;
//   os << fixed << setprecision(0) << curr.eyeHeight << ' ' << curr.eyeDistance;
//   glutSetWindowTitle(os.str().c_str());
//   glutPostRedisplay();
}

// Set current parameter according to menu selection.
void menu(int sel)
{
   param = sel;
   setTitle();
   showTitle = true;
}

int main (int argc, char **argv)
{
   cout << "Wave simulation\n\n"
        " Use menus (right button) to select paramter\n\n"
        " +      increase current parameter (default: DT)\n"
        " -      decrease current parameter (default: DT)\n"
        " n      toggle normal display\n"
        " f      toggle fill/lines display\n"
        " w      append currnt parameters to file\n"
        " 0-9    select stored parameter block\n"
        " space  clear parameter selection\n"
        " esc    quit\n"
        " left/right arrow  change eye distance\n\n"
        " up/down arrow     change eye height\n";

   glutInit(&argc, argv);
   glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGB);
   glutInitWindowSize(width, height);
   glutCreateWindow("");
   glutDisplayFunc(display);
   glutReshapeFunc(reshapeMainWindow);
   glutKeyboardFunc(graphicKeys);
   glutSpecialFunc(functionKeys);
   glutMotionFunc(mouseMovement);
   glutIdleFunc(idle);
   glClearColor(0.8, 0.8, 1.0, 1.0);

   glLightModeli(GL_LIGHT_MODEL_TWO_SIDE, GL_TRUE);
   glEnable(GL_LIGHTING);
   glEnable(GL_LIGHT0);
   glLightfv(GL_LIGHT0, GL_DIFFUSE, sun);
   glLightfv(GL_LIGHT0, GL_AMBIENT, sun);
   glLightfv(GL_LIGHT0, GL_SPECULAR, specular);
   glMaterialfv(GL_FRONT, GL_AMBIENT_AND_DIFFUSE, diffuse);
   glMaterialfv(GL_FRONT, GL_SPECULAR, sun);
   glMaterialfv(GL_FRONT, GL_SHININESS, shininess);

   glutCreateMenu(menu);
   glutAttachMenu(GLUT_RIGHT_BUTTON);
   glutAddMenuEntry("Amplitude", P_AMP);
   glutAddMenuEntry("Amplitude ratio", P_AMP_RATIO);
   glutAddMenuEntry("Wavelength", P_LEN);
   glutAddMenuEntry("Wavelength ratio", P_LEN_RATIO);
   glutAddMenuEntry("Angle", P_ANGLE);
   glutAddMenuEntry("Time increment", P_DT);
   glutAddMenuEntry("Samples", P_SAMPLES);
   glutAddMenuEntry("Field of view", P_FOV);
   glutAddMenuEntry("Mode", P_MODE);

   setTitle();
   dir[0] = 0;
   curr.read(presets);
   glutMainLoop();
   return 0;
}
