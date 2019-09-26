#include "cugl.h"
#include <GL/glui.h>
#include <GL/glut.h>
#include <GL/tube.h>

#include <cctype>
#include <io.h>
#include <cmath>
#include <fstream>
#include <iostream>
#include <sstream>
#include <iomanip>
#include <string>
#include <windows.h>

using namespace std;
using namespace cugl;

// Time increment for integration (ms)
const double DT = 0.001;

// Gravitational constant (f/s^2)
const double G = 32;

// Gravity - set by user interface
float uiGrav = 32;

// Return sign of number as {-1, 0, 1}
inline double sgn(double x)
{
   return x == 0 ? 0 : x / fabs(x);
}

// GL texture index for floor
GLuint floorName;

// GL texture index for trampoline
GLuint trampName;

// Pointer to UI controls
GLUI *glui;

// Forward declaration for UI callback
void control(int action);


//////////////////////////////// Trampoline

class Trampoline
{
public:
   // Dummy constructor
   Trampoline() {}

   // Constructor that is used
   Trampoline(int index, int & actionCounter, float ix, float iy, float iz, float it);

   // GL drawing function
   void draw();

   // Return true if p is within the trampoline boundary
   bool within(Point p);

   // Perform a user interface action
   void uiAction(int act);

   // Return the plane of the trampoline
   Plane getPlane()
   {
      return pl;
   }

   // Return the friction of the trampoline
   double getFriction()
   {
      return 50 - 0.5 * efficiency;
   }

   // Copy important variables
   Trampoline operator=(const Trampoline &t);

   void print(ostream & os);

   // Write important variables
   friend ostream & operator<<(ostream & os, Trampoline *pt);

private:

   // Compute corner coordinates for position parameters
   void transformCorners();

   // Nunmber of trampoline, used in UI
   int index;

   // Computed positions of corners.
   Point corner[4];

   // The plane of the trampoline.
   Plane pl;

   // Dimensions
   float width;
   float length;

   // Position
   float x;
   float y;
   float z;

   // Initial position
   float ix;
   float iy;
   float iz;

   // Rotations (in degrees)
   float slope;
   float twist;
   float itwist;  // initial value

   // Efficiency
   int efficiency;

   // UI action codes
   int actResetStart;
   int actUpdate;
   int actEff;

   // Pointers to UI controls
   GLUI_Spinner *ctlWidth;
   GLUI_Spinner *ctlLength;
   GLUI_Spinner *ctlNS;
   GLUI_Spinner *ctlEW;
   GLUI_Spinner *ctlUD;
   GLUI_Spinner *ctlSlope;
   GLUI_Spinner *ctlTwist;
   GLUI_Spinner *ctlEff;
};


const int INIT_EFF = 90;

