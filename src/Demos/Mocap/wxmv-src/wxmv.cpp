///////////////////////////////////////////////////////////////////////////////
//
// wxmv.cpp
//
// Purpose:   Implementation of wxWindows user interface for WX Motion
//            Viewer. Main file of the program.
//            Classes: MyApp, MyFrame, MySplitter, ExpSetDialog, AboutDialog.
//
// Created:   Jaroslav Semancik, 12/12/2003
//
///////////////////////////////////////////////////////////////////////////////

#include "wx/wxprec.h"

#ifdef __BORLANDC__
    #pragma hdrstop
#endif

#ifndef WX_PRECOMP
    #include <wx/wx.h>
#endif
#include <wx/glcanvas.h>
#include <wx/splitter.h>

#include <string>
#include <sstream>
#include <fstream>
#include <iostream>
#include <iomanip>
#include <stdexcept>
#include <math.h>
#include <list>
#include <vector>

using namespace std;

#include "base.h"
#include "algebra.h"
#include "motion.h"
#include "skeleton.h"
#include "bvh.h"
#include "ogl.h"
#include "wxmv.h"


// resources

#ifdef __WXMSW__
// resources are defined in wxmv.rc
#else
    #include "resources/dancer.xpm"
    #include "resources/open.xpm"
    #include "resources/back.xpm"
    #include "resources/back_one.xpm"
    #include "resources/pause.xpm"
    #include "resources/fwd_one.xpm"
    #include "resources/fwd.xpm"
    #include "resources/reset.xpm"
    #include "resources/help.xpm"
#endif


// event tables and other macros for wxWindows

BEGIN_EVENT_TABLE(MyFrame, wxFrame)
    EVT_MENU(ID_OPEN,                   MyFrame::OnOpen)
    EVT_MENU(ID_CLOSE,                  MyFrame::OnClose)
    EVT_MENU(ID_QUIT,                   MyFrame::OnQuit)
    EVT_MENU(ID_BACK,                   MyFrame::OnBack)
    EVT_MENU(ID_BACK_ONE,               MyFrame::OnBackOne)
    EVT_MENU(ID_PAUSE,                  MyFrame::OnPause)
    EVT_MENU(ID_FWD_ONE,                MyFrame::OnFwdOne)
    EVT_MENU(ID_FWD,                    MyFrame::OnFwd)
    EVT_MENU(ID_RESET_TIME,             MyFrame::OnResetTime)
    EVT_MENU(ID_RESET_VIEW,             MyFrame::OnResetView)
    EVT_MENU(ID_TOGGLE_CYCLE,           MyFrame::OnToggleCycle)
    EVT_MENU(ID_TOGGLE_SOLID,           MyFrame::OnToggleSolid)
    EVT_MENU(ID_TOGGLE_SHOWFILES,       MyFrame::OnToggleShowFiles)
    EVT_MENU(ID_TOGGLE_TOOLBAR,         MyFrame::OnToggleToolbar)
    EVT_MENU(ID_TOGGLE_STATUSBAR,       MyFrame::OnToggleStatusbar)
    EVT_MENU(ID_BLEND,                  MyFrame::OnBlend)
    EVT_MENU(ID_INTERPOLATE,            MyFrame::OnInterpolate)
    EVT_MENU(ID_EXPORT,                 MyFrame::OnExport)
    EVT_MENU(ID_EXPORT_SETTINGS,        MyFrame::OnExportSettings)
    EVT_MENU(ID_ABOUT,                  MyFrame::OnAbout)
    EVT_TIMER(ID_TIMER_FWD,             MyFrame::OnFwdOne)
    EVT_TIMER(ID_TIMER_BACK,            MyFrame::OnBackOne)
    EVT_CHECKLISTBOX(ID_FILELIST,       MyFrame::OnToggleShotCheckbox)
END_EVENT_TABLE()

IMPLEMENT_APP(MyApp)


// global variables

MyFrame *mainFrame;


///////////////////////////////////////////////////////////////////////////////
//
// class MyApp - public methods
//
///////////////////////////////////////////////////////////////////////////////

bool MyApp::OnInit()
{
    srand(time(NULL));

    // main application frame
    mainFrame = new MyFrame(_T("WX Motion Viewer"),
                            wxDefaultPosition, wxDefaultSize);
    mainFrame->Show(true);
    SetTopWindow(mainFrame);
    return true;
}



///////////////////////////////////////////////////////////////////////////////
//
// class MyFrame - public methods
//
///////////////////////////////////////////////////////////////////////////////

