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

const double INF = 1e6;
int numSelected = 0;
enum { WAITING, SEARCHING, FINISHED } mode;
bool heuristics = false;

double sqr(int n)
{
   return n * n;
}

struct Node
{
   Node(char cx, char cy) :
         x(cx - 'a' + 1),
         y(cy - 'a' + 1),
         num(10 * x + y),
         g(INF),
         h(0),
         selected(0),
         open(false),
         closed(false) {}

   void draw() const
   {
      switch (selected)
      {
         default:
         case 0:
            glColor3d(0, 0, 0);
            break;
         case 1:
            glColor3d(1, 0, 0);
            break;
         case 2:
            glColor3d(0, 1, 0);
            break;
      }
      glVertex2i(x, y);
   }
   void select(int sel) const
   {
      selected = sel;
   }
   double f() const
   {
      return g + h;
   }
   int x;
   int y;
   int num;
   mutable double g;
   mutable double h;
   mutable int selected;
   mutable bool open;
   mutable bool closed;
};

bool operator<(const Node & m, const Node & n)
{
   return m.num < n.num;
}

double euc(const Node & m, const Node & n)
{
   return sqrt(sqr(m.x - n.x) + sqr(m.y - n.y));
}

ostream & operator<<(ostream & os, const Node & n)
{
   return os << setw(2) << n.num;
}

struct Edge
{
   Edge(set<Node>::const_iterator f, set<Node>::const_iterator s) : first(f), second(s), used(false) {}
   set<Node>::const_iterator first, second;
   void draw() const
   {
      if (used)
         glColor3d(0, 0, 1);
      else
         glColor3d(0, 0, 0);
      glVertex2i(first->x, first->y);
      glVertex2i(second->x, second->y);
   }

   mutable bool used;
};

bool operator<(const Edge & e, const Edge & f)
{
   return *e.first < *f.first || *e.second < *f.second;
}

ostream & operator<<(ostream & os, const Edge & e)
{
   return os << '(' << setw(2) << e.first->num << ", " << setw(2) << e.second->num << ')';
}

set<Node> nodes;
set<Edge> edges;
set<Node>::iterator start;
set<Node>::iterator goal;

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

   glColor3i(0, 0, 0);
   glBegin(GL_POINTS);
   for (set<Node>::const_iterator i = nodes.begin(); i != nodes.end(); ++i)
      i->draw();
   glEnd();

   glLineWidth(5);
   glBegin(GL_LINES);
   for (set<Edge>::const_iterator e = edges.begin(); e != edges.end(); ++e)
      e->draw();
   glEnd();

   if (heuristics)
   {
      glColor3d(0, 1, 0);
      glLineWidth(1);
      glBegin(GL_LINES);
      for (set<Node>::const_iterator i = nodes.begin(); i != nodes.end(); ++i)
         for (set<Node>::const_iterator j = nodes.begin(); j != nodes.end(); ++j)
            if (*i < *j)
            {
               glVertex2i(i->x, i->y);
               glVertex2i(j->x, j->y);
            }
      glEnd();
   }

   glutSwapBuffers();

   ostringstream os;
   switch (mode)
   {
      case WAITING:
         os << "Waiting. ";
         break;
      case SEARCHING:
         os << "Searching: " << *start << ' ' << *goal;
         break;
      case FINISHED:
         os << "Finished. ";
         break;
   }
   glutSetWindowTitle(os.str().c_str());
}

bool searching(set<Node>::iterator goal)
{
   double min = INF;
   set<Node>::iterator x = nodes.end();
   for (set<Node>::iterator i = nodes.begin(); i != nodes.end(); ++i)
   {
      if (i->open && min > i->f())
      {
         min = i->f();
         x = i;
      }
   }
   if (x == nodes.end())
   {
      cerr << "Failed: no minimum\n";
      return false;
   }
   if (x == goal)
   {
      cerr << "Succeeded!\n";
      return false;
   }
   x->open = false;
   x->closed = true;

   set<Node>::iterator y = nodes.end();
   for (set<Edge>::iterator e = edges.begin(); e != edges.end(); ++e)
   {
      if (e->first == x)
      {
         y = e->second;
         e->used = true;
      }
      else if (e->second == x)
      {
         y = e->first;
         e->used = true;
      }
      if (y != nodes.end())
      {
         bool better;
         if (!y->closed)
         {
            int t = x->g + euc(*x, *y);
            if (!y->open)
            {
               y->open = true;
               better = true;
            }
            else if (t < y->g)
               better = true;
            else
               better = false;
            if (better)
               y->g = t;
         }
      }
   }
   return true;
}

void idle()
{
   switch (mode)
   {
      case WAITING:
         if (numSelected == 2)
         {
            for (set<Node>::iterator i = nodes.begin(); i != nodes.end(); ++i)
            {
               if (i->selected == 1)
                  start = i;
               if (i->selected == 2)
                  goal = i;
            }
            cout << "Start: " << *start << endl;
            cout << "Goal:  " << *goal << endl;
            for (set<Node>::iterator i = nodes.begin(); i != nodes.end(); ++i)
            {
               i->h = euc(*i, *goal);
               i->g = INF;
               i->open = false;
               i->closed = false;
            }
            start->g = 0;
            start->open = true;
            mode = SEARCHING;
         }
         break;

      case SEARCHING:
         if (!searching(goal))
            mode = FINISHED;
         break;

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
            int mx = int(x * 11.0 / windowWidth + 0.5);
            int my = 8 - int(y * 8.0 / windowHeight + 0.5);

            for (set<Node>::iterator i = nodes.begin(); i != nodes.end(); ++i)
               if (i->x == mx && i->y == my)
                  i->select(++numSelected);
            break;

         case FINISHED:
            for (set<Node>::iterator i = nodes.begin(); i != nodes.end(); ++i)
               i->select(0);
            for (set<Edge>::iterator e = edges.begin(); e != edges.end(); ++e)
               e->used = false;
            numSelected = 0;
            mode = WAITING;
            break;
      }
   }
   glutPostRedisplay();
}


void keyboard (unsigned char key, int x, int y)
{
   switch (key)
   {
      case 'h':
         heuristics = !heuristics;
         break;
      case '-':
         break;
      case '+':
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

   glutInit(&argc, argv);
   glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGB | GLUT_DEPTH);
   glutInitWindowSize(windowWidth, windowHeight);
   glutInitWindowPosition(300, 300);
   glutCreateWindow("A*");
   glutDisplayFunc(display);
   glutMouseFunc(mouse);
   glutKeyboardFunc(keyboard);
   glutReshapeFunc(reshape);
   glutIdleFunc(idle);
   glEnable(GL_DEPTH_TEST);
   glClearColor(1, 1, 1, 1);
   glPointSize(15);
   glutMainLoop();
   return 0;
}

