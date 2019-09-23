///////////////////////////////////////////////////////////////////////////////
//
// ogl.cpp
//
// Purpose:   Implementation of a canvas for drawing basic primitives by
//            OpenGL and visualization of skeletons in the scene.
//            Classes: MyGLCanvas.
//
// Created:   Jaroslav Semancik, 21/10/2003
//
///////////////////////////////////////////////////////////////////////////////

#include <wx/wx.h>
#include <wx/glcanvas.h>
#include <wx/image.h>
#include <wx/splitter.h>

#ifdef __WXMAC__
#  ifdef __DARWIN__
#    include <OpenGL/glu.h>
#  else
#    include <glu.h>
#  endif
#else
#  include <GL/glu.h>
#endif

#include <string>
#include <sstream>
#include <fstream>
#include <iostream>
#include <stdexcept>
#include <list>
#include <vector>

using namespace std;

#include "base.h"
#include "algebra.h"
#include "motion.h"
#include "skeleton.h"
#include "ogl.h"
#include "wxmv.h"


///////////////////////////////////////////////////////////////////////////////
//
// class MyGLCanvas event table
//
///////////////////////////////////////////////////////////////////////////////

BEGIN_EVENT_TABLE(MyGLCanvas, wxGLCanvas)
    EVT_SIZE(MyGLCanvas::OnSize)
    EVT_PAINT(MyGLCanvas::OnPaint)
    EVT_MOUSE_EVENTS(MyGLCanvas::OnMouse)
END_EVENT_TABLE()


///////////////////////////////////////////////////////////////////////////////
//
// class MyGLCanvas - public methods
//
///////////////////////////////////////////////////////////////////////////////

// constructor - initialize attributes (projection, scene transformation and
// display lists)

MyGLCanvas::MyGLCanvas(wxWindow *parent, wxWindowID id,
    const wxPoint& pos, const wxSize& size, long style, const wxString& name)
    : wxGLCanvas(parent, id, pos, size, style, name)
{
    solid_rendering = true;

    // set projection parameters
    m_fovy = 45.0;
    m_near = 1.0;
    m_far = 1000.0;

    SetInitialView();

    // allocate display list id's
    m_grid_dl = glGenLists(1);
    m_skeletons_dl = glGenLists(1);
}


// destructor

MyGLCanvas::~MyGLCanvas()
{
}


// initialize OpenGL state - various rendering settings
// prepare static display lists

void MyGLCanvas::InitGL(void)
{
    // two white lights
    GLfloat light_Ka[] = { 0.0, 0.0, 0.0, 1.0 };
    GLfloat light_Kd[] = { 0.8, 0.8, 0.8, 0.8 };
    GLfloat light_Ks[] = { 1.0, 1.0, 1.0, 1.0 };
    GLfloat light0_pos[] = { -50.0, 50.0, 30.0, 0.0 };
    GLfloat light1_pos[] = {  50.0, 50.0, -10.0, 0.0 };

    // common properties of materials
    GLfloat mat_Ks[] = { 0.5, 0.5, 0.5, 1.0 };
    GLfloat mat_Ke[] = { 0.0, 0.0, 0.0, 0.0 };
    GLfloat mat_Sh[] = { 90.0 };

    // lights
    glLightfv(GL_LIGHT0, GL_AMBIENT,  light_Ka);
    glLightfv(GL_LIGHT0, GL_DIFFUSE,  light_Kd);
    glLightfv(GL_LIGHT0, GL_SPECULAR, light_Ks);
    glLightfv(GL_LIGHT0, GL_POSITION, light0_pos);

    glLightfv(GL_LIGHT1, GL_AMBIENT,  light_Ka);
    glLightfv(GL_LIGHT1, GL_DIFFUSE,  light_Kd);
    glLightfv(GL_LIGHT1, GL_SPECULAR, light_Ks);
    glLightfv(GL_LIGHT1, GL_POSITION, light1_pos);

    glEnable(GL_LIGHT0);
    glEnable(GL_LIGHT1);
    glEnable(GL_NORMALIZE);

    // material
    glMaterialfv(GL_FRONT_AND_BACK, GL_SPECULAR, mat_Ks);
    glMaterialfv(GL_FRONT_AND_BACK, GL_EMISSION, mat_Ke);
    glMaterialfv(GL_FRONT_AND_BACK, GL_SHININESS, mat_Sh);

    glColorMaterial(GL_FRONT_AND_BACK, GL_AMBIENT_AND_DIFFUSE);
    glEnable(GL_COLOR_MATERIAL);

    // screen buffer
    glClearColor(0.0, 0.2, 0.1, 0.0);
    glClearDepth(1.0);
    glEnable(GL_DEPTH_TEST);
    glDepthFunc(GL_LEQUAL);
    glShadeModel(GL_SMOOTH);

    // prepare static display lists
    draw_grid();
}


