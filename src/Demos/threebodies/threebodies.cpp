#include <GL/glut.h>
#include <cmath>
#include <iostream>
#include <sstream>
using namespace std;

// General and graphics constants.

const int ESC = 27;				// ASCII Escape code.

const double WIDTH = 600.0;		// Initial window width.
const double HEIGHT = 600.0;	// Initial window height.

const GLfloat GLWHITE[] = { 1.0, 1.0, 1.0, 1.0 };
const GLfloat GLBLACK[] = { 0.0, 0.0, 0.0, 0.0 };
const GLfloat GLBACKGROUND[] = { 0.05, 0.1, 0.1, 1.0 };
const GLfloat ATMOSPHERE[] = { 0.8, 0.0, 0.0, 0.0 };
const GLfloat GLDULL[] = { 0.0 };

// Physics constants.

const double DENSITY = 1.0;		// Commoon density for all objects.
const double GRAV = 2500;		// Gravitational constant.

// Scale for drawing vectors.
const double VECFACTOR = 0.005;

// Amount by which earth radius changes in respose to +/-.
const double RADIUSFACTOR = 1.05;

// Amount by which DT changes in response to </>.
const double SPEEDFACTOR = 1.5;

// System variables.

double width = WIDTH;			// Current window width.
double height = HEIGHT;			// Current window height.
double viewAngle = 45.0;			// Viewing angle w.r.t. orbital plane.
double farAway = 120.0;		// Viewing distance.
bool showVectors = false;		// Toggles vector display.
double DT = 0.0002;				// Time increment for integration.
bool fullScreen = false;

inline double sqr (double x)
{
	return x * x;
}


inline double RadiusToMass (double rad)
{
	return DENSITY * rad * rad * rad;
}

// Sun data.
double SunRadius = 8.0;
double SunMass = RadiusToMass(SunRadius);
GLfloat SunColour[4] = { 1.0, 1.0, 0.9, 1.0 };
GLfloat SunShine[1] = { 0.0 };

// Earth data.
double EarthRadius = 3.0;
double EarthMass = RadiusToMass(EarthRadius);
double EarthOrbit = 50.0;
double EarthVelocity = sqrt(GRAV * SunMass / EarthOrbit);
GLfloat EarthColour[4] = { 0.5, 0.7, 0.7, 1.0 };
GLfloat EarthShine[1] = { 50.0 };

// Moon data.
double MoonRadius = 1.0;
double MoonOrbit = 5.0;
double MoonVelocity = sqrt(GRAV * EarthMass / MoonOrbit);
GLfloat MoonColour[4] = { 0.9, 0.6, 0.4, 1.0 };
GLfloat MoonShine[1] = { 50.0 };

// System declarations.
enum { EARTH, MOON, SUN, NUMBODIES };
class Body;			// Forward declaration.
Body *bods[NUMBODIES];

// Viewpoint
int view = SUN;


/*******************************************/
/*                                         */
/*               class Vector              */
/*                                         */
/*******************************************/

class Vector
{
public:
	Vector (double x = 0.0, double y = 0.0, double z = 0.0);
	const double & operator[] (int i) const;			// Subscript
	double & operator[] (int i);			// Subscript
	Vector & operator= (const Vector & other);	// Assign
	Vector & operator+= (const Vector & other);	// Add and assign
	Vector operator+ (const Vector & other);		// Vector addition
	Vector operator- (const Vector & other);		// Vector difference
	Vector operator- ();					// Vector negation
	Vector operator* (double scalar);		// Vector * Scalar
	Vector operator/ (double scalar);		// Vector / Scalar
	Vector unit ();							// Unit vector in this direction
	double length ();

	GLfloat *glf ();	// Return components as GLfloat's.
	// Warning!  The value returned by glf() must be used
	// immediately, not saved for later use.

	friend ostream & operator<< (ostream & os, Vector v);

private:
	double cpts[3];
};

// Implementation of class vector.

Vector::Vector (double x, double y, double z)
{
	cpts[0] = x;
	cpts[1] = y;
	cpts[2] = z;
}

const double & Vector::operator[] (int i) const
{
	return cpts[i];
}

double & Vector::operator[] (int i)
{
	return cpts[i];
}


Vector & Vector::operator= (const Vector & other)
{
	cpts[0] = other[0];
	cpts[1] = other[1];
	cpts[2] = other[2];
	return *this;
}


Vector & Vector::operator+= (const Vector & other)
{
	cpts[0] += other[0];
	cpts[1] += other[1];
	cpts[2] += other[2];
	return *this;
}


Vector Vector::operator+ (const Vector & other)
{
	return Vector (
		cpts[0] + other[0],
		cpts[1] + other[1],
		cpts[2] + other[2]
		);
}


