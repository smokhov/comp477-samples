#include <GL/cugl.h>
#include <GL/glut.h>
#include <cmath>
#include <iostream>
#include <windows.h>

using namespace std;
using namespace cugl;

const int ESC = 27;           // ASCII Escape code.

const double WIDTH = 800;     // Initial window width.
const double HEIGHT = 800; // Initial window height.

double width = WIDTH;         // Current window width.
double height = HEIGHT;       // Current window height.
double aspect = WIDTH / HEIGHT; // Current aspect ratio

const double GRAV_CONST = 2000;
const double SOLAR_MASS = 100;

const double GM = GRAV_CONST * SOLAR_MASS;
const double DT = 0.01; // seconds
double time = 0;

bool planetFrame = false;

class Sun
{
public:
   Sun(double mass, double rad) : mass(mass), rad(rad) {}
   double getRad()
   {
      return rad;
   }
   void display()
   {
      glColor3f(1, 1, 1);
      glutSolidSphere(10, 20, 20);
   }
private:
   double mass;
   double rad;
};

class Planet
{
public:
   Planet(double mass, double rad, double orbitRadius, double phaseInit) :
      mass(mass),
      rad(rad),
      orbitRadius(orbitRadius),
      phaseInit(phaseInit),
      phase(phaseInit),
      omega(sqrt(GM / (orbitRadius * orbitRadius * orbitRadius))),
      escVel(sqrt((2 * GRAV_CONST * mass) / rad))
   {
      update(time);
   }
   void reset()
   {
      phase = phaseInit;
      update(time);
   }
   double getRad()
   {
      return rad;
   }
   double getEscVel()
   {
      return escVel;
   }
   double getAngle()
   {
      return phase + omega * time;
   }
   Vector getPos()
   {
      return pos;
   }
   Vector getVel()
   {
      return vel;
   }
   double getMass()
   {
      return mass;
   }
   void update(double time);
   void display();
   friend ostream & operator<<(ostream & os, Planet *p);

private:
   double mass;
   double rad;
   double orbitRadius;
   double phaseInit;
   double phase;
   double omega;
   double escVel;
   Vector pos;
   Vector vel;
};

ostream & operator<<(ostream & os, Planet *pp)
{
   return os <<
          "Mass: " << pp->mass <<
          "  Radius: " << pp->rad <<
          "  Esc vel: " << pp->escVel <<
          "  Vel: " << pp->vel <<
          "  Pos: " << pp->pos <<
          endl;
}

void Planet::update(double time)
{
   pos = Vector(orbitRadius * cos(phase + omega * time), orbitRadius * sin(phase + omega * time), 0);
   vel = Vector(- orbitRadius * omega * sin(phase + omega * time), orbitRadius * omega * cos(phase + omega * time), 0);
}

bool drawOrbit = false;

void Planet::display()
{
   glPushMatrix();
   pos.translate();
   glColor3f(1, 1, 0.8);
   glutSolidSphere(rad, 10, 10);
   glPopMatrix();

   if (drawOrbit)
   {
      glColor3f(0, 1, 0);
      glBegin(GL_LINE_LOOP);
      for (int i = 0; i < 100; ++i)
         glVertex3f(orbitRadius * cos(2 * PI * i / 100.0), orbitRadius * sin(2 * PI * i / 100.0), 0);
      glEnd();
   }
}

class Satellite
{
public:
   Satellite(Vector posInit, Vector velInit) :
      size(3),
      posInit(posInit),
      pos(posInit),
      velInit(velInit),
      vel(velInit),
      showAcc(false)
   { }
   void reset()
   {
      pos = posInit;
      vel = velInit;
   }
   void display();
   void update();
   void toggleShowAcc()
   {
      showAcc = !showAcc;
   }

private:
   double size;
   Vector posInit;
   Vector pos;
   Vector velInit;
   Vector vel;
   Vector acc;
   bool showAcc;
};

void Satellite::display()
{
   glPushMatrix();
   pos.translate();
   glColor3f(0, 1, 1);
   glutSolidCube(size);
   if (showAcc)
   {
      glColor3f(1, 1, 1);
      (100 * acc).draw(Point());
   }
   glPopMatrix();
}

