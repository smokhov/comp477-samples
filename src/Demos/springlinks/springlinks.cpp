// Position graph vertexes using spring simulation
// Edges of the graph behave like springs: connected vertexes attract
// one another according to Hooke's law.  In addition, all vertexes
// repel one another with an inverse square law.  Thus connected
// vertexes will tend to be separated by the natural length of the
// springs and unconnected vertexes will drift apart.

// Input data format:
//    attraction factor: kAttraction
//    repulsion factor: kRepulsion
//    damping factor: kDamping
//    for each edge:
//       two vertex numbers
//    terminator:
//       -1 -1

// The vertex numbers should be chosen from a continuous sequence
// 0,1,2,...,n for a graph with (n+1) vertexes.  Gaps in the
// sequence do not cause fatal problems, but the corresponding
// vertexes will not be connected to the graph and will be driven
// to infinity by the repulsive force.

// Link with: opengl32.lib, glu32.lib, glut32.lib.

#include <GL/glut.h>
#include <cassert>
#include <cmath>
#include <iostream>
#include <fstream>
#include <sstream>
#include <cstring>
#include <iomanip>
#include <cstdlib>
#include <vector>
#include <io.h>

using namespace std;

// Values that do not change during simulation

double DT = 0.005;
double kAttraction = 1;
double kRepulsion = 0.1;
double kDamping = 0.01;

// Natural length of a spring: if the length of an edge
// is less than this, the vertexes are repelled.
double springLength = 1;

// The simulation terminates when the total kinetic energy
// of the vertexes falls below this value.
double minEnergy = 0.1;

// Variables that change during simulation
double kineticEnergy = 0;
int cycle = 0;
bool graphReady = false;

// File name table
vector<string> names;

// Global graph data
const int MAXVERTEXES = 50;
int numVertexes;
int numEdges;
class Vertex;
Vertex *vertexes[MAXVERTEXES];
bool adj[MAXVERTEXES][MAXVERTEXES];

const double VSIZE = 5;  // Random factor for initial vertex positions
const int DSIZE = 2 * int(VSIZE) + 1;
const double MINDIST = 0.1; // Minimum distance for initial vertex positions

// Data for graphical display

// Initial size of graphics window.
const int WIDTH  = 600;
const int HEIGHT = 400;

// Current size of window.
int width  = WIDTH;
int height = HEIGHT;

// Mouse positions, normalized to [0,1].
double xMouse = 0.5;
double yMouse = 0.5;

// Bounds of viewing frustum.
double nearPlane =  3;
double farPlane  = 50;

// Initial distance of object
double howFar = 10;

// Viewing angle.
double fovy = 40.0;

// Radius of sphere representing a vertex.
double vertexRadius = 0.05;

// The number of cycles executed between display updates.
int DisplayRate = 10;

// Class declarations

class Vector
{
public:
   Vector(double x, double y, double z)
      : x(x), y(y), z(z) {}

   Vector(Vertex *pu, Vertex *pv);
   // Vector representing displacement between two vertexes

   Vector(Vector v, double s)
   // Vector with length s in direction of v
   // If v is the zero vector, construct the zero vector
   {
      double len = v.len();
      if (len == 0)
      {
         x = 0;
         y = 0;
         z = 0;
      }
      else
      {
         double scale = s / len;
         x = scale * v.x;
         y = scale * v.y;
         z = scale * v.z;
      }
   }

   double norm()
   {
      return x * x + y * y + z * z;
   }

   double len()
   {
      return sqrt(norm());
   }

   Vector & operator+=(const Vector & v)
   {
      x += v.x;
      y += v.y;
      z += v.z;
      return *this;
   }

   Vector & operator-=(const Vector & v)
   {
      x -= v.x;
      y -= v.y;
      z -= v.z;
      return *this;
   }

   void clear()
   {
      x = 0;
      y = 0;
      z = 0;
   }

   friend Vector operator*(double scale, Vector v);
   friend Vector operator-(Vector u, Vector v);
   friend ostream & operator<<(ostream & os, Vector & v);

   double x;
   double y;
   double z;
};


class Vertex
{
public:
   Vertex(double x = 0, double y = 0, double z = 0)
      : x(x), y(y), z(z), vel(0,0,0), force(0,0,0) {}

   void update()
   {
      Vector acc = force - kDamping * vel;
      vel += DT * acc;
      x += DT * vel.x * DT;
      y += DT * vel.y * DT;
      z += DT * vel.z * DT;
   }

   void clearForce()
   {
      force.clear();
   }

   Vertex & operator+=(Vector f)
   {
      force += f;
      return *this;
   }

   Vertex & operator-=(Vector f)
   {
      force -= f;
      return *this;
   }