// Construct a trampoline
Trampoline::Trampoline(int index, int & actionCounter, float ix, float iy, float iz, float it) :
   index(index),
   width(10),
   length(10),
   x(ix), y(iy), z(iz),
   ix(ix), iy(iy), iz(iz),
   slope(0),
   twist(it),
   itwist(it),
   efficiency(INIT_EFF)
{
   actResetStart = actionCounter++;
   actUpdate = actionCounter++;
   actEff = actionCounter++;

   string iden = "Trampoline " + string(1, char(index + '0'));
   cerr << iden << endl;

   GLUI_Panel *main = glui->add_panel(iden.c_str());
   GLUI_Panel *size = glui->add_panel_to_panel(main, "Size");
   ctlWidth = glui->add_spinner_to_panel(size, "Width", GLUI_SPINNER_FLOAT, &width, actUpdate, control);
   ctlLength = glui->add_spinner_to_panel(size, "Length", GLUI_SPINNER_FLOAT, &length, actUpdate, control);

   glui->add_column_to_panel(main, true);

   GLUI_Panel *pos = glui->add_panel_to_panel(main, "Position");
   ctlNS = glui->add_spinner_to_panel(pos, "X", GLUI_SPINNER_FLOAT, &y, actUpdate, control);
   ctlEW = glui->add_spinner_to_panel(pos, "Y", GLUI_SPINNER_FLOAT, &x, actUpdate, control);
   ctlUD = glui->add_spinner_to_panel(pos, "Z", GLUI_SPINNER_FLOAT, &z, actUpdate, control);

   glui->add_column_to_panel(main, true);

   GLUI_Panel *ori = glui->add_panel_to_panel(main, "Orientation");
   ctlSlope = glui->add_spinner_to_panel(ori, "Slope", GLUI_SPINNER_FLOAT, &slope, actUpdate, control);
   ctlTwist = glui->add_spinner_to_panel(ori, "Twist", GLUI_SPINNER_FLOAT, &twist, actUpdate, control);

   ctlEff = glui->add_spinner_to_panel(main, "Efficiency", GLUI_SPINNER_INT, &efficiency, actEff, control);

   glui->add_column_to_panel(main, true);

   glui->add_button_to_panel(size, "Reset", actResetStart, control);

   // Set limits
   ctlWidth->set_float_limits(0, 100);
   ctlLength->set_float_limits(0, 100);
   ctlNS->set_float_limits(-100, 100);
   ctlEW->set_float_limits(-100, 100);
   ctlUD->set_float_limits(0, 100);
   ctlSlope->set_float_limits(-90, 90);
   ctlTwist->set_float_limits(-180, 180);
   ctlEff->set_int_limits(0, 150);

   transformCorners();
}

// Transform corner points according to GLUI controls
void Trampoline::transformCorners()
{
   double hLength = 0.5 * length;
   Point c0(    0, -hLength);
   Point c1(width, -hLength);
   Point c2(width,  hLength);
   Point c3(    0,  hLength);

   // Construct rotation matrix from twist and slope
   Matrix m = Matrix(K, radians(twist)) * Matrix(J, radians(slope));;
   Vector disp(x, y, z);
   corner[0] = m.apply(c0) + disp;
   corner[1] = m.apply(c1) + disp;
   corner[2] = m.apply(c2) + disp;
   corner[3] = m.apply(c3) + disp;

   // Construct the plane of the trampoline
   pl = Plane(corner[1], corner[2], corner[3]);
//   pl.normalize();
}

// Respond to user interface controls.
void Trampoline::uiAction(int action)
{
   if (action == actResetStart)
   {
      ctlWidth->set_float_val(10);
      ctlLength->set_float_val(10);
      ctlNS->set_float_val(iy);
      ctlEW->set_float_val(ix);
      ctlUD->set_float_val(iz);
      ctlSlope->set_float_val(0);
      ctlTwist->set_float_val(itwist);
      ctlEff->set_int_val(90);
      transformCorners();
   }
   else if (action == actUpdate)
      transformCorners();
}

// Draw a trampoline
void Trampoline::draw()
{
   setMaterial(WHITE);
   glEnable(GL_TEXTURE_2D);
   glBindTexture(GL_TEXTURE_2D, trampName);
   glBegin(GL_QUADS);
   glTexCoord2d(0, 0);
   corner[0].draw();
   glTexCoord2d(1, 0);
   corner[1].draw();
   glTexCoord2d(1, 1);
   corner[2].draw();
   glTexCoord2d(0, 1);
   corner[3].draw();
   glEnd();
   glDisable(GL_TEXTURE_2D);

   setMaterial(RUBY);
   double pts[][3] =
   {
      { corner[0][0], corner[0][1], corner[0][2] },
      { corner[1][0], corner[1][1], corner[1][2] },
      { corner[2][0], corner[2][1], corner[2][2] },
      { corner[3][0], corner[3][1], corner[3][2] },
      { corner[0][0], corner[0][1], corner[0][2] },
      { corner[1][0], corner[1][1], corner[1][2] },
      { corner[2][0], corner[2][1], corner[2][2] },
   };
   gleSetJoinStyle(TUBE_JN_ROUND);
   glePolyCylinder(7, pts, NULL, 0.2);
}