Vector Vector::operator- (const Vector & other)
{
	return Vector (
		cpts[0] - other[0],
		cpts[1] - other[1],
		cpts[2] - other[2]
		);
}


Vector Vector::operator- ()
{
	return Vector (-cpts[0], -cpts[1], -cpts[2]);
}


Vector Vector::operator* (double scalar)
{
	return Vector (cpts[0] * scalar, cpts[1] * scalar, cpts[2] * scalar);
}


Vector Vector::operator/ (double scalar)
{
	return Vector (cpts[0] / scalar, cpts[1] / scalar, cpts[2] / scalar);
}


double Vector::length ()
{
	return sqrt (sqr(cpts[0]) + sqr(cpts[1]) + sqr(cpts[2]));
}


Vector Vector::unit ()
{
	return (*this) / length();
}


GLfloat * Vector::glf ()
{
	static GLfloat result[4];
	result[0] = cpts[0];
	result[1] = cpts[1];
	result[2] = cpts[2];
	result[3] = 1.0;
	return result;
}


ostream & operator<< (ostream & os, Vector v)
{
	os << '(' <<
		v[0] << ',' <<
		v[1] << ',' <<
		v[2] << ')';
	return os;
}




/*******************************************/
/*                                         */
/*               class Body                */
/*                                         */
/*******************************************/

class Body
{
public:
	Body (Vector iPos, Vector iVel,
  		  double iRadius, GLfloat * icolour,
		  GLfloat * ishine, int ires);
	double getMass ();
	double getRadius ();
	Vector getPos ();
	Vector getVel ();
	Vector getForce ();
	void setPosVel (Vector newPos, Vector newVel);
	void clearForce ();
	void addForce (Vector f);
	void update (const double DT);
	void display (bool illuminated);
	void showForce ();
	void changeRadius (char c);

private:

	// Physics
	double radius;
	double mass;
	Vector pos;
	Vector vel;
	Vector acc;
	Vector force;

	// Graphics
	GLfloat * colour;
	GLfloat * shine;
	int res;
	bool atmosphere;
};


// Implementation of class 'Body'

Body::Body (Vector iPos, Vector iVel,
			double iRadius, GLfloat * icolour,
			GLfloat * ishine, int ires)
{
	pos = iPos;
	vel = iVel;
	acc = Vector();
	force = Vector();
	radius = iRadius;
	colour = icolour;
	shine = ishine;
	res = ires;
	mass = RadiusToMass(radius);
}


double Body::getMass ()
{
	return mass;
}


double Body::getRadius ()
{
	return radius;
}


Vector Body::getPos ()
{
	return pos;
}


Vector Body::getVel ()
{
	return vel;
}


Vector Body::getForce ()
{
	return force;
}


void Body::clearForce ()
// Set forces acting on body to zero.
{
	force = Vector();
}


void Body::addForce (Vector f)
// Add given force to forces acting on body.
{
	force += f;
}


void Body::setPosVel (Vector newPos, Vector newVel)
// Assign a new position and velocity to body.
{
	pos = newPos;
	vel = newVel;
}


void Body::update (const double DT)
{
	acc = force / mass;
	vel += acc * DT;
	pos += vel * DT;
}


void Body::changeRadius (char c)
// Increase or decrease radius and mass of body.
{
	switch (c)
	{
	case '>':
		radius *= RADIUSFACTOR;
		break;
	case '<':
		radius /= RADIUSFACTOR;
		break;
	}
	mass = RadiusToMass(radius);;
	cout << "Earth mass: " << mass << endl;
}


void Body::display (bool illuminated)
// Display a body in graphics window. If the body is illuminated
// (earth and moon), set material properties.
// Otherwise (sun), set colour.
{
	glPushMatrix();

	// Move to position of body.
	glTranslatef(pos[0], pos[1], pos[2]);

	if (illuminated)
	{
		glMaterialfv(GL_FRONT, GL_AMBIENT_AND_DIFFUSE, colour);
		glMaterialfv(GL_FRONT, GL_SPECULAR, GLWHITE);
		glMaterialfv(GL_FRONT, GL_SHININESS, shine);
		glutSolidSphere(radius, res, res);
	}
	else
	{
		glColor4fv(colour);
		glutSolidSphere(radius, res, res);
	}

	glPopMatrix();
}


void Body::showForce ()
// Show force vector acting on body as a white line.
{
	glPushMatrix();
	glTranslatef(pos[0], pos[1], pos[2]);
	glBegin(GL_LINES);
		glVertex3f(0.0, 0.0, 0.0);
		glVertex3f(VECFACTOR * force[0], VECFACTOR * force[1], VECFACTOR * force[2]);
	glEnd();
	glPopMatrix();
}



