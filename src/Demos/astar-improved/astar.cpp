/* Link with these libraries (omit those unneeded):
 *  libcugl
 *  libglui32
 *  libglut32
 *  libopengl32
 *  libglu32
 */

#include <iostream>
#include <iomanip>
#include <sstream>
#include <fstream>
#include <set>
#include <cmath>
#include <GL/glut.h>

using namespace std;

// Current window dimensions
int windowWidth = 1200;
int windowHeight = 800;

const double DELAY = 500; // milliseconds between cycles
const double INF = 1e6; // a large number
const double POINT_SIZE = 15;

bool coordinates = false; // display coordinates

int nodesChosen = 0; // 1 if start chosen, 2 if start and goal chosen
int numEdgesTraversed = 0; // edges traversed
int numCycles = 0;
string message = "";
enum Mode { WAITING, SEARCHING, FAILED, FINISHED } mode;

inline double sqr(int n)
{
   return n * n;
}

// A node (vertex) of the graph we are processNode in.
struct Node
{
   Node(char cx, char cy) :
      x(cx - 'a' + 1),
      y(cy - 'a' + 1),
      distFromStart(INF),
      distToGoal(0),
      terminal(0),
      open(false),
      closed(false) {}

   // Draw the node, using colour to indicate status.
   void draw() const
   {
      // Assume GL_POINTS mode
      if (terminal == 1)
         glColor3d(1, 0, 0);  // Red for start node
      else if (terminal == 2)
         glColor3d(0, 1, 0);  // Green for goal node
      else if (open)
         glColor3d(0, 0, 1);  // Blue for open
      else
         glColor3d(0, 0, 0);  // Black for closed
      glVertex2i(x, y);
   }

   // Highlight the node with a large point size.
   void highlight() const
   {
      glPointSize(3 * POINT_SIZE);
      glColor3d(1, 1, 0);  // Yellow for highlit node
      glBegin(GL_POINTS);
      glVertex2i(x, y);
      glEnd();
      glPointSize(POINT_SIZE);
   }

   // Node is start (sel = 1) or goal (sel = 2).
   void select(int sel) const
   {
      terminal = sel;
   }

   // The index of a node is used only to provide an ordering for set<Node>
   int getIndex() const
   {
      return 10 * x + y;
   }

   int x;
   int y;
   mutable double distFromStart;
   mutable double distToGoal;
   mutable int terminal; // start = 1, goal = 2, otherwise 0.
   mutable bool open; // in open set
   mutable bool closed; // in closed set
};

bool operator<(const Node & m, const Node & n)
{
   return m.getIndex() < n.getIndex();
}

double euclideanDistance(const Node & m, const Node & n)
{
   return sqrt(sqr(m.x - n.x) + sqr(m.y - n.y));
}

ostream & operator<<(ostream & os, const Node & n)
{
   return os << setw(2) << n.getIndex();
}

// An edge joins two nodes in the graph we are processNode.
struct Edge
{
   Edge(set<Node>::const_iterator f, set<Node>::const_iterator s)
      : first(f), second(s), traversed(false) {}

   void draw() const
   {
      // Assume GL_LINES mode
      if (traversed)
         glColor3d(1, 0, 0);
      else
         glColor3d(0.6, 0.6, 0.6);
      glVertex2i(first->x, first->y);
      glVertex2i(second->x, second->y);
   }

   set<Node>::const_iterator first, second;
   mutable bool traversed; // This edge has been used in the search.
};

// Ordering for set<Edge>.
bool operator<(const Edge & e, const Edge & f)
{
   return *e.first < *f.first || *e.second < *f.second;
}

ostream & operator<<(ostream & os, const Edge & e)
{
   return os << '(' << setw(2) << e.first->getIndex() << ", " << setw(2) << e.second->getIndex() << ')';
}

set<Node> nodes;
set<Edge> edges;
set<Node>::iterator start;
set<Node>::iterator goal;
set<Node>::iterator current = nodes.end();

