///////////////////////////////////////////////////////////////////////////////
//
// ogl.h
//
// Purpose:   Declaration of a canvas for drawing basic primitives by OpenGL
//            and visualization of skeletons in the scene.
//            Classes: MyGLCanvas.
//
// Created:   Jaroslav Semancik, 21/10/2003
//
///////////////////////////////////////////////////////////////////////////////

#ifndef OGL_H
#define OGL_H

///////////////////////////////////////////////////////////////////////////////
//
// class MyGLCanvas
//
// MyGLCanvas is a class for drawing canvas of the application. OpenGL is
// used for rendering. Skeletons are rendered wireframe or solid due to
// solid_rendering attribute value. The scene can be transformed by mouse
// dragging with: left button - rotate, right button - zoom, or both
// buttons - pan.   
//
///////////////////////////////////////////////////////////////////////////////

class MyGLCanvas : public wxGLCanvas
{
public:
    
    bool solid_rendering;       // solid or wireframe OpenGL rendering

    MyGLCanvas(wxWindow *parent, const wxWindowID id = -1, const wxPoint& pos = wxDefaultPosition,
        const wxSize& size = wxDefaultSize, long style = 0, const wxString& name = "MyGLCanvas");
    ~MyGLCanvas();
    void InitGL();
    void SetInitialView();
    void OnSize(wxSizeEvent& event);
    void OnPaint(wxPaintEvent& event);
    void OnMouse(wxMouseEvent& event);


    // Renders point p as a point a or a small sphere due to solid_rendering
    // value.
    
    void RenderPoint(const Vector& p);
                     

    // Renders line segment (p1, p2) as a line or a thin cylinder due to
    // solid_rendering value.
                     
    void RenderLine(const Vector& p1, const Vector& p2);


    // Renders all checked shots in the wxWindows list box shotlistbox
    // to a display list.
                    
    void DrawCheckedShots(const wxCheckListBox *shotlistbox);

    
private:

    // Projection parameters: field of view in y-direction (in degrees),
    // near and far clipping planes.
    
    float m_fovy, m_near, m_far;

    // Current transformation of the scene for viewing.
    
    Matrix m_rotation;
    Vector m_translation;

    // Display lists for the ground grid and for skeletons.
    
    int m_grid_dl, m_skeletons_dl;

    // Last mouse position, being used in dragging.
    
    wxPoint m_last_mouse;

    void draw_grid();
    void draw_coord_frame();

    DECLARE_EVENT_TABLE()
};


// A global instance for main drawing canvas of the application.

extern "C++" MyGLCanvas *glCanvas;

#endif