// Copy a trampoline and recompute corner coordinates
Trampoline Trampoline::operator=(const Trampoline &t)
{
   pl = t.pl;
   width = t.width;
   length = t.length;
   x = t.x;
   y = t.y;
   z = t.z;
   ix = t.ix;
   iy = t.iy;
   z = t.z;
   slope = t.slope;
   twist = t.twist;
   efficiency = t.efficiency;
   transformCorners();
   return *this;
}

// Determine whether p is inside the rectangular volume
// defined by moving the trampoline in the direction of its normal
bool Trampoline::within(Point p)
{
   Vector w(p, corner[0]);
   Vector u(corner[1], corner[0]);
   Vector v(corner[3], corner[0]);
   Vector vu = cross(v, u);
   Vector wu = cross(w, u);
   Vector wv = cross(w, v);
   double a = 0, b = 0;
   if (vu[0] != 0)
   {
      a = -wv[0] / vu[0];
      b =  wu[0] / vu[0];
   }
   else if (vu[1] != 0)
   {
      a = -wv[1] / vu[1];
      b =  wu[1] / vu[1];
   }
   else if (vu[2] != 0)
   {
      a = -wv[2] / vu[2];
      b =  wu[2] / vu[2];
   }
   return 0 <= a && a <= 1 && 0 <= b && b <= 1;
}

void Trampoline::print(ostream & os)
{
   os << "Trampoline " << index << endl <<
   resetiosflags(ios::floatfield) << setiosflags(ios::fixed) << setprecision(3) <<
   "Dimensions: " <<
   setw(9) << width <<
   setw(9) << length << endl <<
   "Position:   " <<
   setw(9) << x <<
   setw(9) << y <<
   setw(9) << z << endl <<
   "Plane:      " << setw(9) << pl << endl <<
   "Corner 0:    " << setw(9) << corner[0] << endl <<
   "Corner 1:    " << setw(9) << corner[1] << endl <<
   "Corner 2:    " << setw(9) << corner[2] << endl <<
   "Corner 3:    " << setw(9) << corner[3] << endl <<
   "Slope:      " << setw(9) << slope << endl <<
   "Rotation:   " << setw(9) << twist << endl <<
   "Efficiency: " << setw(9) << efficiency << endl << endl;
}

// Write the important variables of a trampoline
ostream & operator<<(ostream & os, Trampoline *pTr)
{
   os <<
   pTr->width << "  " << pTr->length << "  " <<
   pTr->pl << "  " << endl <<
   pTr->x << "  " << pTr->y << "  " << pTr->z << "  " <<
   pTr->slope << "  " << pTr->twist << endl;
   return os << endl;
}

// Pointers to the trampolines
Trampoline *pTramp1;
Trampoline *pTramp2;



//////////////////////////////////////// Acrobat

const double ACROBAT_RADIUS = 0.5;


class Acrobat
{
public:
   // Dummy constructor
   Acrobat() {}

   // The constructor that is actually used
   Acrobat(int & actionCounter, Point iPos);

   // Move an acrobat with the given acceleration
   void move(const Vector & acc = Vector(0, 0, -uiGrav));

   // Start the motion
   void start();

   // Draw an acrobat using GL
   void draw();

   // Update the motion
   void actUpdate();

   // Respond to user interface actions
   void uiAction(int action);

   // Copy important variables
   Acrobat operator=(const Acrobat &a);

   void print(ostream & os);

   // Getters
   Point getPos()
   {
      return pos;
   }
   int getTime()
   {
      return int(time);
   }
   double getRadius()
   {
      return radius;
   }
   double getEnergy()
   {
      return mass * (G * pos[2] + 0.5 * dot(vel, vel));
   }

   // Setters
   void setPos(const Point & newPos)
   {
      pos = newPos;
   }
   void setVel(const Vector & newVel)
   {
      vel = newVel;
   }

private:
   // State of motion
   enum { STOPPED, FLYING, COLLIDING1, COLLIDING2 } state;

