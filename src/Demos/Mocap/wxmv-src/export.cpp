///////////////////////////////////////////////////////////////////////////////
//
// export.cpp
//
// Purpose:   Implementation of main frame MyFrame methods related to
//            exporting of animation.
//
// Created:   Jaroslav Semancik, 27/10/2003
//
///////////////////////////////////////////////////////////////////////////////

#include <wx/wx.h>
#include <wx/glcanvas.h>
#include <wx/image.h>
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
#include <io.h>

using namespace std;

#include "base.h"
#include "algebra.h"
#include "motion.h"
#include "skeleton.h"
#include "bvh.h"
#include "ogl.h"
#include "wxmv.h"


///////////////////////////////////////////////////////////////////////////////
//
// MyFrame - public methods related to export
//
///////////////////////////////////////////////////////////////////////////////

// Opens a common file dialog to choose a filename to export to.
// Then calls a proper export function according to selected format
// determined from filename extension or a format filter.

void MyFrame::OnExport(wxCommandEvent& event)
{
    // choose output file by a common dialog
    wxFileDialog *dlg = new wxFileDialog(this, _T("Export to file..."),
        _T(""), _T(".bat"),
        _T("Batch files (*.bat)|*.bat|"
           "Comma-separated value files (*.csv)|*.csv|"
           "POV-Ray include files (*.inc)|*.inc|"
           "All files (*.*)|*.*"),
        wxSAVE | wxOVERWRITE_PROMPT );

    if (dlg->ShowModal() != wxID_OK)
    {
        dlg->Destroy();
        return;
    }

    m_export_path = string(dlg->GetDirectory());
    string filename = string(dlg->GetFilename());
    string ext = StripExtension(filename);
    int fi = dlg->GetFilterIndex();
    dlg->Destroy();

    // choose export format according to filename extension
    if (ext == "inc" || ext == "pov")
        export_to_pov(filename);
    else if (ext == "csv")
        export_to_csv(filename);
    else if (ext =="bat")
        export_to_bat(filename);

    else    // choose export format according to selected format filter
        switch (fi)
        {
            case 0: export_to_pov(filename);
                    break;
                    
            case 1: export_to_csv(filename);
                    break;
            
            case 2: export_to_bat(filename);
                    break;

            default: ErrorMessage("No export format was selected!");
        }
}


// Handles a dialog with settings for export.

void MyFrame::OnExportSettings(wxCommandEvent& event)
{
    // convert params to wxStrings used in dialog validators
    joints_to_csv_wxstr = joints_to_csv.c_str();
    coords_to_csv_wxstr = coords_to_csv.c_str();
    ltoes_name_wxstr = ltoes_name.c_str();
    rtoes_name_wxstr = rtoes_name.c_str();

    height_limit_wxstr.Printf("%g", height_limit);
    velocity_limit_wxstr.Printf("%g", velocity_limit);
    velocity_delta_wxstr.Printf("%g", velocity_delta);

    ExpSetDialog *dlg = new ExpSetDialog(this);

    if (dlg->ShowModal() == wxID_OK)
    {
        // add space to the end of exported joints string
        if (joints_to_csv_wxstr.Last() != ' ')
            joints_to_csv_wxstr += " ";

        // read params back from wxStrings used in dialog validators
        joints_to_csv = joints_to_csv_wxstr.c_str();
        coords_to_csv = coords_to_csv_wxstr.c_str();
        ltoes_name = ltoes_name_wxstr.c_str();
        rtoes_name = rtoes_name_wxstr.c_str();

        height_limit_wxstr.ToDouble(&height_limit);
        velocity_limit_wxstr.ToDouble(&velocity_limit);
        velocity_delta_wxstr.ToDouble(&velocity_delta);
    }

    dlg->Destroy();
}


///////////////////////////////////////////////////////////////////////////////
//
// MyFrame - private methods related to export
//
///////////////////////////////////////////////////////////////////////////////

// Main function for export to POV-Ray. It creates all necessary directories
// and files and exports all frames in a loop. The given filename is assumed
// to be a name of the main .inc file. 

