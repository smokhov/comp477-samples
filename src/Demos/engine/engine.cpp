#include <GL/cugl.h>
#include <GL/glui.h>
#include <GL/glut.h>
#include <GL/tube.h>
#include <cmath>
#include <iostream>
#include <sstream>
#include <iomanip>
#include <ctime>
#include <cstdlib>

using namespace std;
using namespace cugl;

//#include "framecounter.h"

// Extreme coordinates of engine hall.
const GLfloat xMin = -30;
const GLfloat xMax = 60;
const GLfloat yMin = -16;
const GLfloat yMax = 20;
const GLfloat zMin = -30;
const GLfloat zMax = 30;

const GLfloat xMid = 45;
const GLfloat yMid = 0;
const GLfloat zMid = 10;

// Main window key
int mainWindow = 0;

// GLUI window
GLUI *glui;
GLUI_EditText *snapFile;

// GLUI-controlled variables
int light0 = 1;
int light1 = 1;
int light2 = 1;
int showWalls = 1;
int showSupports = 1;
int filled = 1;
int transparent = 0;
int localViewer = 0;
int showAxes = 0;

enum GluiVals
{
   LIGHTS,
   LOCAL_VIEWER,
   AXES,
   FILL_POLYGONS,
   CAM_UP,
   CAM_DOWN,
   CAM_LEFT,
   CAM_RIGHT,
   CAM_FORWARD,
   CAM_BACKWARD,
   CAM_TILTUP,
   CAM_TILTDOWN,
   CAM_PANLEFT,
   CAM_PANRIGHT,
   CAM_ZOOMIN,
   CAM_ZOOMOUT,
   CAM_RESET,
   CAM_SHOW,
   VIEW_00,
   VIEW_01,
   VIEW_02,
   VIEW_03,
   VIEW_04,
   VIEW_05,
   VIEW_06,
   VIEW_07,
   VIEW_08,
   VIEW_09,
   FASTER,
   SLOWER,
   SNAPSHOT
};

// Camera movement control
Camera camera;

// Array of eye positions
Point eyePositions[10] =
{
   Point(   10,     0, zMin ),
   Point(   10,     0, zMin ),
   Point(   30,   -15, zMax ),
   Point(  -20,     5,   10 ),
   Point( xMin,     5, zMax ),
   Point(   20,  yMax,  -10 ),
   Point(   20,  yMax,  -10 ),
   Point(    0,  yMax,   20 ),
   Point( xMax,  yMin,    5 ),
   Point( xMax,   -10, zMax )
};

// Array of viewpoints in model
Point modelPoints[10] =
{
   Point( 15, 0,  0 ),
   Point( 15, 0,  0 ),
   Point( 15, 0,  0 ),
   Point( 15, 5,  5 ),
   Point( 15, 0,  0 ),
   Point( 35, 0,  5 ),
   Point( 10, 0,  5 ),
   Point(  0, 0, 10 ),
   Point( 30, 0,  0 ),
   Point( 20, 0,  0 )
};

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
GLfloat nearPlane =   5;
GLfloat farPlane  = 120;
//GLfloat distance =  -40;

// Viewing angle.
GLfloat fovy = 40;

// Variables.
GLfloat alpha = 0;                                  // Set by idle function.
GLfloat phaselag = 90;

//FrameCounter fc;

double xAxisPos = 0;
double yAxisPos = 0;
double zAxisPos = 0;

const GLfloat xDelta = 1;
GLfloat xShift = -10;

GLUquadricObj *wheel;
const GLint slices = 20;
const GLint loops = 10;

// Coordinates of corners of hall.
const GLfloat corners[8][3] =
{
   // Front (near viewer)
   { xMin, yMin, zMax },
   { xMax, yMin, zMax },
   { xMax, yMax, zMax },
   { xMin, yMax, zMax },

   // Back
   { xMin, yMin, zMin },
   { xMax, yMin, zMin },
   { xMax, yMax, zMin },
   { xMin, yMax, zMin },
};

// Declarations for girders

const gleDouble grScale = 0.15;

gleDouble grContour[17][2] =
{
   {  0 * grScale,  0 * grScale },
   {  5 * grScale,  0 * grScale },
   {  5 * grScale,  2 * grScale },
   {  1 * grScale,  2 * grScale },
   {  1 * grScale,  8 * grScale },
   {  1 * grScale, 14 * grScale },
   {  5 * grScale, 14 * grScale },
   {  5 * grScale, 16 * grScale },
   {  0 * grScale, 16 * grScale },
   { -5 * grScale, 16 * grScale },
   { -5 * grScale, 14 * grScale },
   { -1 * grScale, 14 * grScale },
   { -1 * grScale,  8 * grScale },
   { -1 * grScale,  2 * grScale },
   { -5 * grScale,  2 * grScale },
   { -5 * grScale,  0 * grScale },
   {  0 * grScale,  0 * grScale }
};

const gleDouble r = sqrt(2) / 2;

gleDouble grNormals[17][2] =
{
   {  0, -1 },
   {  1, -1 },
   {  1,  1 },
   {  1,  1 },
   {  1,  0 },
   {  1, -1 },
   {  1, -1 },
   {  1,  1 },
   {  0,  1 },
   { -1,  1 },
   { -1, -1 },
   { -1, -1 },
   { -1,  0 },
   { -1,  1 },
   { -1,  1 },
   { -1, -1 },
   {  0, -1 }
};

gleDouble up[3] = { 0, 1, 0 };

const GLfloat leOffset = 3;
const GLfloat leWidth = 1;

// Main crank
const GLfloat bclInner = 0;          // Big end
const GLfloat bclOuter = 2;
const GLfloat bcsInner = 1;          // Small end
const GLfloat bcsOuter = 1.5;
const GLfloat bcLength = 20;
const GLfloat bcLinkRad = leWidth / 2;