   // Reset control moves acrobat to this position
   Point initPos;

   // Start control moves acrobat to this position
   Point startPos;

   // Current position and velocity
   Point pos;
   Vector vel;

   // Distances to trampolines
   double distToT1;
   double distToT2;

   // Time from start of motion
   double time;

   // Constant attributes
   double mass;
   double radius;

   // UI callback codes
   int actMove;
   int actResetStart;
   int actStart;
   int actStop;
   int actGotoStart;

   // Pointers to UI controls
   GLUI_Spinner *ctlNS;
   GLUI_Spinner *ctlEW;
   GLUI_Spinner *ctlUD;
   GLUI_Button *ctlReset;
   GLUI_Button *ctlPosition;
};

// Constructor
Acrobat::Acrobat(int & actionCounter, Point iniPos) :
   state(STOPPED),
   initPos(iniPos),
   startPos(iniPos),
   pos(iniPos),
   vel(Vector()),
   time(0),
   mass(0.02),
   radius(ACROBAT_RADIUS)
{
   actMove  = actionCounter++;
   actResetStart = actionCounter++;
   actStart = actionCounter++;
   actStop  = actionCounter++;
   actGotoStart = actionCounter++;

   GLUI_Panel *main = glui->add_panel("Acrobat");

   GLUI_Panel *place = glui->add_panel_to_panel(main, "Start position");
   ctlNS = glui->add_spinner_to_panel(place, "X", GLUI_SPINNER_FLOAT, &startPos[1], actMove, control);
   ctlEW = glui->add_spinner_to_panel(place, "Y", GLUI_SPINNER_FLOAT, &startPos[0], actMove, control);
   ctlUD = glui->add_spinner_to_panel(place, "Z", GLUI_SPINNER_FLOAT, &startPos[2], actMove, control);

   glui->add_column_to_panel(main);

   ctlReset = glui->add_button_to_panel(main, "Reset start position", actResetStart, control);
   ctlPosition = glui->add_button_to_panel(main, "Go to start position", actGotoStart, control);
   glui->add_button_to_panel(main, "Start", actStart, control);
   glui->add_button_to_panel(main, "Stop", actStop, control);

}

// Use acceleration to find new velocity and position
void Acrobat::move(const Vector & acc)
{
   vel += acc * DT;
   pos += vel * DT;
   time += DT;
}

// Draw the acrobat at its current position
void Acrobat::draw()
{
   glPushMatrix();
   pos.translate();
   setMaterial(TURQUOISE);
   glutSolidSphere(radius, 20, 20);
   glPopMatrix();
}

// Start the motion
void Acrobat::start()
{
//   pTramp1->print(cerr);
//   pTramp2->print(cerr);
   pos = startPos;
   setVel(Vector());
   state = FLYING;
   distToT1 = dist(pTramp1->getPlane(), pos);
   distToT2 = dist(pTramp2->getPlane(), pos);
   time = 0;
}

// Respond to UI actions
void Acrobat::uiAction(int action)
{
   if (action == actResetStart)
   {
      // Reset start position of acrobat
      ctlNS->set_float_val(initPos[1]);
      ctlEW->set_float_val(initPos[0]);
      ctlUD->set_float_val(initPos[2]);
      pos = startPos;
      time = 0;
   }
   else if (action == actStart)
   {
      // Move to start position and start motion
      start();
   }
   else if (action == actStop)
   {
      // Stop motion but leave acrobat in place
      state = STOPPED;
   }
   else if (action == actMove)
   {
      // Start position has been adjusted
      pos = startPos;
      time = 0;
   }
   else if (action == actGotoStart)
   {
      // Go to start position
      pos = startPos;
      setVel(Vector());
      state = STOPPED;
      time = 0;
   }
}