MyFrame::MyFrame(const wxString& title, const wxPoint& pos, const wxSize& size)
    : wxFrame((wxFrame*) NULL, -1, title, pos, size)
{
    SetIcon(wxICON(dancer));

#if wxUSE_MENUS
    // prepare menu
    build_menu();
#else
#error Recompile wxWindows with wxUSE_MENUS, wxUSE_GLCANVAS and wxUSE_TIMER set to 1 in setup.h
#endif

#if wxUSE_TOOLBAR
    // toolbar is visible by default
    recreate_toolbar();
#else
#message You should recompile wxWindows also with wxUSE_TOOLBAR and wxUSE_STATUSBAR set to 1 in setup.h
    m_toolBar = NULL;
#endif

#if wxUSE_STATUSBAR
    // statusbar is visible by default
    recreate_statusbar();
    SetStatusText(_T("Welcome to Motion Viewer!"));
#else
#message You should recompile wxWindows also with wxUSE_TOOLBAR and wxUSE_STATUSBAR set to 1 in setup.h
    m_statusBar = NULL;
#endif

    // create splitter window
    m_splitter = new MySplitter(this);

    // create list for opened files
    m_fileList = new wxCheckListBox(m_splitter, ID_FILELIST,
        wxDefaultPosition, wxDefaultSize, 0, NULL, wxLB_EXTENDED);

#if wxUSE_GLCANVAS
    // OpenGL drawing canvas
    glCanvas = new MyGLCanvas(m_splitter, -1, wxDefaultPosition,
        wxDefaultSize, wxSUNKEN_BORDER);
    glCanvas->InitGL();
#else
#error Recompile wxWindows with wxUSE_MENUS, wxUSE_GLCANVAS and wxUSE_TIMER set to 1 in setup.h
#endif

    // list of opened files is visible by deafault
    m_splitter->SplitVertically(m_fileList, glCanvas);

#if wxUSE_TIMER
    m_timer = new wxTimer();
#else
#error Recompile wxWindows with wxUSE_MENUS, wxUSE_GLCANVAS and wxUSE_TIMER set to 1 in setup.h
#endif

    // default settings for playback
    fwd_playing = true;
    time = 0;
    frametime = .033333;    // 30 fps

    // joints and coordinates exported to CSV by default
    joints_to_csv = "ltoes-EF rtoes-EF ";
    coords_to_csv = "xyzv";

    // default settings for export to a batch file
    ltoes_name = "ltoes-EF";
    rtoes_name = "rtoes-EF";
    velocity_delta = 0.0001;
    velocity_limit = 10.0;
    height_limit = 1.5;

    // default evaluating of motions - wrap around
    Motion::wrap = true;    // static attribute of the Motion class

    m_menuBar->Check(ID_TOGGLE_CYCLE, Motion::wrap);
    m_menuBar->Check(ID_TOGGLE_SOLID, glCanvas->solid_rendering);

    show_time();
}


// Shows standard open dialog and opens selected BVH file(s).

void MyFrame::OnOpen(wxCommandEvent& event)
{
    wxFileDialog *dlg = new wxFileDialog(this, _T("Open a motion file"),
        _T(""), _T(".bvh"),
        _T("Biovision hierarchy files (*.bvh)|*.bvh|All files (*.*)|*.*"),
        wxOPEN | wxFILE_MUST_EXIST | wxCHANGE_DIR | wxMULTIPLE);

    // exit unless a file is selected
    if (dlg->ShowModal() != wxID_OK)
    {
        dlg->Destroy();
        return;
    }

    string path = string(dlg->GetDirectory());
    wxArrayString filenames;
    dlg->GetFilenames(filenames);
    dlg->Destroy();
    
    open_bvh_files(path, filenames);
}


// Closes all selected shots in the filelist. Destroys respective shots,
// motions and skeletons. Disables animation tools if no shot remains open.

void MyFrame::OnClose(wxCommandEvent& event)
{
    // delete all checked shots and destroy them
    int n = m_fileList->GetCount();
    for (int i = n - 1; i >= 0; i--)
    	if (m_fileList->Selected(i))
        {
            m_fileList->Delete(i);
            delete shots[i];
            shots.erase(shots.begin() + i);
        }

    // disable animation tools if the file list is empty
    if (m_fileList->GetCount() == 0)
        enable_animation_tools(false);

    // redisplay the scene
    glCanvas->DrawCheckedShots(m_fileList);
}