// Valve crank
const GLfloat seRadius = 2;          // Eccentric
const GLfloat seOffset = 1;
const GLfloat seWidth = 1;

const GLfloat sclInner = seRadius;   // Big end
const GLfloat sclOuter = sclInner + 0.5;
const GLfloat scsInner = 0.75;       // Small end
const GLfloat scsOuter = 1;
const GLfloat scLength = 20;
const GLfloat scLinkRad = 0.2f;

// Distance between main axis and valve slider
const GLfloat vHeight = 6;

// Flywheel
const GLfloat fwHubRad = 3;
const GLfloat fwInRad = 12;
const GLfloat fwOutRad = fwInRad + 1;
const GLfloat fwSpokeRad = 0.5;
const GLfloat fwWidth = 2;
const GLfloat fwNumSpokes = 12;

// Main piston
const GLfloat mpThick = 0.7f;
const GLfloat mplLen = 2 * bcsOuter + mpThick;
const GLfloat mpDisp = bcsOuter;
const GLfloat mpGap = leWidth;
const GLfloat mplWidth = 2 * bcsInner;
const GLfloat mprRad = 0.5;
const GLfloat mprLen = 10;
const GLfloat mppRad = 2;
const GLfloat mppLen = 1;

// Main cylinder
const GLfloat mcThick = 0.5;
const GLfloat mcInRad = mppRad;
const GLfloat mcOutRad = mcInRad + mcThick;
const GLfloat mcClearance = 1; // Slack space beyond piston motion
const GLfloat mcLen = 2 * leOffset + mppLen + 2 * mcClearance; // 10;
const GLfloat mcEnd = 0.5;
const GLfloat mcStart = bcLength + (mplLen + mprLen - mpDisp) - leOffset - mcClearance - mcEnd;
const GLfloat mcSteam1 = mcStart + mcEnd + mcClearance / 2;
const GLfloat mcSteam2 = mcStart + mcEnd + mcLen - mcClearance / 2;

// Valve piston
const GLfloat spThick = 0.5;
const GLfloat splLen = 2 * scsOuter + spThick;
const GLfloat spDisp = scsOuter;
const GLfloat spGap = seWidth;
const GLfloat splWidth = 2 * scsInner;
const GLfloat sprRad = 0.2f;
const GLfloat sprLen = 10;
const GLfloat sppRad = 1;
const GLfloat sppLen = 0.4f;

// Valve cylinder
const GLfloat vcThick = 0.25;
const GLfloat vcEnd = 0.25;
const GLfloat vcClearance = 1;
const GLfloat vcInRad = sppRad;
const GLfloat vcOutRad = vcInRad + vcThick;
const GLfloat vcStart = sqrt(sqr(seOffset - scLength) - sqr(vHeight)) + (splLen + sprLen - spDisp) - vcClearance - vcEnd;
const GLfloat vcStroke =   sqrt(sqr(seOffset + scLength) - sqr(vHeight))
                           - sqrt(sqr(seOffset - scLength) - sqr(vHeight));
const GLfloat spr2Len = sprLen + vcStroke + spThick;
const GLfloat vcLen = 2 * seOffset + (spr2Len - sprLen + sppLen) + 2 * vcClearance;
const GLfloat vcCentre = vcStart + vcEnd + vcLen / 2;
const GLfloat vcSteamOut1 = vcCentre - (vcStroke / 2 + sppLen);
const GLfloat vcSteamOut2 = vcCentre + (vcStroke / 2 + sppLen);
const GLfloat vcSteamIn1 = vcStart + vcEnd + vcClearance / 2;
const GLfloat vcSteamIn2 = vcStart + vcEnd + vcLen - vcClearance / 2;

// Axle
const GLfloat rAxle = 1;
const GLfloat l1Axle = 3;
const GLfloat l2Axle = 1;
const GLfloat l3Axle = 1;
const GLfloat l4Axle = 3;

// Crank
const GLfloat wCrank = 3;
const GLfloat lCrank = 6;
const GLfloat hCrank = 0.5;
const GLfloat oCrank = wCrank / 2;   // Offset from centre

// Z coordinates of various parts
const GLfloat mpZ = l1Axle + fwWidth + l2Axle + hCrank;  // Main piston
const GLfloat mcZ = mpZ + leWidth / 2;                // Main cylinder
const GLfloat spZ = l1Axle + fwWidth + l2Axle + hCrank + leWidth + hCrank + l3Axle;  // Small piston
const GLfloat vcZ = spZ + seWidth / 2;   // Valve cylinder

// Tube from valve cylinder to main cylinder
const gleDouble H = 0.2;
const gleDouble exLength = 10;
const gleDouble intake1 = 5;
const gleDouble intake2 = 5;
const gleDouble intake3 = 30; // 10;
gleDouble tube1[6][3];
gleDouble tube2[6][3];
gleDouble tube3[4][3];
gleDouble tube4[6][3];
gleDouble tube5[6][3];

// Support girder paths
gleDouble grPathRear[8][3] =
{
   { -21,          yMin, 0 },
   { -20,          yMin, 0 },
   { -10,          yMin, 0 },
   {  -5, - 8 * grScale, 0 },
   {  35, - 8 * grScale, 0 },
   {  40,          yMin, 0 },
   {  50,          yMin, 0 },
   {  51,          yMin, 0 }
};

gleDouble grPathFront[8][3] =
{
   { -21,          yMin, spZ + l4Axle + 1 },
   { -20,          yMin, spZ + l4Axle + 1 },
   { -10,          yMin, spZ + l4Axle + 1 },
   {  -5, - 8 * grScale, spZ + l4Axle + 1 },
   {  35, - 8 * grScale, spZ + l4Axle + 1 },
   {  40,          yMin, spZ + l4Axle + 1 },
   {  50,          yMin, spZ + l4Axle + 1 },
   {  51,          yMin, spZ + l4Axle + 1 }
};