// Read a graph from an input file: name is hard-wired.
bool readGraph()
{
   char fn[] = "graphdata.txt";
   ifstream ifs(fn);
   if (!ifs)
   {
      cerr << "Could not open " << fn << ".\n";
      return false;
   }
   while (true)
   {
      // Coordinates are letters: a, b, c, d, ...
      // An edge is denoted by two letter pairs.  E.g., 'ac af'
      // '# #' terminates the input.
      char cx1, cy1, cx2, cy2;
      ifs >> cx1 >> cy1;
      if (cx1 == '#')
         break;
      ifs >> cx2 >> cy2;
      Node n1(cx1, cy1);
      Node n2(cx2, cy2);
      set<Node>::iterator i1 = nodes.insert(n1).first;
      set<Node>::iterator i2 = nodes.insert(n2).first;
      edges.insert(Edge(i1, i2));
   }
   return true;
}

void display (void)
{
   glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
   glMatrixMode(GL_MODELVIEW);
   glLoadIdentity();

   // Display nodes.
   glBegin(GL_POINTS);
   for (set<Node>::const_iterator i = nodes.begin(); i != nodes.end(); ++i)
      i->draw();
   glEnd();

   // Display edges.
   glLineWidth(5);
   glBegin(GL_LINES);
   for (set<Edge>::const_iterator e = edges.begin(); e != edges.end(); ++e)
      e->draw();
   glEnd();

   // Highlight current node and show distance to goal.
   if (current != nodes.end())
   {
      current->highlight();
      glColor3d(0, 1, 0);
      glLineWidth(1);
      glBegin(GL_LINES);
      glVertex2i(current->x, current->y);
      glVertex2i(goal->x, goal->y);
      glEnd();
   }

   // Display letter coordinates.
   if (coordinates)
   {
      glColor3d(0, 0, 0);
      for (int x = 1; x <= 26; ++x)
      {
         glRasterPos2f(x, 7.5);
         glutBitmapCharacter(GLUT_BITMAP_8_BY_13, x - 1 + 'a');
      }
      for (int y = 1; y <= 26; ++y)
      {
         glRasterPos2f(0.5, y);
         glutBitmapCharacter(GLUT_BITMAP_8_BY_13, y - 1 + 'a');
      }
   }

   // Display current message.
   glColor3d(0, 0, 0);
   glRasterPos2f(0.5, 7.5);
   size_t len = message.size();
   for (size_t i = 0; i < len; i++)
      glutBitmapCharacter(GLUT_BITMAP_TIMES_ROMAN_24, message[i]);
   glutSwapBuffers();
}

// Update the current message when state changes.
void updateMessage()
{
   ostringstream os;
   switch (mode)
   {
      case WAITING:
         switch (nodesChosen)
         {
            case 0:
               os << "Choose start node.";
               break;
            case 1:
               os << "Choose goal node.";
               break;
         }
         break;
      case SEARCHING:
         os << "Searching:" << setw(3) << numCycles << " steps; " << numEdgesTraversed << " edges.";
         break;
      case FAILED:
         os << "FAILED! Click to try again.";
         break;
      case FINISHED:
         os << "Finished after " << numCycles << " steps. " << numEdgesTraversed << " edges.  Click to try again.";
         break;
   }
   message = os.str();
}

// Perform one iteration of the A* algorithm.
void processNode(int val)
{
   ++numCycles;

   // Find node with smallest g+h.
   double distSmallest = INF;
   set<Node>::iterator x = nodes.end();
   for (set<Node>::iterator i = nodes.begin(); i != nodes.end(); ++i)
   {
      if (i->open)
      {
         double distTotal = i->distFromStart + i->distToGoal;
         if (distSmallest > distTotal)
         {
            distSmallest = distTotal;
            x = i;
         }
      }
   }

   // Check for termination or failure.
   if (x == nodes.end() || x == goal)
   {
      mode = x == goal ? FINISHED : FAILED;
      updateMessage();
      return;
   }

   // Make node current and move it from Open to Closed.
   current = x;
   x->open = false;
   x->closed = true;

   // Find edges that end at the current node.
   set<Node>::iterator y = nodes.end();
   for (set<Edge>::iterator e = edges.begin(); e != edges.end(); ++e)
   {
      if (e->first == x)
      {
         y = e->second;
         if (!e->traversed)
         {
            e->traversed = true;
            ++numEdgesTraversed;
         }
      }
      else if (e->second == x)
      {
         y = e->first;
         if (!e->traversed)
         {
            e->traversed = true;
            ++numEdgesTraversed;
         }
      }
      if (y != nodes.end())
      {
         bool better = true;
         if (!y->closed)
         {
            // Add the node to Open and reduce its distance.
            double totalDist = x->distFromStart + euclideanDistance(*x, *y);
            if (!y->open)
               y->open = true;
            else
               better = totalDist < y->distFromStart;
            if (better)
               y->distFromStart = totalDist;
         }
      }
   }
   updateMessage();
   glutTimerFunc(DELAY, processNode, 0);
}