// Destroys all shots, motions and skeletons before terminating.

void MyFrame::OnQuit(wxCommandEvent& WXUNUSED(event))
{
    while (!shots.empty())
    {
        delete shots.back();
        shots.pop_back();
    }

    while (!motions.empty())
    {
        delete motions.back();
        motions.pop_back();
    }

    while (!skeletons.empty())
    {
        delete skeletons.back();
        skeletons.pop_back();
    }

    Close(true);
}


void MyFrame::OnResetTime(wxCommandEvent& WXUNUSED(event))
{
    time = 0;
    show_time();

    glCanvas->DrawCheckedShots(m_fileList);
}


void MyFrame::OnResetView(wxCommandEvent& WXUNUSED(event))
{
    glCanvas->SetInitialView();
    glCanvas->DrawCheckedShots(m_fileList);
}


void MyFrame::OnToggleCycle(wxCommandEvent& event)
{
    // switch motion wrapping state
    Motion::wrap = Motion::wrap ? false : true;

    // set tools due to cycle_anim
    m_menuBar->Check(ID_TOGGLE_CYCLE, Motion::wrap);
}


void MyFrame::OnToggleSolid(wxCommandEvent& WXUNUSED(event))
{
    // switch solid_rendering
    glCanvas->solid_rendering = glCanvas->solid_rendering ? false : true;

    // set tools due to solid_rendering
    m_menuBar->Check(ID_TOGGLE_SOLID, glCanvas->solid_rendering);

    // redraw scene with new setting
    glCanvas->DrawCheckedShots(m_fileList);
}


void MyFrame::OnToggleShowFiles(wxCommandEvent& WXUNUSED(event))
{
    bool show = m_menuBar->IsChecked(ID_TOGGLE_SHOWFILES);

    if (show)
    {
        m_fileList->Show(TRUE);
        glCanvas->Show(TRUE);
        m_splitter->SplitVertically(m_fileList, glCanvas);
    }
    else
        m_splitter->Unsplit(m_fileList);
}


void MyFrame::OnToggleShotCheckbox(wxCommandEvent& WXUNUSED(event))
{
    // redisplay the scene
    glCanvas->DrawCheckedShots(m_fileList);
}


void MyFrame::OnToggleToolbar(wxCommandEvent& WXUNUSED(event))
{
    bool show = m_menuBar->IsChecked(ID_TOGGLE_TOOLBAR);

    if (show)
        recreate_toolbar();
    else {
        delete m_toolBar;
        m_toolBar = NULL;
        SetToolBar(NULL);
    }

    // resize canvas
    wxSize client = this->GetClientSize();
    glCanvas->SetSize(client);
}


void MyFrame::OnToggleStatusbar(wxCommandEvent& WXUNUSED(event))
{
    bool show = m_menuBar->IsChecked(ID_TOGGLE_STATUSBAR);

    if (show)
        recreate_statusbar();
    else {
        delete m_statusBar;
        m_statusBar = NULL;
        SetStatusBar(NULL);
    }

    // resize canvas
    wxSize client = this->GetClientSize();
    m_splitter->SetSize(client);
}



void MyFrame::OnAbout(wxCommandEvent& WXUNUSED(event))
{
    AboutDialog *dlg = new AboutDialog(this);
    dlg->ShowModal();
    dlg->Destroy();
}


void MyFrame::OnFwdOne(wxCommandEvent& WXUNUSED(event))
{
    time += frametime;
    show_time();

    glCanvas->DrawCheckedShots(m_fileList);
}


void MyFrame::OnBackOne(wxCommandEvent& WXUNUSED(event))
{
    time -= frametime;
    show_time();

    glCanvas->DrawCheckedShots(m_fileList);
}


void MyFrame::OnFwd(wxCommandEvent& WXUNUSED(event))
{
    SetStatusText(_T("Playing forward..."));
    fwd_playing = true;
    m_timer->SetOwner(this, ID_TIMER_FWD);
    m_timer->Start(int(frametime * 1000));  // time interval is in miliseconds
}


void MyFrame::OnBack(wxCommandEvent& WXUNUSED(event))
{
    SetStatusText(_T("Playing backwards..."));
    fwd_playing = false;
    m_timer->SetOwner(this, ID_TIMER_BACK);
    m_timer->Start(int(frametime * 1000));  // time interval is in miliseconds
}


void MyFrame::OnPause(wxCommandEvent& event)
{
    if (m_timer->IsRunning()) stop_animation();
    else
    {
        if (fwd_playing) OnFwd(event);
        else OnBack(event);
    }
}