GLfloat inspectionLightPos[] = { 0, 5, 10, 1 };
GLfloat light1Pos[] = { -5, 15, 10 };
GLfloat light2Pos[] = { 35, 15, 10 };
GLfloat zero[] = { 0, 0, 0, 1 };

GLfloat none[] = { 0, 0, 0, 1 };
GLfloat white[] = { 1, 1, 1, 1 };
GLfloat dull[] = { 0.3f, 0.3f, 0.3f, 1 };

enum { BRICK, CONCRETE, numTex };
GLuint texNames[numTex];
const int txbScale = 5;

PixelMap texBricks;
PixelMap texConcrete;

const GLfloat DSINC = 0.2;     // Delta for desired speed
const GLfloat ASINC = 0.001 * DSINC; // Delta for actual speed
GLfloat desiredSpeed = 1;
GLfloat actualSpeed = 0;

void buildModel ()
{
   wheel = gluNewQuadric();
   gluQuadricDrawStyle(wheel, GLU_FILL);
   gluQuadricOrientation(wheel, GLU_OUTSIDE);
   gluQuadricNormals(wheel, GLU_SMOOTH);

   tube1[0][0] = vcSteamOut1;
   tube1[0][1] = vHeight;
   tube1[0][2] = vcZ - vcOutRad + H;
   tube1[1][0] = vcSteamOut1;
   tube1[1][1] = vHeight;
   tube1[1][2] = vcZ - vcOutRad;
   tube1[2][0] = vcSteamOut1;
   tube1[2][1] = vHeight;
   tube1[2][2] = mcZ;
   tube1[3][0] = mcSteam1;
   tube1[3][1] = mcOutRad + 2;
   tube1[3][2] = mcZ;
   tube1[4][0] = mcSteam1;
   tube1[4][1] = mcOutRad;
   tube1[4][2] = mcZ;
   tube1[5][0] = mcSteam1;
   tube1[5][1] = mcOutRad - H;
   tube1[5][2] = mcZ;

   // Tube from valve cylinder to main cylinder
   tube2[0][0] = vcSteamOut2;
   tube2[0][1] = vHeight;
   tube2[0][2] = vcZ - vcOutRad + H;
   tube2[1][0] = vcSteamOut2;
   tube2[1][1] = vHeight;
   tube2[1][2] = vcZ - vcOutRad;
   tube2[2][0] = vcSteamOut2;
   tube2[2][1] = vHeight;
   tube2[2][2] = mcZ;
   tube2[3][0] = mcSteam2;
   tube2[3][1] = mcOutRad + 2;
   tube2[3][2] = mcZ;
   tube2[4][0] = mcSteam2;
   tube2[4][1] = mcOutRad;
   tube2[4][2] = mcZ;
   tube2[5][0] = mcSteam2;
   tube2[5][1] = mcOutRad - H;
   tube2[5][2] = mcZ;

   // Exhaust tube
   tube3[0][0] = vcCentre;
   tube3[0][1] = vHeight + vcOutRad - H;
   tube3[0][2] = vcZ;
   tube3[1][0] = vcCentre;
   tube3[1][1] = vHeight + vcOutRad;
   tube3[1][2] = vcZ;
   tube3[2][0] = vcCentre;
   tube3[2][1] = yMax;
   tube3[2][2] = vcZ;
   tube3[3][0] = vcCentre;
   tube3[3][1] = yMax + H;
   tube3[3][2] = vcZ;

   // Steam intakes
   tube4[0][0] = vcSteamIn1;
   tube4[0][1] = vHeight;
   tube4[0][2] = vcZ - (vcOutRad - H);
   tube4[1][0] = vcSteamIn1;
   tube4[1][1] = vHeight;
   tube4[1][2] = vcZ - (vcOutRad);
   tube4[2][0] = vcSteamIn1;
   tube4[2][1] = vHeight;
   tube4[2][2] = vcZ - (vcOutRad + intake1);
   tube4[3][0] = vcCentre;
   tube4[3][1] = vHeight;
   tube4[3][2] = vcZ - (vcOutRad + intake1 + intake2);
   tube4[4][0] = vcCentre;
   tube4[4][1] = vHeight;
   tube4[4][2] = vcZ - (vcOutRad + intake1 + intake2 + intake3);
   tube4[5][0] = vcCentre;
   tube4[5][1] = vHeight;
   tube4[5][2] = vcZ - (vcOutRad + intake1 + intake2 + intake3 + H);

   // It is not necessary to draw the last segment of this tube.
   tube5[0][0] = vcSteamIn2;
   tube5[0][1] = vHeight;
   tube5[0][2] = vcZ - (vcOutRad - H);
   tube5[1][0] = vcSteamIn2;
   tube5[1][1] = vHeight;
   tube5[1][2] = vcZ - (vcOutRad);
   tube5[2][0] = vcSteamIn2;
   tube5[2][1] = vHeight;
   tube5[2][2] = vcZ - (vcOutRad + intake1);
   tube5[3][0] = vcCentre;
   tube5[3][1] = vHeight;
   tube5[3][2] = vcZ - (vcOutRad + intake1 + intake2);
   tube5[4][0] = vcCentre;
   tube5[4][1] = vHeight;
   tube5[4][2] = vcZ - (vcOutRad + intake1 + intake2 + intake3);
   tube5[5][0] = vcCentre;
   tube5[5][1] = vHeight;
   tube5[5][2] = vcZ - (vcOutRad + intake1 + intake2 + intake3 + H);

   // Lighting
   glEnable(GL_LIGHTING);

   glEnable(GL_LIGHT0);
   glLightfv(GL_LIGHT0, GL_AMBIENT, dull);
   glLightfv(GL_LIGHT0, GL_DIFFUSE, white);
   glLightfv(GL_LIGHT0, GL_SPECULAR, white);

   glEnable(GL_LIGHT1);
   glLightfv(GL_LIGHT1, GL_AMBIENT, dull);
   glLightfv(GL_LIGHT1, GL_DIFFUSE, white);
   glLightfv(GL_LIGHT1, GL_SPECULAR, white);

   glEnable(GL_LIGHT2);
   glLightfv(GL_LIGHT2, GL_AMBIENT, dull);
   glLightfv(GL_LIGHT2, GL_DIFFUSE, white);
   glLightfv(GL_LIGHT2, GL_SPECULAR, white);

   // Transparency
   glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

   // Textures
   CUGLErrorType err;
   texBricks.read("bricks.bmp");
   err = getError();
   if (err != NO_ERROR)
      cout << "Reading texture failed: " << getErrorString(err) << ".\n";

   texConcrete.read("concrete.bmp");
   err = getError();
   if (err != NO_ERROR)
      cout << "Reading texture failed: " << getErrorString(err) << ".\n";

   glGenTextures(numTex, texNames);
   texBricks.setTexture(texNames[BRICK]);
   texConcrete.setTexture(texNames[CONCRETE]);
}

