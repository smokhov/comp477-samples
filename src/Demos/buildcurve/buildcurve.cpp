#include <cassert>
#include <iostream>
#include <iomanip>
#include <sstream>
#include <vector>
#include <GL/cugl.h>
#include <GL/glut.h>
#include <windows.h>

using namespace std;
using namespace cugl;

// Current window dimensions
int windowWidth = 800;
int windowHeight = 800;

// Current mouse positions
GLdouble mx; // 0 <= mx <= windowWidth
GLdouble my; // 0 <= my <= windowHeight

// Material data
GLfloat white[] = { 1.0, 1.0, 1.0, 1.0 };
GLfloat red[] = { 1, 0, 0, 1  };
GLfloat green[] = { 0, 1, 0, 1 };
GLfloat blue[] = { 0.3, 0.3, 0.9, 1.0 };
GLfloat shiny[] = { 30.0 };
GLfloat dir[] = { 0.0, 1.0, 1.0, 0.0 };

// Ligting data
GLfloat ambient[] = {0.4f, 0.4f, 0.4f, 1};
GLfloat position[] = {5, 20, 0, 1};

// Return a point between p and q with positoin t, 0 <= t <= 1.
// t = 0 gives p; t = 1 gives q.
Point interpolate(const Point & p, const Point & q, double t)
{
   return p + t * (q - p);
}

// A group consists of four points.  The corresponding de Casteljau curve
// passes through p0 and p3.  The tangent at p0 passes through p1 and the
// tangent at p3 passes through p2.
class Group
{
public:
   Group(const Point & p0, const Point & p1, const Point & p2, const Point & p3)
      : p0(p0), p1(p1), p2(p2), p3(p3)
   {
      len = length();
   }

   // Return the point on the curve at time t, 0 <= t <= 1.
   // Uses de Cateljau's interpolation algorithm to find the point.
   Point getPos(double t) const
   {
      Point q0 = interpolate(p0, p1, t);
      Point q1 = interpolate(p1, p2, t);
      Point q2 = interpolate(p2, p3, t);
      Point r0 = interpolate(q0, q1, t);
      Point r1 = interpolate(q1, q2, t);
      return interpolate(r0, r1, t);
   }

   // Return a unit vector giving the direction of the curve at time t, 0 <= t <= 1.
   // Uses two nearby points to obtain the direction vector, then normalizes it.
   Vector getDir(double t) const
   {
      const double DELTA = 0.01;
      Point s, f;
      if (t < 0.5)
      {
         s = getPos(t);
         f = getPos(t + DELTA);
      }
      else
      {
         s = getPos(t - DELTA);
         f = getPos(t);
      }
      return (f - s).unit();
   }

   // Return the arc length of this segment.
   double getLen() const
   {
      return len;
   }

   // Return the normalized length of this segment.
   double getNormLen() const
   {
      return normlen;
   }

   // Set the normalized length of this segment.
   void setNormalizedLength(double len)
   {
      normlen = len;
   }

   // Display the group for debugging purposes.
   friend ostream & operator<<(ostream & os, const Group & g)
   {
      os << g.p0 << g.p1 << g.p2 << g.p3 << "  length = " << g.len << " (" << g.normlen << ")\n";
      return os;
   }

private:

   // Compute an approximation to the length of the segment
   // by adding the lengths of many small parts, each assumed straight.
   double length() const
   {
      const int NP = 100;
      Point p = p0;
      double d = 0;
      for (int i = 1; i <= NP; ++i)
      {
         Point q = getPos(double(i)/double(NP));
         d += dist(q, p);
         p = q;
      }
      return d;
   }

   Point p0, p1, p2, p3;  // The four points of the segment.
   double len;  // The actual length of the segment.
   double normlen;  // The length of the segment, normalized so that the entire curve has length 1.
};


// A Curve consists of several segments.
// Segments join with C^2 continuity.
class Curve
{
public:
   Curve() : ok(true) {};

