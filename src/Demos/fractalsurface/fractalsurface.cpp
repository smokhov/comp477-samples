/* Fractal Surface for COMP 471
 * Peter Grogono (1997)
 */

#include <GL/glut.h>
#include <cstdio>
#include <cstdlib>
#include <cmath>
#include <sstream>
#include <iomanip>
#include <windows.h>

using namespace std;

/* These constants determine the size of the arrays.
 * If the program gives an overflow error message,
 * increase the size of the corresponding array.
 */

#define MAX_POINTS 24000
#define MAX_TRI    36000

/* The number of triangles in the landscape is 4^maxlev. */
int maxlev = 5;

/* The ruggedness of the landscape increases with the value of steepness. */
float steepness = 0.05;

/* Points and vectors are represented as 3 doubles. */
enum { X, Y, Z };
typedef double Point[3];
typedef double Vector[3];

/* Each vertex of a triangle is an index into the "points" array. */
typedef int Triangle[3];

Point points[MAX_POINTS];
Triangle triangles[MAX_TRI];

/* Normals to triangles. */
Vector TriNormals[MAX_TRI];

/* Normals at vertices of landscape. */
Vector PointNormals[MAX_POINTS];

/* Global variables */
int numPoints = 0;
int numTriangles = 0;

/* Split computes four heights: hmin < lo < hi < maxHeight. */
float minHeight = 1000.0;
float maxHeight = -1000.0;
float lo, hi;

inline double sqr(double x)
{
   return x * x;
}

/* Return a normal random number.
   See Knuth, Art of Computer Programming, Vol. 1, page 104. */
double NormalRandomNumber ()
{
   double x, y, s;
   do
   {
      x = 2.0 * (double) rand() / (double) RAND_MAX - 1.0;
      y = 2.0 * (double) rand() / (double) RAND_MAX - 1.0;
      s = x * x + y * y;
   }
   while (s >= 1.0);
   return x * sqrt(-2.0 * log(s) / s);
}

/* Add a new point to the array only if its (X,Z) coordinates are
   different from any point already stored. Report overflow. */
int addPoint (double x, double y, double z)
{
   int i = 0;
   points[numPoints][X] = x;
   points[numPoints][Y] = y;
   points[numPoints][Z] = z;
   while (points[i][X] != x || points[i][Z] != z)
      i++;
   if (i == numPoints)
   {
      numPoints++;
      if (numPoints >= MAX_POINTS)
      {
         printf("The points array has overflowed!\n");
         exit(0);
      }
   }
   return i;
}

/* Add a new triangle, reporting overflow if it occurs. */
void addTriangle (int a, int b, int c)
{
   if (numTriangles < MAX_TRI)
   {
      triangles[numTriangles][0] = a;
      triangles[numTriangles][1] = b;
      triangles[numTriangles][2] = c;
      numTriangles++;
   }
   else
   {
      printf("The triangles array has overflowed!\n");
      exit(0);
   }
}

/* Compute the moveMidPoint of two given points and
   add a random fluctuation to its Y component. */
int moveMidPoint (int p, int q)
{
   double len = sqrt(
                   sqr(points[p][X] - points[q][X]) +
                   sqr(points[p][Y] - points[q][Y]) +
                   sqr(points[p][Z] - points[q][Z]));
   double x = 0.5 * (points[p][X] + points[q][X]);
   double y = 0.5 * (points[p][Y] + points[q][Y]) +
              len * steepness * NormalRandomNumber();
   double z = 0.5 * (points[p][Z] + points[q][Z]);
   if (minHeight > y)
      minHeight = y;
   if (maxHeight < y)
      maxHeight = y;
   return addPoint(x, y, z);
}

/* Compute a unit vector "v" normal to the plane containing p, q, and r. */
void normalToPlane (int p, int q, int r, Vector v)
{
   double x1 = points[p][X];
   double y1 = points[p][Y];
   double z1 = points[p][Z];
   double x2 = points[q][X];
   double y2 = points[q][Y];
   double z2 = points[q][Z];
   double x3 = points[r][X];
   double y3 = points[r][Y];
   double z3 = points[r][Z];
   v[X] = + (y2-y1)*(z3-z1) - (z2-z1)*(y3-y1);
   v[Y] = + (x2-x1)*(z3-z1) - (z2-z1)*(x3-x1);
   v[Z] = + (x2-x1)*(y2-y1) - (y2-y1)*(x3-x1);
}