void MyFrame::export_to_pov(const string& filename)
{
    // find a maximal duration of any checked shot
    m_max_dur = 0;
    int n = m_fileList->GetCount();
    for (int i = 0; i < n; i++)
    	if (m_fileList->IsChecked(i) && shots[i]->motion->duration > m_max_dur)
    	   m_max_dur = shots[i]->motion->duration;

    // set filenames
    m_inc_name = filename + ".inc";
    m_ini_name = filename + ".ini";
    m_pov_name = filename + ".pov";

    // create directories for exported files
    create_pov_directories();

    // create .inc file including an .inc file for particular shots and frames
    if (!create_pov_inc_file()) return;

    // create POV-Ray .ini file with animation settings
    if (!create_pov_ini_file()) return;

    SetStatusText(_T("Exporting animation to POV-Ray..."));

    // export all frames
    m_export_time = 0.0;
    m_frame_number = 0;

    while (m_export_time <= m_max_dur)
    {
        if (!export_frame_to_pov()) break;
        m_export_time += frametime;
        m_frame_number++;
    }

    SetStatusText(_T("Export to POV-Ray finished."));
}


// Creates directories for files exported to POV-Ray. A directory for each
// shot is created to contain all its frame .inc files. Moreover an ./out
// directory is created for output images rendered by POV-Ray.
// Does not check whether directories have been successfully created.

void MyFrame::create_pov_directories()
{
    string dirname;

    // create a directory for frame .inc files of each checked shot
    int n = m_fileList->GetCount();
    for (int i = 0; i < n; i++)
    {
        if (!m_fileList->IsChecked(i)) continue;

        dirname = m_export_path + "/" + shots[i]->name;
        mkdir(dirname.c_str());
    }

    // create ./out directory for POV-Ray output images
    dirname = m_export_path + "/out";
    mkdir(dirname.c_str());
    return;
}



// Create a main .inc file to be included from POV-Ray scene. This file
// contains a POV-Ray blob object for each checked shot. Inside the blob
// a particular .inc file is included containing blob components for
// a skeleton posture in just rendered frame.
// Returns whether the file has been successfully created.

bool MyFrame::create_pov_inc_file()
{
    string filename = m_export_path + "/" + m_inc_name;
    ofstream file;

    // open file for write
    file.open(filename.c_str());
    if (!file)
    {
    	stringstream error;
        error << "Unable to create file " << filename
              << ". Disk full or write protection?";
        ErrorMessage(error.str());
        return false;
    }

    // write initial comments
    file << "//" << endl
         << "// " << m_inc_name << " - main include file with skeletons as a POV-Ray blobs" << endl
         << "//" << endl
         << "// Use in your scene by: " << endl
         << "//" << endl
         << "//     #include \"" << m_inc_name << "\"" << endl
         << "//" << endl
         << "// and set animation .ini file for POV-Ray rendering to " << m_ini_name << endl
         << "//" << endl
         << "// Generated by Motion Viewer" << endl
         << "//" << endl
         << "//////////////////////////////////////////////////////////////////////" << endl
         << endl;

    // write adjustable constants
    file << "#declare th = 1.5;    // thickness of bones" << endl
         << "#declare jnt = -0.9;  // blob adjustment in joints" << endl;

    // write a blob object for each checked shot
    int n = m_fileList->GetCount();
    for (int i = 0; i < n; i++)
    {
    	if (!m_fileList->IsChecked(i)) continue;

		// color components to float range <0,1>
        float r = (float)shots[i]->r / 255;
	    float g = (float)shots[i]->g / 255;
	    float b = (float)shots[i]->b / 255;

        // number of frames for this shot and its width
	    int nf = (int)ROUND(shots[i]->motion->duration / frametime) + 1;
        int w = (int)log10((float)nf) + 1;

        // write blob
        file << endl << endl
             << "// " << shots[i]->name << " shot" << endl << endl
             << "blob {" << endl
             << "    threshold .5" << endl;

        if (Motion::wrap)
            file << endl << "    #declare fn = mod(frame_number, " << nf
                   << ");" << endl;
        else
            file << "    #declare fn = (frame_number < " << nf
                   << " ? frame_number : " << nf - 1 << ");" << endl;

        file << "    #declare incname = " << "concat(\"" << shots[i]->name
               << "/" << shots[i]->name << "\", str(fn, " << -w
               << ", 0), \".inc\");" << endl
             << "    #include incname" << endl
             << endl
             << "    texture {               // YOU MAY CHANGE MATERIAL HERE"
               << endl
             << "        pigment { color < "
               << setprecision(3) << r << ", " << g << ", " << b << " > }"
               << endl
             << "        finish { specular 0.9  roughness 0.01 }" << endl
             << "    }" << endl
             << "}" << endl;
    }

    file.close();
    return true;
}