// Update acrobat motion by one time step
void Acrobat::actUpdate()
{
   Plane plane1 = pTramp1->getPlane();
   Plane plane2 = pTramp2->getPlane();

   double fric1 = pTramp1->getFriction();
   double fric2 = pTramp2->getFriction();

   Vector g(0, 0, -uiGrav);

   switch (state)
   {
      case STOPPED:
         // Do nothing if acrobat is stoped
         break;

      case FLYING:
         // Move acrobat through one time step.
         move();
         if (pos[2] < getRadius())
         {
            // The acrobat has landed on the floor.
            state = STOPPED;
            break;
         }

//         cerr << "T1: " << distToT1 << "  " << pos << "  " << dist(plane1, pos) << endl;
//         cerr << "T2: " << distToT2 << "  " << dist(plane2, pos) << endl;

         if (distToT1 * dist(plane1, pos) < 0)
         {
            // The acrobat has crossed the plane of a trampoline.
            // Calculate time of point of impact and position at this time.
            double impactTime = - (plane1.getA() * pos[0] + plane1.getB() * pos[1] + plane1.getC() * pos[2] + plane1.getD() * pos[3]) /
                                (plane1.getA() * vel[0] + plane1.getB() * vel[1] + plane1.getC() * vel[2]);
            Point p = pos + vel * impactTime;
            if (dist(pos, p) < 2 * dot(vel, plane1.normal()) * DT && pTramp1->within(p))
            {
               // The distance from the acrobat to the plane of the trampoline implies a collision.
               // Also, the impact point is within the boundary of the trampoline.
               pos = p;
               state = COLLIDING1;
            }
         }
         else if (distToT2 * dist(plane2, pos) < 0)
         {
            double impactTime = - (plane2.getA() * pos[0] + plane2.getB() * pos[1] + plane2.getC() * pos[2] + plane2.getD() * pos[3]) /
                                (plane2.getA() * vel[0] + plane2.getB() * vel[1] + plane2.getC() * vel[2]);
            Point p = pos + vel * impactTime;
            if (dist(pos, p) < 2 * dot(vel, plane2.normal()) * DT && pTramp2->within(p))
            {
               pos = p;
               state = COLLIDING2;
            }
         }
         break;

      case COLLIDING1:
         // Acrobat is colliding with Trampoline 1
         move(g - (dist(plane1, pos) / mass) * plane1.normal() - fric1 * vel);
         if (distToT1 * dist(plane1, pos) > 0)
            state = FLYING;
         break;

      case COLLIDING2:
         // Acrobat is colliding with Trampoline 2
         move(g - (dist(plane2, pos) / mass) * plane2.normal() - fric2 * vel);
         if (distToT2 * dist(plane2, pos) > 0)
            state = FLYING;
         break;
   }
}

// Copy important variables
Acrobat Acrobat::operator=(const Acrobat &a)
{
   mass = a.mass;
   initPos = a.initPos;
   startPos = a.startPos;
   pos = a.pos;
   vel = a.vel;
   time = a.time;
   radius = a.radius;
   return *this;
}

void Acrobat::print(ostream & os)
{
   os << "Acrobat" << endl <<
   resetiosflags(ios::floatfield) << setiosflags(ios::fixed) << setprecision(3) <<
   "Start position: " <<
   setw(9) << initPos[0] << "  " <<
   setw(9) << initPos[1] << "  " <<
   setw(9) << initPos[2] << endl << endl;
}

// Pointer to the acrobat
Acrobat *pAcrobat;


////////////////////////////////////////User Interface


// GLUI controls callback: sends message to all objects.
void control(int action)
{
   pTramp1->uiAction(action);
   pTramp2->uiAction(action);
   pAcrobat->uiAction(action);
   glutPostRedisplay();
}


// Save/Load

// Data from the viewing control panel to be saved with the state

// Distance from viewer to model
float uiDistance = 30;

// Height of viewer
float uiHeight = 10;

// Orientation of model
float uiOrientation = 0;