/* If level < maxlev, recursively divide the given
   triangular region into smaller triangles.
   Otherwise, add this triangle to the list. */
void split (int level, int a, int b, int c)
{
   if (level >= maxlev)
      addTriangle(a, b, c);
   else
   {
      int am = moveMidPoint(b, c);
      int bm = moveMidPoint(c, a);
      int cm = moveMidPoint(a, b);
      split(level + 1, a, cm, bm);
      split(level + 1, b, am, cm);
      split(level + 1, c, bm, am);
      split(level + 1, am, bm, cm);
   }
}

/* Construct a scene from four initial points given in counterclockwise order. */
void makeScene (int a, int b, int c, int d)
{
   int k, p, t;
   minHeight = 1000;
   maxHeight = -1000;

   /* Recursively split into subtriangles */
   printf("Generating triangles.\n");
   split(0, a, b, c);
   split(0, a, c, d);
   printf("%d points, %d triangles.\n", numPoints, numTriangles);

   /* Compute height divisions for colouring */
   lo = minHeight + 0.3 * (maxHeight - minHeight);
   hi = minHeight + 0.7 * (maxHeight - minHeight);

   /* Find normals for each triangle */
   printf("Computing normals.\n");
   for (t = 0; t < numTriangles; t++)
      normalToPlane(triangles[t][X], triangles[t][Y], triangles[t][Z], TriNormals[t]);

   /* Compute average normals at each vertex */
   for (p = 0; p < numPoints; p++)
   {
      double x = 0.0;
      double y = 0.0;
      double z = 0.0;
      double sum;
      for (t = 0; t < numTriangles; t++)
      {
         for (k = 0; k < 3; k++)
         {
            if (triangles[t][k] == p)
            {
               x += TriNormals[t][X];
               y += TriNormals[t][Y];
               z += TriNormals[t][Z];
            }
         }
      }
      sum = sqrt(x*x + y*y + z*z);
      PointNormals[p][X] = x/sum;
      PointNormals[p][Y] = y/sum;
      PointNormals[p][Z] = z/sum;
   }
   printf("Done.\n");
}

/******************************************************/

/* The code in this section writes the data to a file.
 * It may be omitted.
 */

/* Print a point. */
void showPoint (FILE *out, int p)
{
   fprintf(out, "(%7.2f %7.2f %7.2f) ", points[p][X], points[p][Y], points[p][Z]);
}

/* Print a vector. */
void showVector (FILE *out, Vector v)
{
   double norm = sqrt(v[X]*v[X]+v[Y]*v[Y]+v[Z]*v[Z]);
   fprintf(out, "(%7.2f %7.2f %7.2f)  %7.2f", v[X], v[Y], v[Z], norm);
}

/* Print distance between two points. */
void showDist(FILE *out, Point p, Point q)
{
   double len = sqrt(sqr(p[X] - q[X]) + sqr(p[Y] - q[Y]) + sqr(p[Z] - q[Z]));
   fprintf(out, "%7.2f", len);
}

/* Print a triangle. i = 0 => point indexes;
   i > 0 => point coordinates. */
void showTriangle (FILE *out, Triangle t, int i)
{
   switch (i)
   {
      case 0:
         fprintf(out, "  %5d %5d %5d ", t[0], t[1], t[2]);
         break;
      case 1:
         showPoint(out, t[0]);
         showPoint(out, t[1]);
         showPoint(out, t[2]);
         showDist(out, points[t[1]], points[t[2]]);
         showDist(out, points[t[2]], points[t[0]]);
         showDist(out, points[t[0]], points[t[1]]);
         break;
   }
}

/* Print a report to output file */
void report ()
{
   int i;
   FILE *out;
   out = fopen("fracres.dat", "w");
   if (!out)
   {
      printf("Cannot open output file!\n");
      exit(0);
   }

   fprintf(out, "Points:\n");
   for (i = 0; i < numPoints; i++)
   {
      fprintf(out, "%5d  ", i);
      showPoint(out, i);
      fprintf(out, "\n");
   }

   fprintf(out, "\nTriangles:\n");
   for (i = 0; i < numTriangles; i++)
   {
      fprintf(out, "%5d ", i);
      showTriangle(out, triangles[i], 1);
      fprintf(out, "\n");
   }

   fprintf(out, "\nTriangle normals:\n");
   for (i = 0; i < numTriangles; i++)
   {
      fprintf(out, "%5d  ", i);
      showVector(out, TriNormals[i]);
      fprintf(out, "\n");
   }

   fprintf(out, "\nPoint normals:\n");
   for (i = 0; i < numPoints; i++)
   {
      fprintf(out, "%5d  ", i);
      showVector(out, PointNormals[i]);
      fprintf(out, "\n");
   }

   fclose(out);
}