// Create a POV-Ray .ini file with animation settings.
// Returns whether the file has been successfully created.

bool MyFrame::create_pov_ini_file()
{
    string filename = m_export_path + "/" + m_ini_name;
    ofstream file;

    // open file for write
    file.open(filename.c_str());
    if (!file)
    {
    	stringstream error;
        error << "Unable to create file " << filename
              << ". Disk full or write protection?";
        ErrorMessage(error.str());
        return false;
    }

    // write initial comments
    file << ";" << endl
         << "; " << m_ini_name << " - POV-Ray animation settings" << endl
         << "; (framerate: " << 1 / frametime << " fps)" << endl
         << ";" << endl
         << "; For settings description see POV-Ray documention" << endl
         << ";" << endl
         << "; Generated by Motion Viewer" << endl
         << ";" << endl
         << ";;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;" << endl
         << endl;

    // max. number of frames
    int max_nf = (int)ROUND(m_max_dur / frametime) + 1;

    // write animation settings
    file << "Input_File_Name=" << m_pov_name << endl
         << "Output_File_Name=./out/" << endl
         << endl
         << endl
         << "; animation settings" << endl
         << endl
         << "Initial_Frame=" << 0 << endl
         << "Final_Frame=" << max_nf - 1 << endl
         << "Cyclic_Animation=off" << endl
         << endl
         << endl
         << "; uncomment and set the following to render a particular frame(s)" << endl
         << endl
         << "; Subset_Start_Frame=" << endl
         << "; Subset_End_Frame=" << endl
         << endl
         << endl
         << "; output size" << endl
         << endl
         << "Width=320" << endl
         << "Height=240" << endl
         << endl
         << endl
         << ";antialiasing" << endl
         << endl
         << "Antialias=on         ; turn off for test renderings" << endl
         << "Sampling_Method=2" << endl
         << "Antialias_Depth=3" << endl
         << "Jitter=off" << endl;

    file.close();
    return true;
}


// Store a pose of the skeleton in each checked shot as a set of of POV-Ray
// blob components. The pose is evaluated for frame m_frame_number with time
// m_export_time and stored to a separate .inc file for each checked shot.
// Return whether the frame has been successfully exported.

bool MyFrame::export_frame_to_pov()
{
    // create an .inc file for each checked shot
    int n = m_fileList->GetCount();
    for (int i = 0; i < n; i++)
    {
    	if (!m_fileList->IsChecked(i)) continue;
    	if (m_export_time > shots[i]->motion->duration) continue;

        // create file name for current frame
        string filename = frame_filename(shots[i], m_frame_number, ".inc");
        filename = m_export_path + "/" + shots[i]->name + "/" + filename;
        ofstream file;

        // open file for write
        file.open(filename.c_str());
        if (!file)
        {
            stringstream error;
            error << "Unable to create file " << filename
                  << ". Disk full or write protection?";
            ErrorMessage(error.str());

            return false;
        }

        // write header comments
        write_pov_frame_header(file, shots[i], m_frame_number);

        // write segments of a skeleton in the shot
        shots[i]->ExportToPov(file, m_export_time);

        file.close();
    }

    return true;
}


// Returns file name with extension ext for a given shot and frame number.