void axle(GLfloat len)
{
   setMaterial(CHROME);
   gluCylinder(wheel, rAxle, rAxle, len, slices, loops);
}

void disk(GLUquadricObj *obj, GLfloat fwRadius, GLfloat fwWidth, int slices = 20)
{
   glPushMatrix();
   setMaterial(BRASS);
   gluCylinder(wheel, fwRadius, fwRadius, fwWidth, slices, loops);
   gluDisk(wheel, 0, fwRadius, slices, loops);
   glTranslatef(0, 0, fwWidth);
   gluDisk(wheel, 0, fwRadius, slices, loops);
   glPopMatrix();
}

void ring(GLUquadricObj *obj, GLfloat inRad, GLfloat bclOuter, GLfloat height)
{
   glPushMatrix();
   setMaterial(BRONZE);
   gluCylinder(obj, inRad, inRad, height, 50, loops);
   gluCylinder(obj, bclOuter, bclOuter, height, 50, loops);
   gluDisk(obj, inRad, bclOuter, 50, loops);
   glTranslatef(0, 0, height);
   gluDisk(obj, inRad, bclOuter, 50, loops);
   glPopMatrix();
}

void flywheel(GLUquadricObj *obj, GLfloat hubRadius, GLfloat innerRim, GLfloat outerRim, GLfloat spokeRadius, GLfloat rimWidth, int numSpokes)
{
   glPushMatrix();
   setMaterial(CHROME);
   gluCylinder(obj, hubRadius, hubRadius, rimWidth, 2 * numSpokes, 1);
   gluCylinder(obj, innerRim, innerRim, rimWidth, 50, 1);
   gluCylinder(obj, outerRim, outerRim, rimWidth, 50, 1);
   gluDisk(obj, 0, hubRadius, numSpokes, loops);
   gluDisk(obj, innerRim, outerRim, 50, loops);
   glTranslatef(0, 0, rimWidth);
   gluDisk(obj, 0, hubRadius, 2 * numSpokes, 5);
   gluDisk(obj, innerRim, outerRim, 50, 10);
   glPopMatrix();

   glPushMatrix();
   setMaterial(BRONZE);
   for (int i = 0; i < numSpokes; i++)
   {
      glRotatef(360 / numSpokes, 0, 0, 1);
      glPushMatrix();
      glTranslatef(hubRadius, 0, rimWidth/2);
      glRotatef(90, 0, 1, 0);
      gluCylinder(obj, spokeRadius, spokeRadius, innerRim - hubRadius, 8, 8);
      glPopMatrix();
   }
   glPopMatrix();
}


// A link is two rings connected by a rod.  The origin is at the
// centre of the first ring.  The length is the distance between
// the centres of the rings.
void link(GLUquadricObj *wheel, GLfloat len, GLfloat height, GLfloat inrad1, GLfloat outrad1, GLfloat inrad2, GLfloat outrad2, GLfloat linkRad)
{
   glPushMatrix();
   ring(wheel, inrad1, outrad1, height);
   glTranslatef(len, 0, 0);
   ring(wheel, inrad2, outrad2, height);
   glPopMatrix();
   glPushMatrix();
   glTranslatef(outrad1, 0, height / 2);
   glRotatef(90, 0, 1, 0);
   gluCylinder(wheel, linkRad, linkRad, len - outrad1 - outrad2, slices, loops);
   glPopMatrix();
   glPopMatrix();
}

void piston(GLfloat lLen, GLfloat disp, GLfloat gap, GLfloat thick,
            GLfloat lWidth, GLfloat rRad, GLfloat rLen, GLfloat pRad, GLfloat pLen, GLfloat r2Len = 0)
{
   setMaterial(CHROME);
   glPushMatrix();
   glTranslatef(lLen/2 - disp, 0, (gap + thick)/2);
   glScalef(lLen, lWidth, thick);
   glutSolidCube(1);
   glPopMatrix();

   glPushMatrix();
   glTranslatef(lLen/2 - disp, 0, - (gap + thick)/2);
   glScalef(lLen, lWidth, thick);
   glutSolidCube(1);
   glPopMatrix();

   glPushMatrix();
   glTranslatef(lLen - thick/2 - disp, 0, 0);
   glScalef(thick, lWidth, gap);
   glutSolidCube(1);
   glPopMatrix();

   setMaterial(BRASS);
   glPushMatrix();
   glTranslatef(lLen - disp, 0, 0);
   glRotatef(90, 0, 1, 0);
   disk(wheel, rRad, r2Len > 0 ? r2Len : rLen);
   glPopMatrix();

   glPushMatrix();
   glTranslatef(lLen + rLen - disp, 0, 0);
   glRotatef(90, 0, 1, 0);
   disk(wheel, pRad, pLen);
   glPopMatrix();

   if (r2Len > 0)
   {
      glPushMatrix();
      glTranslatef(lLen + r2Len - disp, 0, 0);
      glRotatef(90, 0, 1, 0);
      disk(wheel, pRad, pLen);
      glPopMatrix();
   }
}