   void draw()
   {
      glPushMatrix();
      glTranslatef(x, y, z);
      glutSolidSphere(vertexRadius, 15, 15);
      glPopMatrix();
   }

   void setVertex()
   {
      glVertex3f(x, y, z);
   }

   double kineticEnergy()
   {
      return vel.norm();
   }

   friend ostream & operator<<(ostream & os, Vertex *pv);

   double x;
   double y;
   double z;
   Vector vel;
   Vector force;
};

// Member functions

Vector::Vector(Vertex *pu, Vertex *pv)
{
   x = pu->x - pv->x;
   y = pu->y - pv->y;
   z = pu->z - pv->z;
}

// Friendly functions

Vector operator*(double s, Vector v)
{
   return Vector(s * v.x, s * v.y, s * v.z);
}

Vector operator-(Vector u, Vector v)
{
   return Vector(u.x - v.x, u.y - v.y, u.z - v.z);
}

ostream & operator<<(ostream & os, Vector & v)
{
   return os << '(' << v.x << ',' << v.y << ',' << v.z << ')';
}

ostream & operator<<(ostream & os, Vertex *pv)
{
   return os << '(' << pv->x << ',' << pv->y << ',' << pv->z << ')';
}

// Utility functions

double sqr(double x)
{
   return x * x;
}

double dist(Vertex *pu, Vertex *pv)
{
   return sqrt(sqr(pu->x - pv->x) + sqr(pu->y - pv->y) + sqr(pu->z - pv->z));
}

// Return a random number in [-VSIZE, VSIZE]
double rnd()
{
   return double(rand() % DSIZE) - VSIZE;
}

void idle();

void ReadGraph(int entry)
{
   glutIdleFunc(0);
   graphReady = false;

   const char *filename = names[entry].c_str();
   ifstream is(filename);
   if (!is)
   {
      cerr << "Failed to open " << filename << endl;
      return;
   }
   else
      cout << "File name: " << filename << endl;

   graphReady = true;

   // Read attraction, repulsion, and damping factor
   is >> kAttraction >> kRepulsion >> kDamping;
   cout << fixed << setprecision(6) <<
        "Attraction: " << setw(12) << kAttraction << endl <<
        "Repulsion:  " << setw(12) << kRepulsion << endl <<
        "Damping:    " << setw(12) << kDamping << endl;

   // Initialize graph data
   numVertexes = 0;
   numEdges = 0;
   for (int v = 0; v < MAXVERTEXES; v++)
      vertexes[v] = NULL;
   for (int i = 0; i < MAXVERTEXES; i++)
      for (int j = 0; j < MAXVERTEXES; j++)
         adj[i][j] = false;

   // Read edges as vertex pairs
   // Vertex numbers must be 0,1,2,3,...
   // The pair "-1, -1" terminates the list
   while (true)
   {
      if (is.eof())
      {
         cerr <<
              "End of file encountered after " <<
              numEdges << " edges read.\n";
         return;
      }
      int u, v;
      is >> u >> v;
      if (u >= 0 && v >= 0)
      {
         if (u >= numVertexes)
            numVertexes = u + 1;
         if (v >= numVertexes)
            numVertexes = v + 1;
         adj[u][v] = true;
         adj[v][u] = true;
         numEdges++;
      }
      else
         break;
   }
   is.close();

   // Assign random coordinates to the vertexes
   for (int v = 0; v < numVertexes; v++)
      while (true)
      {
         vertexes[v] = new Vertex(rnd(), rnd(), rnd());
         bool farAway = true;
         for (int u = 0; u < v; u++)
            if (dist(vertexes[u], vertexes[v]) < MINDIST)
            {
               farAway = false;
               break;
            }
         if (farAway)
            break;
      }

   for (int i = 0; i < numVertexes; i++)
   {
      int degree = 0;
      for (int j = 0; j < numVertexes; j++)
         if (adj[i][j])
            degree++;
      if (degree == 0)
         cout << i << " is isolated.\n";
   }

   // Move centroid to origin.
   double xc = 0;
   double yc = 0;
   double zc = 0;
   for (int v = 0; v < numVertexes; v++)
   {
      Vertex *p = vertexes[v];
      xc += p->x;
      yc += p->y;
      zc += p->z;
   }
   xc /= numVertexes;
   yc /= numVertexes;
   zc /= numVertexes;
   for (int v = 0; v < numVertexes; v++)
   {
      Vertex *p = vertexes[v];
      p->x -= xc;
      p->y -= yc;
      p->z -= zc;
   }
   glutPostRedisplay();
}