Sun *pSun = new Sun(SOLAR_MASS, 10);

int np = 0;
Planet *planets[10];
Satellite *ps;

void Satellite::update()
{
   // Acceleration due to sun
   acc = - GM * pos.unit() / pos.norm();

   // Acceleration due to planets
   for (int p = 0; p < np; ++p)
   {
      Vector disp = planets[p]->getPos() - pos;
      acc += GRAV_CONST * planets[p]->getMass() * disp.unit() / disp.norm();
   }

   // New velocity and position
   vel += acc * DT;
   pos += vel * DT;
}

bool closeUp = false;

void display (void)
// Display the current position of each body.
{
   glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
   if (closeUp)
   {
      glMatrixMode(GL_PROJECTION);
      glLoadIdentity();
      gluPerspective(80, aspect, 10, 1000);
      glMatrixMode(GL_MODELVIEW);
      glLoadIdentity();
      gluLookAt(-450, 320, 50,   0, 0, 0,  0, 0, 1);
   }
   else
   {
      glMatrixMode(GL_PROJECTION);
      glLoadIdentity();
      glOrtho(-750, 750, -750 / aspect, 750 / aspect, -100, 100);
      glMatrixMode(GL_MODELVIEW);
      glLoadIdentity();
      glTranslatef(0, 0, -100);
   }

   if (planetFrame)
         (-planets[0]->getPos()).translate();

   glColor3f(1, 1, 1);
   pSun->display();

   for (int p = 0; p < np; ++p)
   {
      planets[p]->update(time);
      planets[p]->display();
   }
   ps->display();

   glutSwapBuffers();
   glFlush();
}

double curTime = GetTickCount(); // milliseconds
double simTime = curTime;

void idle ()
{
   curTime = GetTickCount();
   while (simTime < curTime)
   {
      ps->update();
      time += DT;
      simTime += 1000 * DT;
   }
   glutPostRedisplay();
}


void reshape (int w, int h)
{
   glViewport(0, 0, w, h);
   width = w;
   height = h;
   aspect = width / height;
   glutPostRedisplay();
}

void restart()
{
   time = 0;
   for (int p = 0; p < np; ++p)
      planets[p]->reset();
   ps->reset();
}

void keyboard (unsigned char key, int x, int y)
{
   switch (key)
   {
      case ' ':
         cout << planets[0];
         break;

      case 'a':
         ps->toggleShowAcc();
         break;

      case 'c':
         closeUp = ! closeUp;
         restart();
         break;

      case 'f':
         glutFullScreen();
         break;

      case 'p':
         planetFrame = ! planetFrame;
         break;

      case 'o':
         drawOrbit = ! drawOrbit;
         break;

      case 'r':
         restart();
         break;

      case ESC:
         exit(0);

      default:
         break;
   }
}

int main (int argc, char **argv)
{
   cout <<
        "Sling Shot\n\n"
        "  a  toggle satellite acceleration vector\n"
        "  f  full screen\n"
        "  o  toggle planet's orbit\n"
        "  p  toggle sun/planet frame\n"
        "  r  restart\n"
        " ESC exit\n";

   glutInit(&argc, argv);
   glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGB | GLUT_DEPTH);
   glutInitWindowSize(int(WIDTH), int(HEIGHT));
   glutCreateWindow("Sling Shot");
   glMatrixMode(GL_PROJECTION);
   glLoadIdentity();
   gluPerspective(75.0, 1.0, 1.0, 200.0);
   glEnable(GL_DEPTH_TEST);
   glShadeModel(GL_SMOOTH);
   glutDisplayFunc(display);
   glutReshapeFunc(reshape);
   glutKeyboardFunc(keyboard);
   glutIdleFunc(idle);

   double prad = 400;
   planets[np++] = new Planet(SOLAR_MASS/100, 5, prad, 1.22);

   double srad = 200;
   double svel = sqrt((2 * prad * GM) / (srad * (srad + prad)));
   ps = new Satellite(Vector(srad, 0, 0), Vector(0, svel, 0));
   restart();

   glutMainLoop();
   return 0;
}