// Creates a new shot with a motion blending motion of all selected shots
// with uniform weights. Selected shots must have identical skeletons.

void MyFrame::OnBlend(wxCommandEvent& event)
{
    int n = m_fileList->GetCount();

    // count the number of selected items in the file list
    int ns = 0;
    for (int i = 0; i < n; i++)
    	ns += m_fileList->Selected(i);

    if (ns == 0) return;

    // allocate arrays for selected motions and their respective weights
    Motion **source_motion = new (Motion*)[ns];
    double *weight = new double[ns];

    Joint *skeleton = NULL;

    // create the arrays
    int j = 0;
    for (int i = 0; i < n; i++)
    	if (m_fileList->Selected(i))
    	{
    		source_motion[j] = shots[i]->motion;
    		weight[j] = 1.0 / ns;    // uniform weights
    		j++;
    		skeleton = shots[i]->skeleton;
    	}

    // create a new blended motion
    BlendedMotion *motion = new BlendedMotion("Blended", ns,
        source_motion, weight);

    unsigned int r, g, b;
    r = g = b = 128;

    // create a shot for blended motion, identical skeletons across
    // the source motions are assumed
    Shot *shot = new Shot(motion->name, skeleton, motion, r, g, b);

    // add the blended motion and its corresponding shot
    motions.push_back(motion);
    shots.push_back(shot);

    // add the shot to the file list panel
    m_fileList->Append(motion->name.c_str());
    int i = m_fileList->GetCount() - 1;
    m_fileList->GetItem(i)->SetTextColour(wxColour(r, g, b));
    m_fileList->Check(i);

    // redisplay the scene
    try
    {
        glCanvas->DrawCheckedShots(m_fileList);
    }
    // incompatible motions, the exception rised in evaluating of tridofs of
    // a recorded motion which is the blended motion composed of
    catch (out_of_range &oor)
    {
        m_fileList->Delete(i);
        shots.pop_back();
        motions.pop_back();
        delete shot;
        delete motion;

        ErrorMessage(_T("Cannot create blended motion, the selected source")
            _T(" motions have not the same number of degrees-of-freedom."));
        return;
    }

    // report info about shot to status bar
    stringstream s;
    s << shot->name << "  / duration " << setprecision(5)
      << shot->motion->duration << " s /";
    SetStatusText(s.str().c_str(), 0);
}


// Creates a new shot with a motion interpolating motion of the first two
// selected shots (weights for the motions linearly varies from (1,0) at
// first frame to (0,1) at last frame).
// Selected shots must have identical skeletons.

void MyFrame::OnInterpolate(wxCommandEvent& event)
{
    int n = m_fileList->GetCount();

    // count the number of selected items in the file list
    int ns = 0;
    for (int i = 0; i < n; i++)
    	ns += m_fileList->Selected(i);

    if (ns < 2) return;

    // allocate arrays for all motions and their respective weights
    Motion **source_motion = new (Motion*)[n];
    double *weight_start = new double[n];
    double *weight_end = new double[n];

    Joint *skeleton = NULL;

    // create the arrays
    int j = 0;
    for (int i = 0; i < n; i++)
    {
  		source_motion[i] = shots[i]->motion;
   		weight_start[i] = 0;
   		weight_end[i] = 0;

    	if (m_fileList->Selected(i) && j < 2)
    	{
      		weight_start[i] = (j == 0);
      		weight_end[i] = (j == 1);
    		j++;
    		skeleton = shots[i]->skeleton;
    	}
    }

    // create a new interpolated motion
    InterpolatedMotion *motion = new InterpolatedMotion("Interpolated", n,
        source_motion, weight_start, weight_end);

    unsigned int r, g, b;
    r = g = b = 224;

    // create a shot for interpolated motion, identical skeletons across
    // the source motions are assumed
    Shot *shot = new Shot(motion->name, skeleton, motion, r, g, b);

    // add the blended motion and its corresponding shot
    motions.push_back(motion);
    shots.push_back(shot);

    // add the shot to the file list panel
    m_fileList->Append(motion->name.c_str());
    int i = m_fileList->GetCount() - 1;
    m_fileList->GetItem(i)->SetTextColour(wxColour(r, g, b));
    m_fileList->Check(i);

    // redisplay the scene
    try
    {
        glCanvas->DrawCheckedShots(m_fileList);
    }
    // incompatible motions, the exception rised in evaluating of tridofs of
    // a recorded motion which is the blended motion composed of
    catch (out_of_range &oor)
    {
        m_fileList->Delete(i);
        shots.pop_back();
        motions.pop_back();
        delete shot;
        delete motion;

        ErrorMessage(_T("Cannot create interpolated motion, the first two")
            _T(" selected source motions have not the same number of")
            _T(" degrees-of-freedom."));
        return;
    }

    // report info about shot to status bar
    stringstream s;
    s << shot->name << "  / duration " << setprecision(5)
      << shot->motion->duration << " s /";
    SetStatusText(s.str().c_str(), 0);
}