void update(double DT)
// Move bodies according to the law of gravitation
// using first-order integration.
{
	int a, b;

	// Set forces on bodies to zero.
	for (a = 0; a < NUMBODIES; a++)
		bods[a]->clearForce();

	// For each body 'a', and the forces of all
	// other bodies 'b' acting on it.
	for (a = 0; a < NUMBODIES; a++)
		for (b = a + 1; b < NUMBODIES; b++)
		{
			// Compute direction of force as a unit vector.
			// This could be done in one line using a proper compiler,
			// but with VC++ we have to use two lines.
			Vector sep = bods[a]->getPos() - bods[b]->getPos();
			Vector dir = sep.unit();

			// Compute magnitude of force between bodies using Newton's law.
			double dist = sep.length();
			double mag = (GRAV * bods[a]->getMass() * bods[b]->getMass()) / (dist * dist);

			// Compute the actual force and apply it to the bodies.
			Vector force = dir * mag;
			bods[a]->addForce(-force);
			bods[b]->addForce(force);
		}

	// Using the computed forces, update the motion of each body.
	for (a = 0; a < NUMBODIES; a++)
		bods[a]->update(DT);
}

int frameCounter = 0;

void timer(int val)
{
   ostringstream os;
   os << frameCounter << " f/s";
   glutSetWindowTitle(os.str().c_str());
   frameCounter = 0;
   glutTimerFunc(1000, timer, 0);
}

void display (void)
// Display the current position of each body.
{
   ++frameCounter;
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();

	// Choose viewing transformation according to viewpoint.
	double rad;
	Vector pos, eye;
	switch (view)
	{
	case SUN:
		glTranslatef(0.0, 0.0, -farAway);
		glRotatef(viewAngle, 1.0, 0.0, 0.0);
		break;
	case EARTH:
		rad = bods[EARTH]->getRadius();
		pos = bods[EARTH]->getPos() + Vector(0.0, 1.5 * rad, 0.0);
		eye = bods[EARTH]->getPos() + Vector(rad, rad, 0.0);
		gluLookAt(eye[0], eye[1], eye[2], pos[0], pos[1], pos[2], 0.0, 1.0, 0.0);
		break;
	case MOON:
		rad = bods[MOON]->getRadius();
		pos = bods[MOON]->getPos() + Vector(0.0, 1.5 * rad, 0.0);
		eye = bods[MOON]->getPos() + Vector(rad, rad, 0.0);
		gluLookAt(eye[0], eye[1], eye[2], pos[0], pos[1], pos[2], 0.0, 1.0, 0.0);
		break;
	}


	// Display earth and moon with lighting.
	glEnable(GL_LIGHTING);
	glEnable(GL_LIGHT0);
	glLightfv(GL_LIGHT0, GL_POSITION, bods[SUN]->getPos().glf());
	glLightfv(GL_LIGHT0, GL_AMBIENT, GLBACKGROUND);
	bods[EARTH]->display(true);
	bods[MOON]->display(true);
	glDisable(GL_LIGHTING);

	// Display sun without lighting.
	bods[SUN]->display(false);

	// Display forces if requested,.
	if (showVectors)
	{
		bods[EARTH]->showForce();
		bods[MOON]->showForce();
	}

	glutSwapBuffers();
	glFlush();
}


void idle ()
{
	update(DT);
	glutPostRedisplay();
}


void reshape (int w, int h)
{
	glViewport(0, 0, w, h);
	width = w;
	height = h;
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	gluPerspective(70.0, width / height, 1.0, 500.0);
	glutPostRedisplay();
}


// Define initial position and velocity of each body.
// The sun's velocity is computed by initBodes().
Vector EarthInitialPos = Vector(EarthOrbit);
Vector EarthInitialVel = Vector(0.0, EarthVelocity);
Vector MoonInitialPos = Vector(EarthOrbit + MoonOrbit);
Vector MoonInitialVel = Vector(0.0, EarthVelocity + 0.8 * MoonVelocity, 0.6 * MoonVelocity);
Vector SunInitialPos = Vector();
Vector SunInitialVel = Vector();


void initBodies ()
{

	// Create the earth and the moon.
	bods[EARTH] = new Body
	(
		EarthInitialPos,
		EarthInitialVel,
		EarthRadius,
		EarthColour,
		EarthShine,
		20
	);

	bods[MOON] = new Body
	(
		MoonInitialPos,
		MoonInitialVel,
		MoonRadius,
		MoonColour,
		MoonShine,
		20
	);

	// Compute momentum of earth and moon.
	Vector momentum;
	for (int b = 0; b < SUN; b++)
		momentum += bods[b]->getVel() * bods[b]->getMass();
	SunInitialVel = - momentum / RadiusToMass(SunRadius);

	// Construct sun so that total momentum of system is zero.
	// This prevents the picture drifting off the screen.
	bods[SUN] = new Body
	(
		SunInitialPos,
		SunInitialVel,
		SunRadius,
		SunColour,
		SunShine,
		30
	);
}