   void setPoints(const vector<Point> & nsp, const vector<Point> & nep)
   {
      sp = nsp;
      ep = nep;
      np = sp.size();
      if (ep.size()!= np)
      {
         cerr << "Start and end point arrays are not the same size\n";
         ok = false;
         return;
      }

      // Find midpoints
      for (size_t n = 0; n < np; ++n)
         mp.push_back(interpolate(sp[n], ep[n], 0.5));

      // Form groups
      for (size_t n = 0; n <= np-2; ++n)
         gps.push_back(Group(mp[n], ep[n], sp[n+1], mp[n+1]));

      // Find total length of curve.
      totlen = 0;
      for (vector<Group>::const_iterator it = gps.begin(); it != gps.end(); ++it)
         totlen += it->getLen();

      // Find start points of each segment.
      dist.push_back(0);
      double cumdist = 0;
      for (vector<Group>::iterator it = gps.begin(); it != gps.end(); ++it)
      {
         double normlen = it->getLen() / totlen;
         cumdist += normlen;
         dist.push_back(cumdist);
         it->setNormalizedLength(normlen);
      }
   }

   // Return the point corresponding to time t on the curve.
   // Use binary search to find which segment it belongs to.
   Point getPos(double t)
   {
      if (ok)
      {
         size_t L = 0;
         size_t R = dist.size();
         while (R - L > 1)
         {
            size_t M = (L + R)/2;
            double d = dist[M];
            if (t > d)
               L = M;
            else
               R = M;
         }
         assert(R = L + 1);
         double r = (t - dist[L]) / (dist[R] - dist[L]);
         return gps[L].getPos(r);
      }
      else return Point();
   }

   // Return the direction at time t on the curve.
   // Use binary search to find which segment it belongs to.
   Vector getDir(double t)
   {
      if (ok)
      {
         size_t L = 0;
         size_t R = dist.size();
         while (R - L > 1)
         {
            size_t M = (L + R)/2;
            double d = dist[M];
            if (t > d)
               L = M;
            else
               R = M;
         }
         assert(R = L + 1);
         double r = (t - dist[L]) / (dist[R] - dist[L]);
         return gps[L].getDir(r);
      }
      else return Vector();
   }

   // Draw the tangents used to construct the curve.
   void showTangents()
   {
      if (ok)
      {
         for (size_t n = 0; n < np; ++n)
         {
            sp[n].draw();
            ep[n].draw();
         }
      }
   }

   // Display Curve for debugging purposes.
   friend ostream & operator<<(ostream & os, const Curve & c)
   {
      if (c.ok)
      {
         os << "Length: " << c.totlen << endl;
         os << "Distances: ";
         for (vector<double>::const_iterator it = c.dist.begin(); it != c.dist.end(); ++it)
            os << ' ' << *it;
         os << endl;
         os << "Points: \n";
         for (size_t n = 0; n < c.np; ++n)
            os << setw(3) << c.sp[n] << setw(3) << c.mp[n] << setw(3) << c.ep[n] << endl;
         os << "Groups:\n";
         for (vector<Group>::const_iterator it = c.gps.begin(); it != c.gps.end(); ++it)
            os << *it;
      }
      return os;
   }

private:
   bool ok;  // Valid data has been supplied and used.
   size_t np;  // Number of tangent pairs.
   double totlen;  // Total length of the curve.
   vector<Point> sp; // Start points (given)
   vector<Point> ep; // End points (given)
   vector<Point> mp; // Midpoints (computed)
   vector<Group> gps; // Groups of 4 linked points
   vector<double> dist; // Normalize distance to start of group
};



Curve c;  // Global curve
double t = 0;  // Global time on curve, used by display function.

// Display flags
bool displayAxes = false;
bool displayBall = false;
bool displayCurve = true;
bool displayModel = false;
bool displayTangents = false;
bool displayVector = false;
bool useQuaternion = false;

// Construct a curve from a set of built-in point pairs.
void buildCurve()
{
   vector<Point> sp;
   sp.push_back(Point(-24,  0,  0));
   sp.push_back(Point(-15,  8,  0));
   sp.push_back(Point(  0,  8, 12));
   sp.push_back(Point(  8, 10, 12));
   sp.push_back(Point( 15,  0,  0));

   vector<Point> ep;
   ep.push_back(Point(-16,   0, 0));
   ep.push_back(Point( -5,  15, 6));
   ep.push_back(Point(  0,  -8, 15));
   ep.push_back(Point( 15, -10, 15));
   ep.push_back(Point( 25,   0, 0));
   c.setPoints(sp, ep);
}