// This function is called to display the scene.
void display ()
{
//   fc.countFrame();
   glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

   // Set up model view
   glLoadIdentity();
   camera.apply();

   inspectionLightPos[0] = 50 * (xMouse - 0.5);
   inspectionLightPos[1] = 50 * (yMouse - 0.5);
   glLightfv(GL_LIGHT0, GL_POSITION, inspectionLightPos);

   // Show lights as spheres
   glDisable(GL_LIGHTING);
   glPushMatrix();
   glTranslatef(light1Pos[0], light1Pos[1], light1Pos[2]);
   if (light1)
      glColor3f(1, 1, 0.8);
   else
      glColor3f(0, 0, 0);
   glutSolidSphere(0.5, 15, 15);
   glPopMatrix();

   glPushMatrix();
   glTranslatef(light2Pos[0], light2Pos[1], light2Pos[2]);
   if (light2)
      glColor3f(1, 1, 0.8);
   else
      glColor3f(0, 0, 0);
   glutSolidSphere(0.5, 15, 15);
   glPopMatrix();

   glEnable(GL_LIGHTING);
   glPushMatrix();
   glTranslatef(light1Pos[0], light1Pos[1], light1Pos[2]);
   glPushMatrix();
   glRotatef(90, -1, 0, 0);
   setMaterial(COPPER);
   gluCylinder(wheel, 0.1, 0.1, 5, 10, 1);
   glPopMatrix();
   glLightfv(GL_LIGHT1, GL_POSITION, zero);
   glPopMatrix();

   glPushMatrix();
   glTranslatef(light2Pos[0], light2Pos[1], light2Pos[2]);
   glPushMatrix();
   glRotatef(90, -1, 0, 0);
   setMaterial(COPPER);
   gluCylinder(wheel, 0.1, 0.1, 5, 10, 1);
   glPopMatrix();
   glLightfv(GL_LIGHT2, GL_POSITION, zero);
   glPopMatrix();

   if (showAxes)
   {
      glPushMatrix();
      glTranslatef(xAxisPos, yAxisPos, zAxisPos);
      glScalef(5, 5, 5);
      axes();
      glPopMatrix();
   }

   // Walls, floor, and ceiling.
   if (showWalls)
   {
      setMaterial(WHITE);
      glEnable(GL_TEXTURE_2D);
      glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);

      texBricks.setTexture(texNames[BRICK]);
      glBegin(GL_QUADS);

      // Left wall
      glNormal3f(1, 0, 0);
      glTexCoord2f(       0,        0);
      glVertex3fv(corners[4]);
      glTexCoord2f(       0, txbScale);
      glVertex3fv(corners[7]);
      glTexCoord2f(txbScale, txbScale);
      glVertex3fv(corners[3]);
      glTexCoord2f(txbScale,        0);
      glVertex3fv(corners[0]);

      // Right wall
      glNormal3f(-1, 0, 0);
      glTexCoord2f(       0,        0);
      glVertex3fv(corners[1]);
      glTexCoord2f(       0, txbScale);
      glVertex3fv(corners[2]);
      glTexCoord2f(txbScale, txbScale);
      glVertex3fv(corners[6]);
      glTexCoord2f(txbScale,        0);
      glVertex3fv(corners[5]);

      // Back wall
      glNormal3f(0, 0, 1);
      glTexCoord2f(       0,        0);
      glVertex3fv(corners[5]);
      glTexCoord2f(       0, txbScale);
      glVertex3fv(corners[6]);
      glTexCoord2f(txbScale, txbScale);
      glVertex3fv(corners[7]);
      glTexCoord2f(txbScale,        0);
      glVertex3fv(corners[4]);

      // Front wall
      glNormal3f(0, 0, -1);
      glTexCoord2f(       0,        0);
      glVertex3fv(corners[0]);
      glTexCoord2f(       0, txbScale);
      glVertex3fv(corners[3]);
      glTexCoord2f(txbScale, txbScale);
      glVertex3fv(corners[2]);
      glTexCoord2f(txbScale,        0);
      glVertex3fv(corners[1]);

      glEnd();

      texConcrete.setTexture(texNames[CONCRETE]);

      glBegin(GL_QUADS);

      // Floor
      glNormal3f(0, 1, 0);
      glTexCoord2f(       0,        0);
      glVertex3fv(corners[0]);
      glTexCoord2f(       0, txbScale);
      glVertex3fv(corners[1]);
      glTexCoord2f(txbScale, txbScale);
      glVertex3fv(corners[5]);
      glTexCoord2f(txbScale,        0);
      glVertex3fv(corners[4]);

      // Ceiling
      glNormal3f(0, -1, 0);
      glTexCoord2f(       0,        0);
      glVertex3fv(corners[2]);
      glTexCoord2f(       0, txbScale);
      glVertex3fv(corners[3]);
      glTexCoord2f(txbScale, txbScale);
      glVertex3fv(corners[7]);
      glTexCoord2f(txbScale,        0);
      glVertex3fv(corners[6]);

      glEnd();
      glDisable(GL_TEXTURE_2D);
   }

   // Girders
   if (showSupports)
   {
      setMaterial(CHROME);
      gleSetJoinStyle(TUBE_JN_ROUND);
      gleExtrusion(17, grContour, grNormals, up, 8, grPathRear, NULL);
      gleExtrusion(17, grContour, grNormals, up, 8, grPathFront, NULL);
   }

   // Rotating parts
   glPushMatrix();
   glRotatef(alpha, 0, 0, 1);
   // Cap axle
   gluDisk(wheel, 0, rAxle, slices, loops);
   // Axle
   axle(l1Axle);
   glTranslatef(0, 0, l1Axle);
   // Flywheel
   flywheel(wheel, fwHubRad, fwInRad, fwOutRad, fwSpokeRad, fwWidth, fwNumSpokes);
   glTranslatef(0, 0, fwWidth);
   // Axle
   axle(l2Axle);
   glTranslatef(0, 0, l2Axle);
   // First crank
   glTranslatef(0, 0, hCrank / 2);
   glPushMatrix();
   glTranslatef(oCrank, 0, 0);
   glScalef(lCrank, wCrank, hCrank);
   setMaterial(CHROME);
   glutSolidCube(1);
   glPopMatrix();
   glTranslatef(0, 0, hCrank + leWidth);
   // Second crank
   glPushMatrix();
   glTranslatef(oCrank, 0, 0);
   glScalef(lCrank, wCrank, hCrank);
   setMaterial(CHROME);
   glutSolidCube(1);
   glPopMatrix();
   glTranslatef(0, 0, hCrank / 2);
   // Axle
   axle(l3Axle);
   glTranslatef(0, 0, l3Axle);
   // Eccentric
   glPushMatrix();
   glRotatef(phaselag, 0, 0, 1);
   glTranslatef(seOffset, 0, 0);
   disk(wheel, seRadius, seWidth, 50);
   glPopMatrix();
   glTranslatef(0, 0, seWidth);
   // Axle
   axle(l4Axle);
   glTranslatef(0, 0, l4Axle);
   // Cap axle
   gluDisk(wheel, 0, rAxle, slices, loops);
   glPopMatrix();


   // Position of main piston
   glPushMatrix();
   GLfloat dist = sqrt(sqr(bcLength) - sqr(leOffset * sin(radians(alpha))));
   glTranslatef(leOffset * cos(radians(alpha)) + dist, 0, mpZ);

   // Main piston
   glPushMatrix();
   glTranslatef(0, 0, leWidth / 2);
   piston(mplLen, mpDisp, mpGap, mpThick, mplWidth, mprRad, mprLen, mppRad, mppLen);
   glPopMatrix();

   // Main crank
   glPushMatrix();
   GLfloat gamma = degrees(atan(leOffset * sin(radians(alpha)) / dist));
   glRotatef(180 - gamma, 0, 0, 1);
   link(wheel, bcLength, leWidth, bcsInner, bcsOuter, bclInner, bclOuter, bcLinkRad);
   glPopMatrix();

   // Position of valve piston
   glPushMatrix();
   GLfloat smallheight = seOffset * sin(radians(alpha + phaselag)) - vHeight;
   GLfloat smalldist = sqrt(sqr(scLength) - sqr(smallheight));
   glTranslatef(seOffset * cos(radians(alpha + phaselag)) + smalldist, vHeight, spZ);

   // Valve piston
   glPushMatrix();
   glTranslatef(0, 0, seWidth / 2);
   piston(splLen, spDisp, spGap, spThick, splWidth, sprRad, sprLen, sppRad, sppLen, spr2Len);
   glPopMatrix();

   // Small crank
   glPushMatrix();
   gamma = degrees(atan(smallheight / smalldist));
   glRotatef(180 - gamma, 0, 0, 1);
   link(wheel, scLength, seWidth, scsInner, scsOuter, sclInner, sclOuter, scLinkRad);
   glPopMatrix();

   glPopMatrix();

   // Settings for tubes
   setMaterial(BLACK_PLASTIC);
   gleSetJoinStyle(TUBE_JN_ROUND);