/************************************************/

/* All code below this point is for OpenGL. */

const double PI = 3.1415926535;

int width = 1000;
int height = 800;
double bottom;
double spacing = 0.1;
double sun = 0;

// Viewpoint in model
double xViewPoint = 0;
double yViewPoint = 0;
double zViewPoint = 0;

GLfloat sunPosition[] = { 1, -1, 0, 0 };
GLfloat sunColour[] = { 1.0, 1.0, 1.0, 0.0 };
GLfloat earth[] = { 0.4, 0.2, 0.2, 0.0 };
GLfloat trees[] = { 0.1, 0.6, 0.2, 0.0 };
GLfloat snow[] = { 1.0, 1.0, 1.0, 0.0 };

enum Mode { REDRAW, VIEW, SUN, NORMALS, EXIT } mode = VIEW;
enum ShadingModel { LINES, FLAT, SMOOTH } shading = SMOOTH;
bool showNormals = false;

void keys (unsigned char key, int x, int y)
{
   switch (key)
   {
      case 27:
         exit(0);
   }
}

void drag (int mx, int my)
{
   double x = (double) mx / (double) width;
   double y = (double) my / (double) height;
   switch (mode)
   {
      case VIEW:
         {
            double a = 2 * PI * x;
            double r = 5 * y + 3;
            xViewPoint = r * cos(a);
            yViewPoint = 2;
            zViewPoint = r * sin(a);
         }
         glutPostRedisplay();
         break;
      case SUN:
      case REDRAW:
      case EXIT:
      default:
         break;
   }
}

const double DT = 20; // milliseconds
bool sunMoving = true;
double sunAngle = 0;
double realTime = GetTickCount();
double pathTime = realTime;

void idle()
{
   if (sunMoving)
   {
      realTime = GetTickCount();
      while (pathTime < realTime)
      {
         sunPosition[1] = cos(sunAngle);
         sunPosition[2] = sin(sunAngle);
         sunAngle += 0.01;
         pathTime += DT;
      }
      glutPostRedisplay();
   }
}

void reshape (int w, int h)
{
   double ar = (float) w / (float) h;
   width = w;
   height = h;
   glViewport(0, 0, (GLsizei) w, (GLsizei) h);
   glMatrixMode(GL_PROJECTION);
   glLoadIdentity();
   gluPerspective(40, ar, 1, 50);
   glutPostRedisplay();
}

void display (void)
{
   glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
   glMatrixMode(GL_MODELVIEW);
   glLoadIdentity();

   gluLookAt(
      xViewPoint, yViewPoint, zViewPoint,
      0, 0, 0,
      0, 1, 0);
   glLightfv(GL_LIGHT0, GL_POSITION, sunPosition);

   switch (shading)
   {
      default:
      case SMOOTH:
      case FLAT:
         glEnable(GL_LIGHTING);
         if (shading == SMOOTH)
            glShadeModel(GL_SMOOTH);
         else
            glShadeModel(GL_FLAT);
         glBegin(GL_TRIANGLES);
         for (int i = 0; i < numTriangles; i++)
         {
            for (int j = 0; j < 3; j++)
            {
               int t = triangles[i][j];
               float h = points[t][1];
               if (h > hi)
                  glMaterialfv(GL_FRONT, GL_AMBIENT_AND_DIFFUSE, snow);
               else if (h > lo)
                  glMaterialfv(GL_FRONT, GL_AMBIENT_AND_DIFFUSE, trees);
               else
                  glMaterialfv(GL_FRONT, GL_AMBIENT_AND_DIFFUSE, earth);
               glNormal3dv(PointNormals[t]);
               glVertex3dv(points[t]);
            }
         }
         glEnd();
         break;

      case LINES:
         glDisable(GL_LIGHTING);
         glColor3f(0, 0, 0);
         for (int i = 0; i < numTriangles; i++)
         {
            glBegin(GL_LINE_LOOP);
            for (int j = 0; j < 3; j++)
            {
               int t = triangles[i][j];
               glVertex3dv(points[t]);
            }
            glEnd();
         }
         break;
   }
   if (showNormals)
   {
      glDisable(GL_LIGHTING);
      glColor3f(1, 1, 1);
      glBegin(GL_LINES);
      for (int i = 0; i < numTriangles; i++)
      {
         for (int j = 0; j < 3; j++)
         {
            int t = triangles[i][j];
            glVertex3dv(points[t]);
            glVertex3d(
               points[t][X] + 0.2 * PointNormals[t][X],
               points[t][Y] + 0.2 * PointNormals[t][Y],
               points[t][Z] + 0.2 * PointNormals[t][Z]
            );

         }
      }
      glEnd();
   }
   glutSwapBuffers();
}