// other public MyFrame methods related to exporting of the animation are in
// a separate file export.cpp


///////////////////////////////////////////////////////////////////////////////
//
// class MyFrame - private methods
//
///////////////////////////////////////////////////////////////////////////////

void MyFrame::build_menu()
{
    wxMenu *menuFile = new wxMenu;
    menuFile->Append(ID_OPEN, _T("&Open...\tO"), _T("Open a motion file"));
    menuFile->Append(ID_CLOSE, _T("&Close selected  "), _T("Close selected motion files"));
    menuFile->AppendSeparator();
    menuFile->Append(ID_QUIT, _T("E&xit\tAlt-X"), _T("Quit this program"));

    wxMenu *menuAnim = new wxMenu;
    menuAnim->Append(ID_BACK, _T("Play &backwards\tCtrl-Left"), _T("Play animation backwards"));
    menuAnim->Append(ID_BACK_ONE, _T("B&ack one frame\tLeft"), _T("Play back one frame"));
    menuAnim->Append(ID_PAUSE, _T("&Pause\tSpace"), _T("Pause the animation"));
    menuAnim->Append(ID_FWD_ONE, _T("F&orward one frame\tRight"), _T("Play forward one frame"));
    menuAnim->Append(ID_FWD, _T("Play &forward\tCtrl-Right"), _T("Play animation forward"));
    menuAnim->AppendSeparator();
    menuAnim->Append(ID_RESET_TIME, _T("&Reset time\tT"), _T("Reset time to zero"));
    menuAnim->AppendCheckItem(ID_TOGGLE_CYCLE, _T("&Cycle animation\tC"), _T("Cycle/stop animation after last frame"));
    menuAnim->AppendSeparator();
    menuAnim->Append(ID_BLEND, _T("&Blend selected\tB"), _T("Create a motion blending selected motions"));
    menuAnim->Append(ID_INTERPOLATE, _T("&Interpolate two selected  \tI"), _T("Create a motion interpolating two selected motions"));
    menuAnim->Append(ID_EXPORT, _T("&Export...\tE"), _T("Export animation in various formats"));

    wxMenu *menuView = new wxMenu;
    menuView->AppendCheckItem(ID_TOGGLE_SOLID, _T("&Solid redering\tS"), _T("Switch between solid and wireframe OpenGL rendering"));
    menuView->AppendSeparator();
    menuView->Append(ID_RESET_VIEW, _T("&Reset view\tV"), _T("Set the initial view to the scene"));
    menuView->AppendSeparator();
    menuView->AppendCheckItem(ID_TOGGLE_SHOWFILES, _T("Show opened files"), _T("Show/hide the list of opened files"));
    menuView->AppendCheckItem(ID_TOGGLE_TOOLBAR, _T("Show toolbar"), _T("Show/hide toolbar"));
    menuView->AppendCheckItem(ID_TOGGLE_STATUSBAR, _T("Show statusbar"), _T("Show/hide statusbar"));
    menuView->AppendSeparator();
    menuView->Append(ID_EXPORT_SETTINGS, _T("Settings for export..."), _T("Modify settings for export"));
    //defaults
    menuView->Check(ID_TOGGLE_SHOWFILES, true);
    menuView->Check(ID_TOGGLE_TOOLBAR, true);
    menuView->Check(ID_TOGGLE_STATUSBAR, true);

    wxMenu *menuHelp = new wxMenu;
    menuHelp->Append(ID_ABOUT, _T("&About...  \tF1"), _T("Show dialog with program information"));

    m_menuBar = new wxMenuBar();
    m_menuBar->Append(menuFile, _T("&File"));
    m_menuBar->Append(menuAnim, _T("&Animation"));
    m_menuBar->Append(menuView, _T("&View"));
    m_menuBar->Append(menuHelp, _T("&Help"));

    SetMenuBar(m_menuBar);
    enable_animation_tools(false);
}