void intro ()
{
	cout << "This program demonstrates a sun, earth, and moon system" << endl;
	cout << "using artificial units for mass, length, and time.  You" << endl;
	cout << "can change the mass of the earth and thus the stability" << endl;
	cout << "of the moon's orbit.  With the initial settings, the moon" << endl;
	cout << "escapes from the earth after a few (simulated) years." << endl << endl;
}


void help ()
{
	cout << "The graphics window shows the Sun, the Earth, and the Moon." << endl;
	cout << "With graphics window current, the following keys have the effect shown." << endl;
	cout << "+    Speed up the simulation." << endl;
	cout << "-    Slow down the simulation." << endl;
	cout << ">    Decrease earth's radius and mass." << endl;
	cout << "<    Increase earth's radius and mass." << endl;
	cout << "d    Move viewpoint downwards." << endl;
	cout << "e    View from earth." << endl;
   cout << "f    Toggle full screen view." << endl;
	cout << "h    Redisplay these instructions." << endl;
	cout << "m    View from moon." << endl;
	cout << "o    View from outer space." << endl;
	cout << "r    Reset bodies to initial positions." << endl;
	cout << "s    View system from side." << endl;
	cout << "t    View system from top." << endl;
	cout << "u    Move viewpoint upwards." << endl;
	cout << "v    Show (hide) force vectors for earth and moon." << endl << endl;
	cout << "Time increment = " << DT << endl;
	cout << "View angle = " << viewAngle << endl;
}


void keyboard (unsigned char key, int x, int y)
{
	switch (key)
	{

	case '<':
	case '>':
		bods[0]->changeRadius(key);
		break;

	case '-':
		DT /= SPEEDFACTOR;
		cout << "Time increment = " << DT << endl;
		break;

	case '+':
		DT *= SPEEDFACTOR;
		cout << "Time increment = " << DT << endl;
		break;

	case 'd':
	case 'D':
		viewAngle -= 10.0;
		cout << "View angle = " << viewAngle << endl;
		break;

	case 'e':
	case 'E':
		view = EARTH;
		break;

   case 'f':
   case 'F':
      {
         if (fullScreen)
         {
            glutReshapeWindow(int(width), int(height));
            fullScreen = false;
            cout << "Normal mode" << endl;
         }
         else
         {
            glutFullScreen();
            fullScreen = true;
            cout << "Full screen mode" << endl;
         }
      }
      break;

	case 'h':
	case 'H':
		help();
		break;

	case 'm':
	case 'M':
		view = MOON;
		break;

	case 'o':
	case 'O':
		view = SUN;
		viewAngle = 45.0;
		break;

	case 'r':
	case 'R':
		bods[EARTH]->setPosVel(EarthInitialPos, EarthInitialVel);
		bods[MOON]->setPosVel(MoonInitialPos, MoonInitialVel);
		bods[SUN]->setPosVel(SunInitialPos, SunInitialVel);
		break;

	case 's':
	case 'S':
		viewAngle = 90.0;
		cout << "View angle = " << viewAngle << endl;
		break;

	case 't':
	case 'T':
		viewAngle = 0.0;
		cout << "View angle = " << viewAngle << endl;
		break;

	case 'u':
	case 'U':
		viewAngle += 10.0;
		cout << "View angle = " << viewAngle << endl;
		break;

	case 'v':
	case 'V':
		showVectors = ! showVectors;
		break;

	case ESC:
		exit(0);

	default:
		break;
	}

   int state = glutGetModifiers();
   if (state)
      cout << "Keyboard state: " << state << endl;
}


void visible (int state)
{
   switch (state)
   {
   case GLUT_VISIBLE:
      cout << "Window is visible" << endl;
      break;
   case GLUT_NOT_VISIBLE:
      cout << "Window is not visible" << endl;
      break;
   }
}


int main (int argc, char **argv)
{
	glutInit(&argc, argv);
	glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGB | GLUT_DEPTH);
	glutInitWindowSize(int(WIDTH), int(HEIGHT));
	glutCreateWindow("The Three Bodies");
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	gluPerspective(75.0, 1.0, 1.0, 200.0);
	glEnable(GL_DEPTH_TEST);
	glShadeModel(GL_SMOOTH);
	glutDisplayFunc(display);
	glutReshapeFunc(reshape);
	glutKeyboardFunc(keyboard);
   glutVisibilityFunc(visible);
	glutIdleFunc(idle);
	glutTimerFunc(1000, timer, 0);
	initBodies();
	intro();
	help();
	glutMainLoop();
	return 0;
}