void printView(ostream & os)
{
   os << "View" << endl <<
   resetiosflags(ios::floatfield) << setiosflags(ios::fixed) << setprecision(3) <<
   "Distance: " << setw(9) << uiDistance << endl <<
   "Height:   " << setw(9) << uiHeight << endl <<
   "Rotation: " << setw(9) << uiOrientation << endl <<
   "Gravity:  " << setw(9) << uiGrav << endl << endl;
}

// Extension used for saved files
const char *FILE_EXTENSION = ".cds";
const char *FILE_PATTERN = "*.cds";

// This class summarizes the data to be stored and retrieved
class Data
{
public:

   // Dummy constructor
   Data() {}

   // Save current state to given filename
   void save(string filename);

   // Restore current state from given filename
   void load(string filename);

private:

   // Data stored in saved state
   Trampoline dtr1;
   Trampoline dtr2;
   Acrobat dab;
   float dviewDistance;
   float dviewHeight;
   float dviewRotate;
   float dviewGrav;
} data;

// Save current state: the filename has no extension
void Data::save(string filename)
{
   // Copy data
   dtr1 = *pTramp1;
   dtr2 = *pTramp2;
   dab = *pAcrobat;
   dviewDistance = uiDistance;
   dviewHeight = uiHeight;
   dviewRotate = uiOrientation;
   dviewGrav = uiGrav;

   // Construct full file name
   string fullname = filename + FILE_EXTENSION;
   ofstream file(fullname.c_str(), ios::binary);

   // Write data to file
   file.write((char*)(&data), sizeof(data));
   file.close();
   cout << "Configuration written to " << fullname << endl;
}

void Data::load(string filename)
{
   // Construct full file name
   string fullname = filename + FILE_EXTENSION;
   ifstream file(fullname.c_str(), ios::binary);

   // Read file
   file.read((char*)(&data), sizeof(data));
   file.close();
   cout << "Configuration read from " << fullname << endl;

   // Copy data
   *pTramp1 = dtr1;
   *pTramp2 = dtr2;
   *pAcrobat = dab;
   uiDistance = dviewDistance;
   uiHeight = dviewHeight;
   uiOrientation = dviewRotate;
   uiGrav = dviewGrav;

   // Synchronize controls
   glui->sync_live();
}

////////////////////////////////////////////// Graphics

// Main window identifier
int mainWindow;

// Initial size of graphics window.
const int WIDTH  = 600;
const int HEIGHT = 400;

// Current size of window.
int width  = WIDTH;
int height = HEIGHT;

// Bounds of viewing frustum.
double nearPlane = 5;
double farPlane  = 120;

// Viewing angle.
double fovy = 40.0;

// Light properties

GLfloat lightPosition[] = { 1, 0, 1, 0 };
GLfloat lightDiffuse[]  = { 1, 1, 1, 1 };
GLfloat lightAmbient[]  = { 1, 1, 1, 1 };
GLfloat lightSpecular[] = { 1, 1, 1, 1 };

// Floor area
const double FLOOR_SIZE = 40;
const double XMin = -FLOOR_SIZE;
const double XMax = FLOOR_SIZE;
const double YMin = -FLOOR_SIZE;
const double YMax = FLOOR_SIZE;

// Mouse coordinates - used to move light
double mx = 0;
double my = 0;

// GLUT main display
void display ()
{
   glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
   glMatrixMode(GL_MODELVIEW);
   glLoadIdentity();

   // Set view according to controls
   double rot = radians(uiOrientation);
   double ex = uiDistance * sin(rot);
   double ey = -uiDistance * cos(rot);
   gluLookAt(ex, ey, uiHeight,  0, 0, 10,  0, 0, 1);

   // Move light to position set by mouse
   glLightfv(GL_LIGHT0, GL_POSITION, lightPosition);

   // Draw the floor
   setMaterial(WHITE);
   glEnable(GL_TEXTURE_2D);
   glBindTexture(GL_TEXTURE_2D, floorName);
   glBegin(GL_QUADS);
   glTexCoord2f( 0,  0);
   glVertex2f(XMin, YMin);
   glTexCoord2f(10,  0);
   glVertex2f(XMax, YMin);
   glTexCoord2f(10, 10);
   glVertex2f(XMax, YMax);
   glTexCoord2f( 0, 10);
   glVertex2f(XMin, YMax);
   glEnd();
   glDisable(GL_TEXTURE_2D);

   // Draw trampolines
   pTramp1->draw();
   pTramp2->draw();

   // Draw acrobat
   pAcrobat->draw();

   glutSwapBuffers();
}