void MyFrame::recreate_toolbar()
{
    m_toolBar = CreateToolBar(wxTB_FLAT | wxTB_HORIZONTAL, ID_TOOLBAR);

    wxBitmap toolBarBitmaps[8];
    #ifdef __WXMSW__
        toolBarBitmaps[0] = wxBITMAP(open);
        toolBarBitmaps[1] = wxBITMAP(help);
        toolBarBitmaps[2] = wxBITMAP(back);
        toolBarBitmaps[3] = wxBITMAP(back_one);
        toolBarBitmaps[4] = wxBITMAP(stop);
        toolBarBitmaps[5] = wxBITMAP(fwd_one);
        toolBarBitmaps[6] = wxBITMAP(fwd);
        toolBarBitmaps[7] = wxBITMAP(reset);
    #else
        toolBarBitmaps[0] = wxBitmap(open_xpm);
        toolBarBitmaps[1] = wxBitmap(help_xpm);
        toolBarBitmaps[2] = wxBITMAP(back_xpm);
        toolBarBitmaps[3] = wxBITMAP(back_one_xpm);
        toolBarBitmaps[4] = wxBITMAP(stop_xpm);
        toolBarBitmaps[5] = wxBITMAP(fwd_one_xpm);
        toolBarBitmaps[6] = wxBITMAP(fwd_xpm);
        toolBarBitmaps[7] = wxBITMAP(reset_xpm);
    #endif // __WXMSW__

    m_toolBar->AddTool(ID_OPEN, _T("Open"), toolBarBitmaps[0], _T("Open file"));
    m_toolBar->AddSeparator();

    m_toolBar->AddTool(ID_BACK, _T("Back"), toolBarBitmaps[2], _T("Play backward"));
    m_toolBar->AddTool(ID_BACK_ONE, _T("Back 1 frame"), toolBarBitmaps[3], _T("Back 1 frame"));
    m_toolBar->AddTool(ID_PAUSE, _T("Pause"), toolBarBitmaps[4], _T("Pause the animation"));
    m_toolBar->AddTool(ID_FWD_ONE, _T("Forward 1 frame"), toolBarBitmaps[5], _T("Forward 1 frame"));
    m_toolBar->AddTool(ID_FWD, _T("Forward"), toolBarBitmaps[6], _T("Play forward"));
    m_toolBar->AddSeparator();

    m_toolBar->AddTool(ID_RESET_VIEW, _T("Reset"), toolBarBitmaps[7], _T("Reset view"));
    m_toolBar->AddSeparator();

    m_toolBar->AddTool(ID_ABOUT, _T("Help"), toolBarBitmaps[1], _T("Information about the program"));

    m_toolBar->Realize();
    enable_animation_tools(!shots.empty());
}


// Load all BVH files in an array filename, with a given path.
// Create a skeleton, motion and shot for each loaded animation, add
// it to the scene and enable animation tools.

void MyFrame::open_bvh_files(string path, const wxArrayString& filename)
{
    BvhFile bvh;
    Joint *skeleton;
    RecordedMotion *motion;
    Shot *shot;
    unsigned int i, n, r, g, b;
    stringstream s;

//        bool opened = create_test(&skeleton, &motion);
    
    n = filename.GetCount();

    for (i = 0; i < n; i++)
    {
        SetStatusText(_T("Loading..."));
    
        // load a new skeleton and a recorded motion from a BVH file
        bvh.Assign(string(path + "/" + filename[i].c_str()));

        bool opened = bvh.Load(&skeleton, &motion);

        // error message was already displayed by the Load method
        if (!opened) break;
        
        // create random color
        r = rand() % 256;
        g = rand() % 256;
        b = rand() % 256;

        // create a shot associating loaded skeleton and motion
        shot = new Shot(motion->name, skeleton, motion, r, g, b);

        // add new skeleton, motion and shot
        skeletons.push_back(skeleton);
        motions.push_back(motion);
        shots.push_back(shot);

        // add shot to the file list panel
        m_fileList->Append(motion->name.c_str());
        int k = m_fileList->GetCount() - 1;
        m_fileList->GetItem(k)->SetTextColour(wxColour(r, g, b));
        m_fileList->Check(k);

        // report info about shot to status bar
        s.str("");
        s << filename[i] << "  / duration " << setprecision(5)
          << shot->motion->duration << " s /";
        SetStatusText(s.str().c_str(), 0);

        enable_animation_tools(true);
    }
    
    // redisplay the scene
    glCanvas->DrawCheckedShots(m_fileList);

    if (n > 1)
    {
        s.str("");
        s << n << " files loaded.";
        SetStatusText(s.str().c_str(), 0);
    }
}