// set initial view to the scene

void MyGLCanvas::SetInitialView()
{
    // set initial rotation of the scene
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
    glRotatef(40.0, 0.0, 1.0, 0.0);
    glGetDoublev(GL_MODELVIEW_MATRIX, (GLdouble*)m_rotation.c);

    // set initial translation of the scene
    m_translation = Vector(0.0, -50.0, -250.0);
}


void MyGLCanvas::OnPaint(wxPaintEvent& event)
{
    // must always be here
    wxPaintDC dc(this);

#ifndef __WXMOTIF__
    if (!GetContext()) return;
#endif

    SetCurrent();

    // scene transformation
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
    glTranslated(m_translation.x, m_translation.y, m_translation.z);
    glMultMatrixd((GLdouble*)m_rotation.c);

    glClear( GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT );

    // draw scene
    glDisable(GL_LIGHTING);
    glCallList(m_grid_dl);          // basic grid

    if (solid_rendering) glEnable(GL_LIGHTING);
    glCallList(m_skeletons_dl);     // skeletons

    // flush and swap buffers
    glFlush();
    SwapBuffers();
}


// on resize reset viewport and projection

void MyGLCanvas::OnSize(wxSizeEvent& event)
{
    // this is also necessary to update the context on some platforms
    wxGLCanvas::OnSize(event);

    int w, h;
    GetClientSize(&w, &h);
#ifndef __WXMOTIF__
    if (GetContext())
#endif
    {
        SetCurrent();
        glViewport(0, 0, w, h);

        // reset perspective projection
        glMatrixMode(GL_PROJECTION);
        glLoadIdentity();
        gluPerspective(m_fovy, (float)w / h, m_near, m_far);
    }
}


void MyGLCanvas::OnMouse(wxMouseEvent& event)
{
    int w, h;
    GetClientSize(&w, &h);

    if (event.Dragging())
    {
    	wxPoint mouse = event.GetPosition();

        if (m_last_mouse.x < 0);    // just save current mouse position

    	else if (event.LeftIsDown() && event.RightIsDown())
    	{
    		// pan
            float k = 2.0 * tan(DEG2RAD(m_fovy / 2)) / h;
    		m_translation.x -= (mouse.x - m_last_mouse.x) * m_translation.z * k;
    		m_translation.y += (mouse.y - m_last_mouse.y) * m_translation.z * k;
    	}

    	else if (event.LeftIsDown())
    	{
    		// rotate
    		float kx = 100.0 / h;
    		float ky = 100.0 / w;
            float rx = (mouse.y - m_last_mouse.y) * ky;
            float ry = (mouse.x - m_last_mouse.x) * kx;

            // add rotation to rotation matrix of the scene
            glMatrixMode(GL_MODELVIEW);
    		glLoadIdentity();
    		glRotatef(rx, 1.0, 0.0, 0.0);
    		glRotatef(ry, 0.0, 1.0, 0.0);
            glMultMatrixd((GLdouble*)m_rotation.c);
            glGetDoublev(GL_MODELVIEW_MATRIX, (GLdouble*)m_rotation.c);
    	}

        else if (event.RightIsDown())
        {
        	// scale
            float k = 3.0 / h;
        	m_translation.z += (double)(mouse.y - m_last_mouse.y)
                                * m_translation.z * k;
        }

        m_last_mouse = mouse;
        // orientation has changed, redraw the scene
        Refresh(false);     // wxWindows call
    }

    else
        m_last_mouse = wxPoint(-1, -1);
}


// Renders point p as a point a or a small sphere due to solid_rendering
// value.

#define POINT_SIZE  1.0

void MyGLCanvas::RenderPoint(const Vector& p)
{
    if (solid_rendering)
    {
        glPushMatrix();
        glTranslated(p.x, p.y, p.z);
        GLUquadricObj *point = gluNewQuadric();
            gluSphere(point, POINT_SIZE, 10, 10);
        gluDeleteQuadric(point);
        glPopMatrix();
    }

    else
    {
        glBegin(GL_POINTS);
            glVertex3dv(p.c);
        glEnd();
    }
}


// Renders line segment (p1, p2) as a line or a thin cylinder due to
// solid_rendering value.
                     
#define LINE_THICKNESS  0.5