// gleSetNumSlices(12);
   glePolyCylinder(6, tube1, NULL, spThick);
   glePolyCylinder(6, tube2, NULL, spThick);
   glePolyCylinder(4, tube3, NULL, spThick);
   glePolyCylinder(6, tube4, NULL, spThick);
   glePolyCylinder(5, tube5, NULL, spThick);

   // Cylinders may be transparent
   if (transparent)
      glEnable(GL_BLEND);

   // Main cylinder
   glPushMatrix();
   glTranslatef(mcStart, 0, mcZ);
   glRotatef(90, 0, 1, 0);
   disk(wheel, mcOutRad, mcEnd);
   glTranslatef(0, 0, mcEnd);
   if (transparent)
      setMaterial(GLASS);
   gluCylinder(wheel, mcInRad, mcInRad, mcLen, slices, loops);
   gluCylinder(wheel, mcOutRad, mcOutRad, mcLen, slices, loops);
   glTranslatef(0, 0, mcLen);
   disk(wheel, mcOutRad, mcEnd);
   glPopMatrix();

   // Valve cylinder
   glPushMatrix();
   glTranslatef(vcStart, vHeight, vcZ);
   glRotatef(90, 0, 1, 0);
   disk(wheel, vcOutRad, vcEnd);
   glTranslatef(0, 0, vcEnd);
   if (transparent)
      setMaterial(GLASS);
   gluCylinder(wheel, vcInRad, vcInRad, vcLen, slices, loops);
   gluCylinder(wheel, vcOutRad, vcOutRad, vcLen, slices, loops);
   glTranslatef(0, 0, vcLen);
   disk(wheel, vcOutRad, vcEnd);
   glPopMatrix();

   if (transparent)
      glDisable(GL_BLEND);

   glutSwapBuffers();

   // Refresh title bar
   ostringstream s;
   s <<
   resetiosflags(ios::floatfield) << setiosflags(ios::fixed) << setprecision(1) <<
   "Speed: desired = " << desiredSpeed << ", actual = " << actualSpeed << '.';
   if (showAxes)
   {
      s <<
      "  Origin at (" << xAxisPos << "  " << yAxisPos << "  " << zAxisPos << ')';
   }
//   s << "   " << fc.framesPerSecond() << " fps." << ends;
   glutSetWindowTitle(s.str().c_str());
}