// Respond to mouse movement
void mouse(int x, int y)
{
   mx = double(x) / width;
   my = double(y) / height;
   glutPostRedisplay();
}

double curTime = GetTickCount(); // milliseconds
double simTime = curTime;

// Use idle functionto update motion of acrobat
void idle ()
{
   double curTime = GetTickCount();
   while (simTime < curTime)
   {
      pAcrobat->actUpdate();
      simTime += 1000 * DT;
   }
   glutSetWindow(mainWindow);
   glutPostRedisplay();
}

// Respond to change of size of main window
void reshapeMainWindow (int newWidth, int newHeight)
{
   width = newWidth;
   height = newHeight;
   glViewport(0, 0, width, height);
   glMatrixMode(GL_PROJECTION);
   glLoadIdentity();
   gluPerspective(fovy, GLfloat(width) / GLfloat(height), nearPlane, farPlane);
}

// Respond to graphic keys
void graphicKeys (unsigned char key, int x, int y)
{
   switch (key)
   {
      case ' ':
         pAcrobat->start();
         break;
      case 27:
         exit(0);
   }
}

// Pointers to GLUI pointers
GLUI_Spinner *ctlDistance;
GLUI_Spinner *ctlHeight;
GLUI_Spinner *ctlRotation;
GLUI_Spinner *ctlSpeed;
GLUI_StaticText *gluiRep;
GLUI_Listbox *gluiLoad;

// Text in file name box
char uiFileName[sizeof(GLUI_String)] = "";

// Current selected file
int fileNum = -1;

// File name store
const int MAX_FILE_NAMES = 256;
string filenames[MAX_FILE_NAMES];
int numfiles = 0;

// Display a message after a save/load action
void message(string text = "OK")
{
   ostringstream os;
   os << "Status: " << text << ends;
   gluiRep->set_text(os.str().c_str());
}

// Display a message if a name has been used previously.
// This does not prevent the user from using the name.
bool checkName(string name)
{
   if (name.size() == 0)
   {
      message("Name has not been specified.");
      return false;
   }
   for (int i = 0; i < numfiles; i++)
   {
      if (name == filenames[i])
      {
         message("Name has already been used.");
         return true;
      }
   }
   message();
   return true;
}

// Options for user interface action callback
enum { RESET, SAVE, LOAD, NEWNAME, PRINT };

// User interface control handlers
void gluiControls(int sel)
{
   switch (sel)
   {
      case RESET:
         // Reset view
         ctlDistance->set_float_val(30);
         ctlHeight->set_float_val(10);
         ctlRotation->set_float_val(0);
         ctlSpeed->set_float_val(3);
         break;

      case NEWNAME:
         // User has changed save file name
         // This is not called as often as it should be!
         checkName(uiFileName);
         break;

      case SAVE:
         // User has requested save state
         if (checkName(uiFileName))
         {
            data.save(uiFileName);
            filenames[numfiles] = uiFileName;
            gluiLoad->add_item(numfiles, uiFileName);
            numfiles++;
            message();
         }
         break;

      case LOAD:
         // User has requested load state
         if (fileNum >= 0)
         {
            data.load(filenames[fileNum]);
            message();
         }
         else
            message("No file selected.");
         break;

      case PRINT:
         cout << endl;
         printView(cout);
         pTramp1->print(cout);
         pTramp2->print(cout);
         pAcrobat->print(cout);
   }
}

// Remove the extension from a file name, construct a new string,
// and return it.
string strip(string name)
{
   string result = name;
   size_t pos = result.find('.');
   return result.substr(0, pos);
}



