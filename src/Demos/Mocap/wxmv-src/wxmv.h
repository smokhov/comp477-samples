///////////////////////////////////////////////////////////////////////////////
//
// wxmv.h
//
// Purpose:   Declarations for wxWindows user interface of WX Motion
//            Viewer.
//            Classes: MyApp, MyFrame, MySplitter, ExpSetDialog, AboutDialog.
//
// Created:   Jaroslav Semancik, 12/12/2003
//
///////////////////////////////////////////////////////////////////////////////


///////////////////////////////////////////////////////////////////////////////
//
// class MyApp
//
// Main application class executed by wxWindows.
//
///////////////////////////////////////////////////////////////////////////////

class MyApp : public wxApp
{
public:
    virtual bool OnInit();
};



///////////////////////////////////////////////////////////////////////////////
//
// class MyFrame
//
// Class for main frame (window) of the application. It contains all controls
// (e.g. menu, toolbar, statusbar) except the OpenGL canvas. OpenGL canvas is
// a global instance to be accesible by all classes. Most of the user
// interface funcionality is provided by MyFrame methods.
//
///////////////////////////////////////////////////////////////////////////////

class MySplitter;   // forward reference for use in MyFrame

class MyFrame : public wxFrame
{
public:

    bool fwd_playing;   // forward or backward playing
    double time;        // current time
    double frametime;   // time difference between two succesive frames

    // Names of joints and coordinates (x,y,z) to export to a CSV file.
    // Joint names are terminated by spaces (note that space is needed also
    // behind the last one!)
    // All joints are exported in case of empty string with joint names.
    // A special coordinate value v stands for velocity of the joint.
    string joints_to_csv;
    string coords_to_csv;

    // For export to a batch file.
    string ltoes_name;      // name of a left toes end-effector
    string rtoes_name;      // name of a right toes end-effector
    double velocity_delta;  // for joint velocity calc. by fwd. difference
    double velocity_limit;  // vel. limit for foot to be assumed as planted
    double height_limit;    // height limit for foot to be assumed as planted

    // string versions of the above params. used to r/w from/to settings dialog
    wxString joints_to_csv_wxstr;
    wxString coords_to_csv_wxstr;
    wxString ltoes_name_wxstr;
    wxString rtoes_name_wxstr;
    wxString velocity_delta_wxstr;
    wxString velocity_limit_wxstr;
    wxString height_limit_wxstr;


    MyFrame(const wxString& title, const wxPoint& pos, const wxSize& size);

    // event handlers
    void OnOpen(wxCommandEvent& event);
    void OnClose(wxCommandEvent& event);
    void OnQuit(wxCommandEvent& event);
    void OnBack(wxCommandEvent& event);
    void OnBackOne(wxCommandEvent& event);
    void OnPause(wxCommandEvent& event);
    void OnFwd(wxCommandEvent& event);
    void OnFwdOne(wxCommandEvent& event);
    void OnResetTime(wxCommandEvent& event);
    void OnResetView(wxCommandEvent& event);
    void OnToggleCycle(wxCommandEvent& event);
    void OnToggleSolid(wxCommandEvent& event);
    void OnToggleShowFiles(wxCommandEvent& event);
    void OnToggleShotCheckbox(wxCommandEvent& event);
    void OnToggleToolbar(wxCommandEvent& event);
    void OnToggleStatusbar(wxCommandEvent& event);
    void OnBlend(wxCommandEvent& event);
    void OnInterpolate(wxCommandEvent& event);
    void OnExport(wxCommandEvent& event);
    void OnExportSettings(wxCommandEvent& event);
    void OnAbout(wxCommandEvent& event);

private:

    // controls in the frame
    wxMenuBar *m_menuBar;
    wxToolBar *m_toolBar;
    wxStatusBar *m_statusBar;
    MySplitter *m_splitter;
    wxCheckListBox *m_fileList;
    wxTimer *m_timer;

    // private attributes for animation export
    string m_export_path;
    string m_inc_name, m_ini_name, m_pov_name;
    string m_csv_name;
    ofstream m_csv_file;
    double m_max_dur, m_export_time;
    int m_frame_number;