string MyFrame::frame_filename(const Shot* shot, int fn, const string& ext)
{
    stringstream filename;

    int w = (int)log10(ROUND(shot->motion->duration / frametime) + 1) + 1;
    filename << shot->name << setw(w) << setfill('0') << fn << ext;

    return filename.str();
}


// Writes comments in the beginning of each frame .inc file.

void MyFrame::write_pov_frame_header(ofstream& file, const Shot* shot, int fn)
{
    file << "//" << endl
         << "// " << shot->name << ": frame " << fn
            << ",  blob components included from " << m_inc_name << endl
         << "//" << endl
         << "// Generated by Motion Viewer" << endl
         << "//" << endl
         << "//////////////////////////////////////////////////////////" << endl
         << endl;
}



// Main function for export to a CSV file with given filename.
// Exports all shots and frames to one file.

void MyFrame::export_to_csv(const string& filename)
{
    // find a maximal duration of any checked shot
    m_max_dur = 0;
    int n = m_fileList->GetCount();
    for (int i = 0; i < n; i++)
    	if (m_fileList->IsChecked(i) && shots[i]->motion->duration > m_max_dur)
    	   m_max_dur = shots[i]->motion->duration;

    // create a .csv file with header line - names of degress-of-freedom
    m_csv_name = filename + ".csv";
    if (!open_csv_file()) return;

    SetStatusText(_T("Exporting animation to a CSV file..."));

    // export all frames
    m_export_time = 0.0;
    m_frame_number = 0;

    while (m_export_time <= m_max_dur)
    {
        export_frame_to_csv();
        m_export_time += frametime;
        m_frame_number++;
    }

    m_csv_file.close();
    SetStatusText(_T("Export to CSV finished."));
}


// Create a new CSV file and write names of coordinates of chosen joints
// in all checked shots.

bool MyFrame::open_csv_file()
{
    m_csv_file.clear();
    string filename = m_export_path + "/" + m_csv_name;
    m_csv_file.open(filename.c_str());

    if (!m_csv_file)
    {
        stringstream error;
        error << "Unable to create file " << filename
              << ". Disk full or write protection?";
        ErrorMessage(error.str());

        return false;
    }

    // 1st column will be the frame number
    m_csv_file << "Frame,";

    // write coordinates of chosen joints in all checked shots
    int n = m_fileList->GetCount();
    for (int i = 0; i < n; i++)
    {
    	if (m_fileList->IsChecked(i))
            shots[i]->HeaderToCsv(m_csv_file);
    }
    
    m_csv_file << endl;
    return true;
}


// Store world coordinates of chosen joints in all checked shots to one line
// in a .csv file. Joint's coordinates are evaluated for time m_export_time
// and stored as frame m_frame_number.

void MyFrame::export_frame_to_csv()
{
    // write the current frame number
    m_csv_file << m_frame_number << ",";

    // write chosen joint coordinates in all checked shots to one line
    // in the .csv file
    int n = m_fileList->GetCount();
    for (int i = 0; i < n; i++)
    {
    	if (m_fileList->IsChecked(i))
            shots[i]->ExportToCsv(m_csv_file, m_export_time);
    }

    m_csv_file << endl;
}



// Main function for export of a cutting script to a batch file.

void MyFrame::export_to_bat(const string& filename)
{
    string bat_name = m_export_path + "/" + filename + ".bat";
    ofstream bat_file(bat_name.c_str());

    SetStatusText(_T("Exporting animation to a batch file..."));

    int n = m_fileList->GetCount();

    // write initial comments
    bat_file << "REM A script cutting animation(s) ";
    for (int i = 0; i < n; i++)
    	if (m_fileList->IsChecked(i))
            bat_file << shots[i]->name << " ";
    bat_file << "to individual steps." << endl;
    bat_file << "REM Generated by Motion Viewer" << endl;

    // export all checked shots
    for (int i = 0; i < n; i++)
    	if (m_fileList->IsChecked(i))
            shots[i]->ExportToBat(bat_file);

    SetStatusText(_T("Export to batch file finished."));

    bat_file.close();
}