// enable/disable animation menu and tools due to en value

void MyFrame::enable_animation_tools(bool en)
{
    // en/dis Animation menu
    m_menuBar->EnableTop(1, en);

    // en/dis animation buttons on toolbar
    if (m_toolBar != NULL) {
        m_toolBar->EnableTool(ID_BACK, en);
        m_toolBar->EnableTool(ID_BACK_ONE, en);
        m_toolBar->EnableTool(ID_PAUSE, en);
        m_toolBar->EnableTool(ID_FWD_ONE, en);
        m_toolBar->EnableTool(ID_FWD, en);
        m_toolBar->EnableTool(ID_RESET_VIEW, en);
    }
}


void MyFrame::recreate_statusbar()
{
    m_statusBar = CreateStatusBar(2);
}


void MyFrame::stop_animation()
{
    SetStatusText(_T("Stop"));
    m_timer->Stop();
}


// show current time in 2-nd field of statusbar

void MyFrame::show_time()
{
    char s[30];
    sprintf(s, "Time: %8.3f s", time);
    SetStatusText(_T(s), 1);
}

// other private MyFrame methods related to exporting of the animation are in
// a separate file export.cpp



///////////////////////////////////////////////////////////////////////////////
//
// class MySplitter - public methods
//
///////////////////////////////////////////////////////////////////////////////

BEGIN_EVENT_TABLE(MySplitter, wxSplitterWindow)
    EVT_SPLITTER_SASH_POS_CHANGED(-1, MySplitter::OnPositionChanged)
END_EVENT_TABLE()

MySplitter::MySplitter(wxWindow *parent)
    : wxSplitterWindow(parent, -1, wxDefaultPosition, wxDefaultSize,
    wxCLIP_CHILDREN)
{
    m_position = 100;
    SetMinimumPaneSize(20);
}


bool MySplitter::SplitVertically(wxWindow* win1, wxWindow* win2)
{
    return wxSplitterWindow::SplitVertically(win1, win2, m_position);
}


void MySplitter::OnPositionChanged(wxSplitterEvent& event)
{
    m_position = GetSashPosition();
//    event.Skip();
}



/////////////////////////////////////////////////////////////////////////////
//
// class ExpSetDialog - settings for export dialog
//
/////////////////////////////////////////////////////////////////////////////

ExpSetDialog::ExpSetDialog(wxWindow *parent)
    : wxDialog(parent, -1, "Settings for export", wxDefaultPosition,
               wxSize(308,240), wxDEFAULT_DIALOG_STYLE | wxRESIZE_BORDER )
{
    m_okB = new wxButton(this, wxID_OK, "OK", wxPoint(134, 183));
    m_cancelB = new wxButton(this, wxID_CANCEL, "Cancel", wxPoint(218, 183));

    m_csvSB = new wxStaticBox(this, -1, _T("To CSV format "), wxPoint(7,8), wxSize(288,52), wxSTATIC_BORDER);
    m_jointsST = new wxStaticText(this, -1, _T("Joints"), wxPoint(17,30));
    m_jointsTC = new wxTextCtrl(this, -1, _T(""), wxPoint(49,28), wxSize(105,20), 0,
                                wxTextValidator(wxFILTER_ASCII, &mainFrame->joints_to_csv_wxstr));
    m_coordsST = new wxStaticText(this, -1, _T("Coordinates"), wxPoint(175,30));
    m_coordsTC = new wxTextCtrl(this, -1, _T(""), wxPoint(238,28), wxSize(43,20), 0,
                                wxTextValidator(wxFILTER_ASCII, (wxString*)&mainFrame->coords_to_csv_wxstr));

    m_batSB = new wxStaticBox(this, -1, _T("To batch file "), wxPoint(7,68), wxSize(288,104), wxSTATIC_BORDER);
    m_ltoesST = new wxStaticText(this, -1, _T("Left toes name"), wxPoint(24,90));
    m_ltoesTC = new wxTextCtrl(this, -1, _T(""), wxPoint(100,88), wxSize(49,20), 0,
                                wxTextValidator(wxFILTER_ASCII, (wxString*)&mainFrame->ltoes_name_wxstr));
    m_rtoesST = new wxStaticText(this, -1, _T("Right toes name"), wxPoint(17,116));
    m_rtoesTC = new wxTextCtrl(this, -1, _T(""), wxPoint(100,114), wxSize(49,20), 0,
                                wxTextValidator(wxFILTER_ASCII, (wxString*)&mainFrame->rtoes_name_wxstr));
    m_hlimitST = new wxStaticText(this, -1, _T("Height limit"), wxPoint(173,90));
    m_hlimitTC = new wxTextCtrl(this, -1, _T(""), wxPoint(231,88), wxSize(49,20), 0,
                                wxTextValidator(wxFILTER_NUMERIC, &mainFrame->height_limit_wxstr));
    m_vlimitST = new wxStaticText(this, -1, _T("Velocity limit"), wxPoint(166,116));
    m_vlimitTC = new wxTextCtrl(this, -1, _T(""), wxPoint(231,114), wxSize(49,20), 0,
                                wxTextValidator(wxFILTER_NUMERIC, &mainFrame->velocity_limit_wxstr));
    m_vdeltaST = new wxStaticText(this, -1, _T("Velocity delta"), wxPoint(161,142));
    m_vdeltaTC = new wxTextCtrl(this, -1, _T(""), wxPoint(231,140), wxSize(49,20), 0,
                                wxTextValidator(wxFILTER_NUMERIC, &mainFrame->velocity_delta_wxstr));
}