    // GUI methods
    void build_menu();
    void recreate_toolbar();
    void recreate_statusbar();
    void open_bvh_files(string path, const wxArrayString& filename);
    void enable_animation_tools(bool en);
    void stop_animation();
    void show_time();

    // export to POV-Ray
    void export_to_pov(const string& filename);
    void create_pov_directories();
    bool create_pov_inc_file();
    bool create_pov_ini_file();
    bool export_frame_to_pov();
    string frame_filename(const Shot* shot, int fn, const string& ext);
    void write_pov_frame_header(ofstream& file, const Shot* shot, int fn);

    // export to CSV
    void export_to_csv(const string& filename);
    bool open_csv_file();
    void export_frame_to_csv();

    // export to batch
    void export_to_bat(const string& filename);

    DECLARE_EVENT_TABLE()
};



///////////////////////////////////////////////////////////////////////////////
//
// class MySplitter
//
// A class for sash vertically splitting main frame to a file list window and
// an OpenGL canvas window. The frame can be unsplit and split again and the
// class MySplitter remembers sash position.
//
///////////////////////////////////////////////////////////////////////////////

class MySplitter : public wxSplitterWindow
{
public:

    MySplitter(wxWindow *parent);
    bool SplitVertically(wxWindow* win1, wxWindow* win2);

    // event handlers
    void OnPositionChanged(wxSplitterEvent& event);

private:

    int m_position;

    DECLARE_EVENT_TABLE()
};



///////////////////////////////////////////////////////////////////////////////
//
// class ExpSetDialog
//
// A class for the "Settings for export" dialog. When OK button is pressed
// the settings are copied from dialog to main frame attributes.
//
///////////////////////////////////////////////////////////////////////////////

class ExpSetDialog : public wxDialog
{
public:
    ExpSetDialog(wxWindow *parent);
    virtual ~ExpSetDialog();

private:
    // dialog controls
    wxStaticBox *m_csvSB, *m_batSB;
    wxStaticText *m_jointsST, *m_coordsST, *m_ltoesST, *m_rtoesST,
                 *m_hlimitST, *m_vlimitST, *m_vdeltaST;
    wxTextCtrl *m_jointsTC, *m_coordsTC, *m_ltoesTC, *m_rtoesTC,
                 *m_hlimitTC, *m_vlimitTC, *m_vdeltaTC;
    wxButton *m_okB, *m_cancelB;
};



///////////////////////////////////////////////////////////////////////////////
//
// class AboutDialog
//
// A class for the "About application" dialog. Creates an info dialog, sizers
// are used to smartly position the dialog content.
//
///////////////////////////////////////////////////////////////////////////////

class AboutDialog : public wxDialog
{
public:
    AboutDialog(wxWindow *parent);
    virtual ~AboutDialog();

private:
    wxBoxSizer *m_dialogSizer, *m_titleSizer, *m_descSizer, *m_buttonSizer;
    wxStaticBitmap *m_iconBitmap;
    wxStaticText *m_titleText, *m_descText;
    wxButton *m_okButton;
};



///////////////////////////////////////////////////////////////////////////
//
// event IDs
//
///////////////////////////////////////////////////////////////////////////

enum
{
    // events
    ID_OPEN = wxID_OPEN,
    ID_CLOSE = wxID_CLOSE,
    ID_QUIT = wxID_EXIT,
    ID_ABOUT = wxID_ABOUT,

    ID_BACK,
    ID_BACK_ONE,
    ID_PAUSE,
    ID_FWD_ONE,
    ID_FWD,
    ID_RESET_TIME,
    ID_TOGGLE_CYCLE,
    ID_BLEND,
    ID_INTERPOLATE,
    ID_EXPORT,

    ID_TOGGLE_SOLID,
    ID_RESET_VIEW,
    ID_TOGGLE_SHOWFILES,
    ID_TOGGLE_TOOLBAR,
    ID_TOGGLE_STATUSBAR,
    ID_EXPORT_SETTINGS,

    ID_TOOLBAR,
    ID_FILELIST,

    ID_TIMER_FWD,
    ID_TIMER_BACK
};


///////////////////////////////////////////////////////////////////////////
//
// Global variables, to be accessible by all classes.
//
///////////////////////////////////////////////////////////////////////////

extern "C++" MyFrame *mainFrame;