// This function is called when there is nothing else to do.
void idle ()
{
   if (actualSpeed < desiredSpeed)
      actualSpeed += ASINC;
   if (actualSpeed > desiredSpeed)
      actualSpeed -= ASINC;

   const GLfloat ALL_ROUND = 360;
   alpha += actualSpeed;
   if (alpha > ALL_ROUND)
      alpha -= ALL_ROUND;

   camera.idle();

   glutSetWindow(mainWindow);
   glutPostRedisplay();
}

void mouseMovement (int mx, int my)
{
   xMouse = GLfloat(mx) / GLfloat(width);
   yMouse = 1 - GLfloat(my) / GLfloat(height);
}

void changeProjection()
{
   glutSetWindow(mainWindow);
   glMatrixMode(GL_PROJECTION);
   glLoadIdentity();
   gluPerspective(fovy, GLfloat(width) / GLfloat(height), nearPlane, farPlane);
   glMatrixMode(GL_MODELVIEW);
}

void reshapeMainWindow (int newWidth, int newHeight)
{
   width = newWidth;
   height = newHeight;
   glViewport(0, 0, width, height);
   changeProjection();
}

void graphicKeys (unsigned char key, int mx, int my)
{
   switch (key)
   {
      case '+':
         desiredSpeed += DSINC;
         break;

      case '-':
         if (desiredSpeed >= DSINC)
            desiredSpeed -= DSINC;
         break;

      case '0':
         camera.set(eyePositions[0], modelPoints[0]);
         break;

      case '1':
         camera.set(eyePositions[1], modelPoints[1]);
         break;

      case '2':
         camera.set(eyePositions[2], modelPoints[2]);
         break;

      case '3':
         camera.set(eyePositions[3], modelPoints[3]);
         break;

      case '4':
         camera.set(eyePositions[4], modelPoints[4]);
         break;

      case '5':
         camera.set(eyePositions[5], modelPoints[5]);
         break;

      case '6':
         camera.set(eyePositions[6], modelPoints[6]);
         break;

      case '7':
         camera.set(eyePositions[7], modelPoints[7]);
         break;

      case '8':
         camera.set(eyePositions[8], modelPoints[8]);
         break;

      case '9':
         camera.set(eyePositions[9], modelPoints[9]);
         break;

      case 'f':
         glutFullScreen();
         break;

      case 't':
         transparent = ! transparent;
         break;

      case 'x':
         xAxisPos -= 1;
         break;

      case 'X':
         xAxisPos += 1;
         break;

      case 'y':
         yAxisPos -= 1;
         break;

      case 'Y':
         yAxisPos += 1;
         break;

      case 'z':
         zAxisPos -= 1;
         break;

      case 'Z':
         zAxisPos += 1;
         break;

      case 27:
         exit(0);
   }

   changeProjection();
}

void resetCamera()
{
   Point eye(xMin, yMid, zMid);
   Point model(0, 0, 0);
   camera.set(eye, model);
}

void snapshot()
{
   const char *filename = snapFile->get_text();
   PixelMap map(0, 0, width, height);
   map.write(filename);
}

void gluiControls(int mode)
{
   glutSetWindow(mainWindow);
   switch (mode)
   {

      case LIGHTS:
         if (light0)
            glEnable(GL_LIGHT0);
         else
            glDisable(GL_LIGHT0);
         if (light1)
            glEnable(GL_LIGHT1);
         else
            glDisable(GL_LIGHT1);
         if (light2)
            glEnable(GL_LIGHT2);
         else
            glDisable(GL_LIGHT2);
         break;

      case LOCAL_VIEWER:
         if (localViewer)
            glLightModeli(GL_LIGHT_MODEL_LOCAL_VIEWER, GL_FALSE);
         else
            glLightModeli(GL_LIGHT_MODEL_LOCAL_VIEWER, GL_TRUE);
         break;

      case FILL_POLYGONS:
         if (filled)
            gluQuadricDrawStyle(wheel, GLU_FILL);
         else
            gluQuadricDrawStyle(wheel, GLU_LINE);
         break;

      case CAM_ZOOMIN:
         if (fovy > 10)
            fovy -= 5;
         cout << fovy << endl;
         changeProjection();
         break;

      case CAM_ZOOMOUT:
         if (fovy < 85)
            fovy += 5;
         cout << fovy << endl;
         changeProjection();
         break;

      case CAM_UP:
         camera.moveUp(5);
         break;

      case CAM_DOWN:
         camera.moveUp(-5);
         break;

      case CAM_LEFT:
         camera.moveLeft(5);
         break;

      case CAM_RIGHT:
         camera.moveLeft(-5);
         break;

      case CAM_FORWARD:
         camera.moveForward(5);
         break;

      case CAM_BACKWARD:
         camera.moveForward(-5);
         break;

      case CAM_TILTUP:
         camera.tiltUp(0.3);
         break;

      case CAM_TILTDOWN:
         camera.tiltUp(-0.3);
         break;

      case CAM_PANLEFT:
         camera.panLeft(0.3);
         break;

      case CAM_PANRIGHT:
         camera.panLeft(-0.3);
         break;

      case CAM_RESET:
         resetCamera();
         fovy = 40;
         changeProjection();
         break;

      case CAM_SHOW:
         cout << camera;
         break;

      case VIEW_00:
         camera.set(eyePositions[0], modelPoints[0]);
         break;

      case VIEW_01:
         camera.set(eyePositions[1], modelPoints[1]);
         break;

      case VIEW_02:
         camera.set(eyePositions[2], modelPoints[2]);
         break;

      case VIEW_03:
         camera.set(eyePositions[3], modelPoints[3]);
         break;

      case VIEW_04:
         camera.set(eyePositions[4], modelPoints[4]);
         break;

      case VIEW_05:
         camera.set(eyePositions[5], modelPoints[5]);
         break;

      case VIEW_06:
         camera.set(eyePositions[6], modelPoints[6]);
         break;

      case VIEW_07:
         camera.set(eyePositions[7], modelPoints[7]);
         break;

      case VIEW_08:
         camera.set(eyePositions[8], modelPoints[8]);
         break;

      case VIEW_09:
         camera.set(eyePositions[9], modelPoints[9]);
         break;

      case FASTER:
         desiredSpeed += DSINC;
         break;

      case SLOWER:
         if (desiredSpeed >= DSINC)
            desiredSpeed -= DSINC;
         break;

      case SNAPSHOT:
         snapshot();
   }
}