ExpSetDialog::~ExpSetDialog()
{
    delete m_vdeltaTC;
    delete m_vdeltaST;
    delete m_vlimitTC;
    delete m_vlimitST;
    delete m_hlimitTC;
    delete m_hlimitST;
    delete m_rtoesTC;
    delete m_rtoesST;
    delete m_ltoesTC;
    delete m_ltoesST;
    delete m_batSB;

    delete m_coordsTC;
    delete m_coordsST;
    delete m_jointsTC;
    delete m_jointsST;
    delete m_csvSB;

    delete m_cancelB;
    delete m_okB;
}



/////////////////////////////////////////////////////////////////////////////
//
// class AboutDialog - about dialog
//
/////////////////////////////////////////////////////////////////////////////

AboutDialog::AboutDialog(wxWindow *parent)
    : wxDialog(parent, -1, "About WX Motion Viewer", wxDefaultPosition,
               wxSize(260,175), wxDEFAULT_DIALOG_STYLE)// | wxRESIZE_BORDER )
{
    m_dialogSizer = new wxBoxSizer(wxVERTICAL);

    // icon, title and author area
    m_titleSizer = new wxBoxSizer(wxHORIZONTAL);
    m_iconBitmap = new wxStaticBitmap(this, -1, wxICON(dancer), wxPoint(0, 0), wxSize(32, 32));
    m_titleText = new wxStaticText(this, -1, _T("WX Motion Viewer\n\nby Jaroslav Semancik, 2004"));
    m_titleSizer->Add(m_iconBitmap, 0, wxTOP | wxLEFT | wxRIGHT, 20);
    m_titleSizer->Add(m_titleText, 1, wxTOP, 15);

    // program description area
    m_descSizer = new wxBoxSizer(wxHORIZONTAL);
    m_descText = new wxStaticText(this, -1,
        _T("A viewer of .bvh motion files. They can be played,\n")
        _T("blended, interpolated and exported in several\n")
        _T("formats for rendering, analysis and editing."), wxDefaultPosition, wxDefaultSize, wxALIGN_CENTER);
    m_descSizer->Add(m_descText, 1, wxTOP, 10);

    // OK button
    m_buttonSizer = new wxBoxSizer(wxHORIZONTAL);
    m_okButton = new wxButton(this, wxID_OK, "OK");
    m_buttonSizer->Add(m_okButton, 1, wxLEFT | wxRIGHT, 80);

    // vertical list of elements above
    m_dialogSizer->Add(m_titleSizer, 0, wxGROW);
    m_dialogSizer->Add(m_descSizer, 0, wxGROW);
    m_dialogSizer->Add(0, 10, 1);
    m_dialogSizer->Add(m_buttonSizer, 0, wxGROW);
    m_dialogSizer->Add(0, 10, 0);
    SetSizer(m_dialogSizer);
    SetAutoLayout(true);
    Layout();
}


AboutDialog::~AboutDialog()
{
    delete m_okButton;
    delete m_buttonSizer;
    delete m_descText;
    delete m_descSizer;
    delete m_titleText;
    delete m_iconBitmap;
    delete m_titleSizer;
    delete m_dialogSizer;
}