///////////////////////////////////////// Main

int main (int argc, char **argv)
{
   glutInit(&argc, argv);
   glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGB);
   glutInitWindowSize(width, height);
   glutInitWindowPosition(0, 0);
   mainWindow = glutCreateWindow("Bouncing Ball");

   glutDisplayFunc(display);
   glutMotionFunc(mouse);
   glutReshapeFunc(reshapeMainWindow);
   glutKeyboardFunc(graphicKeys);

   glEnable(GL_DEPTH_TEST);

   glEnable(GL_LIGHTING);
   glEnable(GL_LIGHT0);
   glLightfv(GL_LIGHT0, GL_DIFFUSE, lightDiffuse);
   glLightfv(GL_LIGHT0, GL_AMBIENT, lightAmbient);
   glLightfv(GL_LIGHT0, GL_SPECULAR, lightSpecular);

   glClearColor(0, 0, 0.3, 1);

   GLUI_Master.set_glutIdleFunc(idle);
   glui = GLUI_Master.create_glui("Controls", 0, 0, HEIGHT + 50);
   glui->set_main_gfx_window(mainWindow);

   GLUI_Panel *psl = glui->add_panel("Save/Load");

   glui->add_edittext_to_panel(psl, "File name", GLUI_EDITTEXT_TEXT, uiFileName, NEWNAME, gluiControls);
   glui->add_button_to_panel(psl, "Save", SAVE, gluiControls);

   glui->add_column_to_panel(psl, true);

   gluiLoad = glui->add_listbox_to_panel(psl, "Load", &fileNum, LOAD, gluiControls);
   gluiLoad->add_item(fileNum++, "");
   gluiRep = glui->add_statictext_to_panel(psl, "");

   glui->add_column_to_panel(psl, true);

   glui->add_button_to_panel(psl, "Print", PRINT, gluiControls);

   struct _finddata_t fd;
   long dir = _findfirst(FILE_PATTERN, &fd);

   if (dir > 0)
   {
      while (true)
      {
         cout << "Stored configuration " << fd.name << " found." << endl;
         string fn = strip(string(fd.name));
         filenames[numfiles] = fn;
         gluiLoad->add_item(numfiles, fn.c_str());
         numfiles++;
         if (_findnext(dir, &fd))
            break;
      }
   }
   message();

   GLUI_Panel *view = glui->add_panel("View");

   ctlDistance = glui->add_spinner_to_panel(view, "Distance", GLUI_SPINNER_FLOAT, &uiDistance);
   ctlDistance->set_float_limits(5, 100);

   ctlHeight = glui->add_spinner_to_panel(view, "Height", GLUI_SPINNER_FLOAT, &uiHeight);
   ctlHeight->set_float_limits(5, 50);
   glui->add_column_to_panel(view, true);

   ctlRotation = glui->add_spinner_to_panel(view, "Rotation", GLUI_SPINNER_FLOAT, &uiOrientation);
   ctlRotation->set_float_limits(-180, 180);

   ctlSpeed = glui->add_spinner_to_panel(view, "Speed", GLUI_SPINNER_FLOAT, &uiGrav);
   ctlSpeed->set_int_limits(1, 50);
   glui->add_column_to_panel(view, true);

   glui->add_button_to_panel(view, "Reset", RESET, gluiControls);

   int actionCounter = 0;
   pTramp1 = new Trampoline(1, actionCounter, -5, 0, 5, 180);
   pTramp2 = new Trampoline(2, actionCounter,  5, 0, 5, 0);

   pAcrobat = new Acrobat(actionCounter, Point(-10, 0, 20));

   PixelMap floor("concrete.bmp");
   glGenTextures(1, &floorName);
   floor.setTexture(floorName);

   PixelMap tramp("cotton.bmp");
   glGenTextures(1, &trampName);
   tramp.setTexture(trampName);

   cout << "Starting simulation ..." << endl;

   glutMainLoop();
   return 0;
}