void newModel(bool glStarted)
{
   if (glStarted)
   {
      glutSetWindowTitle("Calculating ...");
   }
   numPoints = 0;
   numTriangles = 0;
   makeScene(
      addPoint(5,0,5),
      addPoint(-5,0,5),
      addPoint(-5,0,-5),
      addPoint(5,0,-5));
   report();
   if (glStarted)
   {
      ostringstream os;
      os << "Fractal landscape.  " <<
      maxlev << " iterations.  " <<
      numPoints << " points.  " <<
      numTriangles << " triangles.  Steepness factor: " <<
      fixed << setprecision(2) << steepness;
      glutSetWindowTitle(os.str().c_str());
   }
}

void setDepth(int depth)
{
   maxlev = depth;
   newModel(true);
   glutPostRedisplay();
}

void setSteepness(int s)
{
   switch (s)
   {
      default:
      case 1:
         steepness = 0.025;
         break;
      case 2:
         steepness = 0.05;
         break;
      case 3:
         steepness = 0.1;
         break;
      case 4:
         steepness = 0.2;
         break;
   }
   newModel(true);
   glutPostRedisplay();
}

void setShading(int s)
{
   shading = ShadingModel(s);
   glutPostRedisplay();
}

void setMode(int newMode)
{
   switch(Mode(newMode))
   {
      case REDRAW:
         newModel(true);
         break;
      case SUN:
         sunMoving = ! sunMoving;
         break;
      case NORMALS:
         showNormals = ! showNormals;
         break;
      case EXIT:
         exit(0);
         break;
      default:
      case VIEW:
         break;
   }
   glutPostRedisplay();
}

int main(int argc, char *argv[])
{
   newModel(false);
   glutInit(&argc, argv);
   glutInitDisplayMode (GLUT_DOUBLE | GLUT_RGB | GLUT_DEPTH);
   glutInitWindowSize(width, height);
   glutCreateWindow("Fractal landscape");
   glClearColor(0.7, 0.7, 1.0, 0.0);
   glutDisplayFunc(display);
   glutKeyboardFunc(keys);
   glutMotionFunc(drag);
   glutReshapeFunc(reshape);
   glutIdleFunc(idle);
   glEnable(GL_DEPTH_TEST);
   glEnable(GL_LIGHTING);
   glEnable(GL_LIGHT0);
   glLightfv(GL_LIGHT0, GL_DIFFUSE, sunColour);

   int recdepth = glutCreateMenu(setDepth);
   glutAddMenuEntry("3", 3);
   glutAddMenuEntry("4", 4);
   glutAddMenuEntry("5", 5);
   glutAddMenuEntry("6", 6);
   glutAddMenuEntry("7", 7);

   int recSteep = glutCreateMenu(setSteepness);
   glutAddMenuEntry("0.025", 1);
   glutAddMenuEntry("0.05", 2);
   glutAddMenuEntry("0.1", 3);
   glutAddMenuEntry("0.2", 4);

   int shading = glutCreateMenu(setShading);
   glutAddMenuEntry("Lines", int(LINES));
   glutAddMenuEntry("Flat", int(FLAT));
   glutAddMenuEntry("Smooth", int(SMOOTH));

   glutCreateMenu(setMode);
   glutAddMenuEntry("New landscape", int(REDRAW));
   glutAddMenuEntry("Sun moves", int(SUN));
   glutAddMenuEntry("Show normals", int(NORMALS));
   glutAddSubMenu("Shading model", shading);
   glutAddSubMenu("Recursion depth", recdepth);
   glutAddSubMenu("Steepness", recSteep);
   glutAddMenuEntry("Exit", int(EXIT));
   glutAttachMenu(GLUT_RIGHT_BUTTON);

   glutMainLoop();
   return 0;
}