// Display function
void display (void)
{

   glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
   glMatrixMode(GL_MODELVIEW);
   glLoadIdentity();

   // Set up camera.
   glTranslated(0, 0, -40);
   glRotated(360 * mx, 0, 1, 0);
   glRotated(360 * my, 0, 0, 1);

   // Display curve, tangents, and vector without lighting.
   glDisable(GL_LIGHTING);

   if (displayCurve)
   {
      glColor3d(0, 1, 0);
      glBegin(GL_LINE_STRIP);
      const int NP = 500;
      for (int i = 0; i <= NP; ++i)
         c.getPos(double(i) / double(NP)).draw();
      glEnd();
   }

   if (displayTangents)
   {
      glColor3d(1, 0, 0);
      glBegin(GL_LINES);
      c.showTangents();
      glEnd();
   }

   // Translate to current curve position and get local direction
   c.getPos(t).translate();
   Vector d = c.getDir(t);

   if (displayVector)
   {
      glColor3d(1, 1, 1);
      (10 * d).draw(Point());
   }

   // Use lighting for model and axes.
   glEnable(GL_LIGHTING);

   if (useQuaternion)
   {
      // Rotate so that -Z is parallel to tangent vector.
      Quaternion(-K, d).apply();
   }

   if (displayBall)
   {
      glMaterialfv(GL_FRONT, GL_AMBIENT_AND_DIFFUSE, red);
      glMaterialfv(GL_FRONT, GL_SPECULAR, white);
      glMaterialfv(GL_FRONT, GL_SHININESS, shiny);
      glutSolidSphere(1, 50, 50);
   }

   if (displayModel)
   {
      const double s = 0.3;
      glScaled(s, s, s);
      buildPlane();
   }

   if (displayAxes)
   {
      const double s = 3;
      glScaled(s, s, s);
      axes();
   }
   glutSwapBuffers();
}

// Timing data
const double DT = 10;
const double totalTime = 10000; // Milliseconds needed to traverse curve
double startTime = GetTickCount();
double pathTime = 0;

void idle()
{
   double currTime = GetTickCount() - startTime;
   while (pathTime < currTime)
      pathTime += DT;
   t = pathTime / totalTime;
   if (t > 1)
   {
      startTime = GetTickCount();
      pathTime = 0;
   }
   glutPostRedisplay();
}

void mouse(int x, int y)
{
   mx = double(x) / double(windowWidth);
   my = 1.0 - double(y) / double(windowHeight);
   glutPostRedisplay();
}

void keyboard (unsigned char key, int x, int y)
{
   switch (key)
   {
      case 'a':
         displayAxes = !displayAxes;
         break;
      case 'b':
         displayBall = !displayBall;
         break;
      case 'c':
         displayCurve = !displayCurve;
         break;
      case 'f':
         glutFullScreen();
         break;
      case 'm':
         displayModel = !displayModel;
         break;
      case 'q':
         useQuaternion = !useQuaternion;
         break;
      case 't':
         displayTangents = !displayTangents;
         break;
      case 'v':
         displayVector = !displayVector;
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
   gluPerspective(60, double(w)/double(h), 1, 100);
   glutPostRedisplay();
}

int main(int argc, char *argv[])
{
   // Inform user.
   cout <<
        "Following a synthesized 3D curve\n\n"
        " a    toggle display axes\n"
        " b    toggle display ball\n"
        " c    toggle display curve\n"
        " f    full screen\n"
        " m    toggle display model (plane)\n"
        " q    toggle use quaternion\n"
        " t    toggle display tangents\n"
        " v    toggle display local tangent vector\n"
        " ESC  exit\n\n"
        "Use mouse to rotate view.\n";

   // Construct the curve.
   buildCurve();

   // Initialize OpenGL stuff.
   glutInit(&argc, argv);
   glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGB | GLUT_DEPTH);
   glutInitWindowSize(windowWidth, windowHeight);
   glutCreateWindow("Moving along a curve");
   glutDisplayFunc(display);
   glutMotionFunc(mouse);
   glutKeyboardFunc(keyboard);
   glutReshapeFunc(reshape);
   glutIdleFunc(idle);
   glEnable(GL_DEPTH_TEST);
   glEnable(GL_NORMALIZE);
   glEnable(GL_LIGHT0);
   glLightfv(GL_LIGHT0, GL_AMBIENT, ambient);
   glLightfv(GL_LIGHT0, GL_POSITION, position);
   glutMainLoop();
   return 0;
}
