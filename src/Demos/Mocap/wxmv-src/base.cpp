///////////////////////////////////////////////////////////////////////////////
//
// base.cpp
//
// Purpose:   Implementation of fundamental and support functions
//            (maybe in a platform-dependent way)
//
// Created:   Jaroslav Semancik, 8/10/2003
//
///////////////////////////////////////////////////////////////////////////////

#include <wx/wx.h>
#include <wx/image.h>
#include <wx/splitter.h>

#include <string>
#include <sstream>
#include <fstream>
#include <iostream>
#include <stdexcept>
#include <vector>

using namespace std;

#include "base.h"
#include "algebra.h"
#include "motion.h"
#include "skeleton.h"
#include "wxmv.h"


// Function for reporting of errors.
// Implemented as simple console output or a message box under wxWindows.

void ErrorMessage(const string& msg)
{
    // wxWindows message box
    wxMessageBox(msg.c_str(), _T("Error"), wxOK | wxICON_ERROR, mainFrame);

    // simple output to console
///    cerr << endl << msg << endl;
}


// Strips and returns extension from filename. Extension is considered as
// a smallest suffix starting by period.

string StripExtension(string& filename)
{
    unsigned int n = filename.rfind(".", filename.length() - 1);

    if (n == string::npos)
        return "";

    string ext = filename.substr(n + 1);    
    filename.erase(n);
    return ext;
}