void move()
{
   for (int v = 0; v < numVertexes; v++)
      vertexes[v]->clearForce();

   for (int i = 0; i < numVertexes; i++)
      for (int j = 0; j < i; j++)
      {
         Vertex *pu = vertexes[i];
         Vertex *pv = vertexes[j];
         Vector d(pu, pv);
         double len = d.len();

         // Compute repulsive forces
         Vector repel(d, kRepulsion / (len * len));
         *vertexes[i] += repel;
         *vertexes[j] -= repel;

         // Compute attractive forces.
         if (adj[i][j])
         {
            double att = kAttraction * (len - springLength);
            Vector attract(d, att);
            *vertexes[i] -= attract;
            *vertexes[j] += attract;
         }
      }

   // Move vertexes according to forces
   kineticEnergy = 0;
   for (int v = 0; v < numVertexes; v++)
   {
      vertexes[v]->update();
      kineticEnergy += vertexes[v]->kineticEnergy();
   }
}


// This function is called to display the scene.
void display ()
{
   glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
   glMatrixMode(GL_MODELVIEW);
   glLoadIdentity();
   glPushMatrix();
   glTranslatef(0, 0, -howFar);
   glRotatef(180 * xMouse, 0, 1, 0);
   glRotatef(- 180 * yMouse, 0, 0, 1);
   if (graphReady)
   {

      // Draw vertices
      glColor3d(1, 1, 0);
      for (int v = 0; v < numVertexes; v++)
         vertexes[v]->draw();

      // Draw edges
      glColor3d(0, 1, 1);
      glBegin(GL_LINES);
      for (int i = 0; i < numVertexes; i++)
         for (int j = 0; j < i; j++)
            if (adj[i][j])
            {
               vertexes[i]->setVertex();
               vertexes[j]->setVertex();
            }
      glEnd();
      glPopMatrix();

      ostringstream os;
      os << fixed << setprecision(5) <<
      "DT: " << DT <<
      "  Cycle: " << cycle <<
      "  KE: " << kineticEnergy << ends;
      const char *p = os.str().c_str();

      glColor3d(1, 1, 1);
      glTranslatef(0, 0, -10);
      glRasterPos2f(-2.5, -3);
      while (*p)
         glutBitmapCharacter(GLUT_BITMAP_TIMES_ROMAN_24, *p++);
   }
   glutSwapBuffers();
}

// This function is called when there is nothing else to do.
void idle ()
{
   for (int c = 0; c < DisplayRate; c++)
   {
      move();
      cycle++;
   }
   glutPostRedisplay();
}

void mouseMovement (int mx, int my)
{
   // Normalize mouse coordinates.
   xMouse = double(mx) / double(width);
   yMouse = 1 - double(my) / double(height);
   glutPostRedisplay();
}

// Respond to window resizing, preserving proportions.
// Parameters give new window size in pixels.
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
// Parameters give key code and mouse coordinates.
void graphicKeys (unsigned char key, int x, int y)
{
   switch (key)
   {
      case ' ':
         glutIdleFunc(idle);
         break;

      case '-':
         DT /= 1.2;
         break;

      case '+':
         DT *= 1.2;
         break;

      case '[': // Nearer
         howFar -= 0.5;
         break;

      case ']': // Further
         howFar += 0.5;
         break;

      case 'q':
      case 27: // Quit
         exit(0);
   }
   glutPostRedisplay();
}

void select(int entry)
{
   ReadGraph(entry);
}

int main (int argc, char **argv)
{
   // GLUT initialization.
   glutInit(&argc, argv);
   glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGB);
   glutInitWindowSize(width, height);
   glutCreateWindow("3D Vertex Layout");

   // Register call backs.
   glutDisplayFunc(display);
   glutReshapeFunc(reshapeMainWindow);
   glutKeyboardFunc(graphicKeys);
   glutMotionFunc(mouseMovement);

   // OpenGL initialization
   glEnable(GL_DEPTH_TEST);

   int menu = glutCreateMenu(select);
   int entryNum = 0;

   glutSetMenu(menu);
   glutAttachMenu(GLUT_RIGHT_BUTTON);
   struct _finddata_t fd;
   long dir = _findfirst("*.dat", &fd);
   while (true)
   {
      glutAddMenuEntry(fd.name, entryNum++);
      names.push_back(string(fd.name));
      if (_findnext(dir, &fd))
         break;
   }
   cout << "Spring links\n\n" <<
        names.size() << " files available.\n" <<
        "Right click in graphics window to select file.\n"
        "Mouse rotates image\n"
        "Space starts motion\n"
        "  +  faster\n"
        "  -  slower\n"
        "  [  nearer\n"
        "  ]  further\n"
        "  q  quit\n"
        " ESC quit\n";

   // Enter GLUT loop.
   glutMainLoop();
   return 0;
}