void gluiSetup(int mainWindow)
{
   GLUI_Master.set_glutIdleFunc(idle);
   glui = GLUI_Master.create_glui("Controls", 0, 0, HEIGHT + 50);

   GLUI_Panel *cameraControl = glui->add_panel("Camera control");

   glui->add_button_to_panel(cameraControl, "Up",        CAM_UP,       gluiControls);
   glui->add_button_to_panel(cameraControl, "Down",      CAM_DOWN,     gluiControls);
   glui->add_button_to_panel(cameraControl, "Left",      CAM_LEFT,     gluiControls);
   glui->add_button_to_panel(cameraControl, "Right",     CAM_RIGHT,    gluiControls);
   glui->add_button_to_panel(cameraControl, "Forward",   CAM_FORWARD,  gluiControls);
   glui->add_button_to_panel(cameraControl, "Backward",  CAM_BACKWARD, gluiControls);
   glui->add_button_to_panel(cameraControl, "Tilt up",   CAM_TILTUP,   gluiControls);
   glui->add_button_to_panel(cameraControl, "Tilt down", CAM_TILTDOWN, gluiControls);
   glui->add_button_to_panel(cameraControl, "Pan left",  CAM_PANLEFT,  gluiControls);
   glui->add_button_to_panel(cameraControl, "Pan right", CAM_PANRIGHT, gluiControls);
   glui->add_button_to_panel(cameraControl, "Zoom in",   CAM_ZOOMIN,   gluiControls);
   glui->add_button_to_panel(cameraControl, "Zoom out",  CAM_ZOOMOUT,  gluiControls);
   glui->add_button_to_panel(cameraControl, "Reset",     CAM_RESET,    gluiControls);
   glui->add_button_to_panel(cameraControl, "Show",      CAM_SHOW,     gluiControls);

   glui->add_column(true);

   GLUI_Panel *views = glui->add_panel("Views");
   glui->add_button_to_panel(views, "Start",   VIEW_00, gluiControls);
   glui->add_button_to_panel(views, "View  1", VIEW_01, gluiControls);
   glui->add_button_to_panel(views, "View  2", VIEW_02, gluiControls);
   glui->add_button_to_panel(views, "View  3", VIEW_03, gluiControls);
   glui->add_button_to_panel(views, "View  4", VIEW_04, gluiControls);
   glui->add_button_to_panel(views, "View  5", VIEW_05, gluiControls);
   glui->add_button_to_panel(views, "View  6", VIEW_06, gluiControls);
   glui->add_button_to_panel(views, "View  7", VIEW_07, gluiControls);
   glui->add_button_to_panel(views, "View  8", VIEW_08, gluiControls);
   glui->add_button_to_panel(views, "View  9", VIEW_09, gluiControls);

   glui->add_column(true);

   GLUI_Panel *switches = glui->add_panel("Switches");
   glui->add_checkbox_to_panel(switches, "Light 1",                 &light1, LIGHTS, gluiControls);
   glui->add_checkbox_to_panel(switches, "Light 2",                 &light2, LIGHTS, gluiControls);
   glui->add_checkbox_to_panel(switches, "Inspection light",        &light0, LIGHTS, gluiControls);
   glui->add_checkbox_to_panel(switches, "Show supports",           &showSupports);
   glui->add_checkbox_to_panel(switches, "Show walls",              &showWalls);
   glui->add_checkbox_to_panel(switches, "Filled polygons",         &filled, FILL_POLYGONS, gluiControls);
   glui->add_checkbox_to_panel(switches, "Transparent cylinders",   &transparent);
   glui->add_checkbox_to_panel(switches, "'Local viewer' lighting", &localViewer, LOCAL_VIEWER, gluiControls);
   glui->add_checkbox_to_panel(switches, "Axes",                    &showAxes, AXES, gluiControls);

   GLUI_Panel *actions = glui->add_panel("Actions");
   glui->add_button_to_panel(actions, "Faster", FASTER, gluiControls);
   glui->add_button_to_panel(actions, "Slower", SLOWER, gluiControls);

   GLUI_Panel *snapshot = glui->add_panel("Snapshot");
   snapFile = glui->add_edittext_to_panel(snapshot, "File name");
   snapFile->set_text("engine.bmp");
   glui->add_button_to_panel(snapshot, "Snap", SNAPSHOT, gluiControls);

   resetCamera();
}

int main (int argc, char **argv)
{
   cout <<
   "Engine Simulation\n\n"
   "Keys:\n"
   "  0..9  camera positions\n"
   "  f     full screen\n"
   "  t     toggle tranparent cylinders\n"
   "  +     faster\n"
   "  -     slower\n"
   " ESC    exit\n";

   glutInit(&argc, argv);
   glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGB | GLUT_DEPTH);
   glutInitWindowSize(width, height);
   mainWindow = glutCreateWindow("Steam Engine");
   glutDisplayFunc(display);
   glutReshapeFunc(reshapeMainWindow);
   glutKeyboardFunc(graphicKeys);
   glutPassiveMotionFunc(mouseMovement);

   buildModel();

   glEnable(GL_DEPTH_TEST);
   glEnable(GL_NORMALIZE);
   gluiSetup(mainWindow);
   glutMainLoop();
   return 0;
}