void MyGLCanvas::RenderLine(const Vector& p1, const Vector& p2)
{
    double d, x_rot, y_rot;

    if (solid_rendering)
    {
        // determine line length and rotation angles needed to align
        // positive Z-axis onto vector direction
        Vector v = p2 - p1;
        v.polar(d, x_rot, y_rot);

        // transform
        glPushMatrix();
        glTranslated(p1.x, p1.y, p1.z);
        glRotated(RAD2DEG(y_rot), 0.0, 1.0, 0.0);
        glRotated(RAD2DEG(x_rot), 1.0, 0.0, 0.0);
        GLUquadricObj *line = gluNewQuadric();
            gluCylinder(line, LINE_THICKNESS, LINE_THICKNESS, d, 10, 1);
        gluDeleteQuadric(line);
        glPopMatrix();
    }

    else
    {
        glBegin(GL_LINES);
            glVertex3dv(p1.c);
            glVertex3dv(p2.c);
        glEnd();
    }
}


// Renders all checked shots in the wxWindows list box shotlistbox
// to a display list.
                    
void MyGLCanvas::DrawCheckedShots(const wxCheckListBox *shotlistbox)
{
    glNewList(m_skeletons_dl, GL_COMPILE);

    // draw all checked shots for current time value
    int n = shotlistbox->GetCount();
    for (int i = 0; i < n; i++)
    	if (shotlistbox->IsChecked(i))
            shots[i]->Draw(mainFrame->time);

    glEndList();
   	Refresh(false);     // wxWindows call
}


///////////////////////////////////////////////////////////////////////////////
//
// class MyGLCanvas - private methods
//
///////////////////////////////////////////////////////////////////////////////

// draw basic grid in XZ plane around origin and a coordinate frame to
// a display list

void MyGLCanvas::draw_grid()
{
    glNewList(m_grid_dl, GL_COMPILE);

    // grid
    glColor3f(0.5, 0.5, 0.5);
    glBegin(GL_LINES);
    for (int i = -100; i <= 100; i += 20)
    {
        glVertex3f(i, 0.0, -100.0);
        glVertex3f(i, 0.0, 100.0);
        glVertex3f(-100.0, 0.0, i);
        glVertex3f(100.0, 0.0, i);
    }
    glEnd();

    draw_coord_frame();

    glEndList();
}


// draw XYZ frame at origin

void MyGLCanvas::draw_coord_frame()
{
    bool sr_bak = solid_rendering;
    solid_rendering = false;

    double d = 20.0;                // length of axes
    double h = 4.0;                 // height and width of letters
    double w = 3.0;
    Vector tx(2*d/3, -4*h/3, 0.0);  // translation of characters
    Vector ty(-4*w/3, 2*d/3, 0.0);
    Vector tz(-w/2, -4*h/3, 2*d/3);

    // X axis
    glColor3f(1.0, 0.0, 0.0);
    glCanvas->RenderLine(zero_vector, d * x_vector);

    // letter X
    glPushMatrix();
    glTranslated(tx.x, tx.y, tx.z);
    glCanvas->RenderLine(zero_vector, Vector(w, h, 0.0));
    glCanvas->RenderLine(Vector(w, 0.0, 0.0), Vector(0.0, h, 0.0));
    glPopMatrix();

    // Y axis
    glColor3f(0.0, 1.0, 0.0);
    glCanvas->RenderLine(zero_vector, d * y_vector);

    // letter Y
    glPushMatrix();
    glTranslated(ty.x, ty.y, ty.z);
    glCanvas->RenderLine(Vector(w/2, 0.0, 0.0), Vector(w/2, h/2, 0.0));
    glCanvas->RenderLine(Vector(w/2, h/2, 0.0), Vector(0.0, h, 0.0));
    glCanvas->RenderLine(Vector(w/2, h/2, 0.0), Vector(w, h, 0.0));
    glPopMatrix();

    // Z axis
    glColor3f(0.0, 0.0, 1.0);
    glCanvas->RenderLine(zero_vector, d * z_vector);

    // letter Z
    glPushMatrix();
    glTranslated(tz.x, tz.y, tz.z);
    glCanvas->RenderLine(Vector(0.0, h, 0.0), Vector(w, h, 0.0));
    glCanvas->RenderLine(Vector(w, h, 0.0), zero_vector);
    glCanvas->RenderLine(zero_vector, Vector(w, 0.0, 0.0));
    glPopMatrix();
    
    solid_rendering = sr_bak;
}



// A global instance for main drawing canvas of the application.

MyGLCanvas *glCanvas = NULL;