// Idle function starts things going when the user has selected two nodes.
void idle()
{
   switch (mode)
   {
      case WAITING:
         if (nodesChosen == 2)
         {
            // User has chosen start and goal nodes.
            for (set<Node>::iterator i = nodes.begin(); i != nodes.end(); ++i)
            {
               if (i->terminal == 1)
                  start = i;
               if (i->terminal == 2)
                  goal = i;
            }

            // Initialize nodes.
            for (set<Node>::iterator i = nodes.begin(); i != nodes.end(); ++i)
            {
               i->distToGoal = euclideanDistance(*i, *goal);
               i->distFromStart = INF;
               i->open = false;
               i->closed = false;
            }

            // Initialize and start searching.
            nodesChosen = 0;
            numEdgesTraversed = 0;
            start->distFromStart = 0;
            start->open = true;
            numCycles = 0;
            mode = SEARCHING;
            glutTimerFunc(DELAY, processNode, 0);
         }
         break;

      case SEARCHING:
      case FAILED:
      case FINISHED:
         break;
   }
   glutPostRedisplay();
}

void mouse(int button, int state, int x, int y)
{
   if (button == GLUT_LEFT_BUTTON && state == GLUT_DOWN)
   {
      switch (mode)
      {
         case WAITING:
            {
               // If mouse is on a node, select that node.
               int mx = int(x * 11.0 / windowWidth + 0.5);
               int my = 8 - int(y * 8.0 / windowHeight + 0.5);

               for (set<Node>::iterator i = nodes.begin(); i != nodes.end(); ++i)
                  if (i->x == mx && i->y == my && i->terminal == 0)
                     i->select(++nodesChosen);
            }
            break;

         case SEARCHING:
            break;

         case FAILED:
         case FINISHED:
            // Clean up display and selections.
            for (set<Node>::iterator i = nodes.begin(); i != nodes.end(); ++i)
            {
               i->select(0);
               i->open = false;
            }
            for (set<Edge>::iterator e = edges.begin(); e != edges.end(); ++e)
               e->traversed = false;
            current = nodes.end();
            mode = WAITING;
            break;
      }
      updateMessage();
   }
   glutPostRedisplay();
}


void keyboard (unsigned char key, int x, int y)
{
   switch (key)
   {
      case 'c':
         coordinates = !coordinates;
         updateMessage();
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
   glOrtho(0, 11, 0, 8, -1, 1);
   glutPostRedisplay();
}

int main(int argc, char *argv[])
{
   if (!readGraph())
      return 1;

   cout <<
        "A* Demonstration\n\n"
        " 'c' toggles coordinate display.\n"
        " 'f' sets full screen mode.\n\n"
        "Choose start and goal nodes node by clicking them.\n"
        "Node colurs:\n"
        "  Red   = start\n"
        "  Green = goal\n"
        "  Black = closed\n"
        "  Blue  = open\n";

   glutInit(&argc, argv);
   glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGB | GLUT_DEPTH);
   glutInitWindowSize(windowWidth, windowHeight);
   glutInitWindowPosition(100, 100);
   glutCreateWindow("A*");
   glutDisplayFunc(display);
   glutMouseFunc(mouse);
   glutKeyboardFunc(keyboard);
   glutReshapeFunc(reshape);
   glutIdleFunc(idle);
   glEnable(GL_DEPTH_TEST);
   glClearColor(1, 1, 1, 1);
   glPointSize(15);
   updateMessage();
   glutMainLoop();
   return 0;
}

