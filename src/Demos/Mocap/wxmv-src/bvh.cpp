///////////////////////////////////////////////////////////////////////////////
//
// bvh.cpp
//
// Purpose:   Implementation of classes for Biovision Hierarchy (BVH) file
//            format loading.
//            Classes: BvhFile.
//
// Created:   Jaroslav Semancik, 13/10/2003
//
///////////////////////////////////////////////////////////////////////////////

// #define DEBUG

#include <string>
#include <sstream>
#include <iostream>
#include <fstream>
#include <stdexcept>
#include <list>
#include <vector>

using namespace std;

#include "base.h"
#include "algebra.h"
#include "motion.h"
#include "skeleton.h"
#include "bvh.h"

///////////////////////////////////////////////////////////////////////////////
//
// Used BVH file format gramatics:
//
// bvh         -> hierarchy motion end-of-file
// hierarchy   -> "HIERARCHY" root
// root        -> "ROOT" name { joint-data }
// joint       -> "JOINT" name { joint-data }
// joint-data  -> offset [channels] (joint/end-site)+
// end-site    -> "End" "Site" { offset }
// offset      -> "OFFSET" double double double
// channels    -> "CHANNELS" int (Xposition/Yposition/Zposition/Xrotation/Yrotation/Zrotation)*
// motion      -> "MOTION" "Frames:" int "Frame" "Time:" double motion-data
// motion-data -> (double)+
//
// where
//
// [item]   means an optional item
// "token"  means token (without quotes) in the input file
// foo/bar  means either foo or bar
// (item)*  means any number (including zero) of items
// (item)+  means at least one item
// { }      means { or } in the input file
//
///////////////////////////////////////////////////////////////////////////////


///////////////////////////////////////////////////////////////////////////////
//
// class BvhFile - public methods
//
///////////////////////////////////////////////////////////////////////////////

// Loads an animation from the file. Creates a skeleton hierarchy of
// joints and a recorded motion consisting of all degrees-of-freedom.
// Pointers to the newly created skeleton and the motion are stored to
// parameters' destination. Reports a message if an error occured and returns
// a bool if the file has been loaded successfully.

bool BvhFile::Load(Joint** skeleton, RecordedMotion** motion) throw()
{
    bool ok = true;

#ifdef DEBUG
    cerr << "Loading BVH file " << filename << "..." << endl;
#endif

    // open file
    file.clear();
    file.open(filename.c_str());
    if (!file)
    {
        stringstream error;
        error << "Unable to open file " << filename;
        ErrorMessage(error.str());

        *skeleton = NULL;
        *motion = NULL;

        return false;
    }

    m_skeleton = NULL;
    m_motion = NULL;

    tridof_names.clear();
    
    // parse
    line = 1;
    try {
        parse_bvh();
    }

    // delete created structures and show message if a parse error occured
    catch (parse_error &pe)
    {
        delete m_skeleton;
        delete m_motion;
        m_skeleton = NULL;
        m_motion = NULL;

        stringstream error;
        error << "Parse error at " << filename << ':' << line << ": "
            << pe.what();
        ErrorMessage(error.str());

        ok = false;
    }

    // store pointers to created structures
    *skeleton = m_skeleton;
    *motion = m_motion;

#ifdef DEBUG
    if (ok) cerr << "...OK (BVH file loaded)" << endl;
#endif

    // close file
    file.close();
    return ok;
}



///////////////////////////////////////////////////////////////////////////////
//
// class BvhFile - private methods
//
///////////////////////////////////////////////////////////////////////////////

// Read one token from file, skiping all whitespace (' ','\t','\n'),
// "end-of-file" token is returned at the end of file.

void BvhFile::read_next_token() throw(parse_error)
{
    char c;

    // skip leading whitespace while counting lines
    while (file.get(c) && strchr(" \t\n", c))
        if (c == '\n') line++;

    // test end of file
    if (!file)
    {
        token = "end-of-file";
        return;
    }

    file.unget();
    file >> token;
}


// Test if current token is a specified token.

bool BvhFile::token_is(const string& tok) throw()
{
    return (token == tok);
}


// If current token is the specified one, read next token else throw
// parse error exception about misplaced token.

void BvhFile::take_token(const string& tok) throw(parse_error)
{
    if (token_is(tok)) read_next_token();
    else
    {
        stringstream error;
        if (tok != "end-of-file") error << "Token \"" << tok << '"';
        else error << "end of file";
        error << " expected, but \"" << token << "\" found instead";

        throw parse_error(error.str());
    }
}


// Store current token to a string variable and reads next token.

void BvhFile::token_to_string(string& str) throw(parse_error)
{
    str = token;
    read_next_token();
}


// Convert current token to double and read next token. If conversion failed,
// throw parse error exception about it.

void BvhFile::token_to_double(double& num) throw(parse_error)
{
    char *c;

    errno = 0;
    num = strtod(token.c_str(), &c);

    if (errno || c != token.c_str() + token.length())
    {
        stringstream error;
        error << "\"" << token << "\" cannot be converted to double";
        throw parse_error(error.str());
    }

    read_next_token();
}


// Convert current token to int and read next token. If conversion failed,
// throw parse error exception about it.

void BvhFile::token_to_int(int& num) throw(parse_error)
{
    stringstream s;
    s << token;
    s >> num;

    if (!s)
    {
        stringstream error;
        error << "\"" << token << "\" cannot be converted to int";
        throw parse_error(error.str());
    }

    read_next_token();
}


// bvh -> hierarchy motion end-of-file

void BvhFile::parse_bvh() throw(parse_error)
{
    // read first token
    read_next_token();

    parse_hierarchy();

    parse_motion();

    take_token("end-of-file");
}



// hierarchy -> "HIERARCHY" root

void BvhFile::parse_hierarchy() throw(parse_error)
{
    take_token("HIERARCHY");

    parse_root();
}


// root -> "ROOT" name { joint-data }

void BvhFile::parse_root() throw(parse_error)
{
    take_token("ROOT");

    token_to_string(joint_name);

    take_token("{");

    n_tridofs = 0;
    parse_joint_data(NULL);

    take_token("}");
}


// joint -> "JOINT" name { joint-data }

void BvhFile::parse_joint(Joint *parent) throw(parse_error)
{
    take_token("JOINT");

    token_to_string(joint_name);

    take_token("{");

    parse_joint_data(parent);

    take_token("}");
}


// joint-data -> offset [channels] (joint/end-site)+

void BvhFile::parse_joint_data(Joint *parent) throw(parse_error)
{
    parse_offset();

    joint_position_tridof = joint_rotation_tridof = -1;

    if (token_is("CHANNELS")) parse_channels();

    // create the joint
    Joint *joint = new Joint(joint_name, joint_offset, identity_quaternion,
        joint_position_tridof, joint_rotation_tridof);

    // link joint to the hierarchy or set it as a root
    if (parent)
        parent->AddChild(joint);
    else
    	m_skeleton = joint;

    do {
        if (token_is("JOINT")) parse_joint(joint);
        else if (token_is("End")) parse_end_site(joint);
        else
        {
            stringstream error;
        	error << "\"JOINT\" or \"End Site\" expected, but \"" << token
                  << "\" found instead";
        	throw parse_error(error.str());
        }
    }
    while (token_is("JOINT") || token_is("End"));
}


// end-site -> "End" "Site" { offset }

void BvhFile::parse_end_site(Joint *parent) throw(parse_error)
{
    take_token("End");
    take_token("Site");

    // end-site is without name, create name as parent joint end-effector
    joint_name = parent->name + "-EF";

    take_token("{");

    parse_offset();

    // create the joint (end-effector)
    Joint *joint = new Joint(joint_name, joint_offset, identity_quaternion,
        -1, -1);

    // link joint to the hierarchy or set it as a root
    if (parent)
        parent->AddChild(joint);
    else
    	m_skeleton = joint;    // Note: this should never occur

    take_token("}");
}


// offset -> "OFFSET" double double double

void BvhFile::parse_offset() throw(parse_error)
{
    take_token("OFFSET");

    token_to_double(joint_offset.x);
    token_to_double(joint_offset.y);
    token_to_double(joint_offset.z);
}


// channels -> "CHANNELS" int (Xposition/Yposition/Zposition/Xrotation/Yrotation/Zrotation)*

void BvhFile::parse_channels() throw(parse_error)
{
    int declared_channels;

    take_token("CHANNELS");

    token_to_int(declared_channels);

    if (declared_channels != 3 && declared_channels != 6)
        throw parse_error("Only 3 or 6 channels per joint is supported");

    if (declared_channels == 6)
    {
    	// position channels are expected first and in XYZ order
    	take_token("Xposition");
    	take_token("Yposition");
    	take_token("Zposition");

        // add the position triDOF name to the list
        tridof_names.push_back(joint_name + ".position");

        // set the index of joint's triDOF
        joint_position_tridof = n_tridofs;
        n_tridofs++;
    }

 	// rotation channels are expected then in ZXY order
	take_token("Zrotation");
	take_token("Xrotation");
	take_token("Yrotation");

    // add the rotation triDOF name to the list
    tridof_names.push_back(joint_name + ".rotation");

    // set the index of joint's triDOF
    joint_rotation_tridof = n_tridofs;
    n_tridofs++;
}


// motion -> "MOTION" "Frames:" int "Frame" "Time:" double motion-data

void BvhFile::parse_motion() throw(parse_error)
{
    take_token("MOTION");

    take_token("Frames:");

    token_to_int(n_frames);

    take_token("Frame");
    take_token("Time:");

    token_to_double(frame_time);

    parse_motion_data();
}


// motion-data -> (double)*

void BvhFile::parse_motion_data() throw(parse_error)
{

    if (n_frames <= 0 || n_tridofs <= 0)
    {
        stringstream error;
        error << "Number of frames and number of degrees-of-freedom must be > 0";
        throw parse_error(error.str());
    }

#ifdef DEBUG
    cerr << "Reading motion data...";
#endif

    // read sampled motion data, create one array for each DOF
    int n_dofs = 3 * n_tridofs;
    double* sampled_motion[n_dofs];
    for (int i = 0; i < n_dofs; i++)
        sampled_motion[i] = new double[n_frames];

    for (int i = 0; i < n_frames; i++)
        for (int j = 0; j < n_dofs; j++)
        	token_to_double(sampled_motion[j][i]);

#ifdef DEBUG
    cerr << "OK" << endl;
#endif

    // allocate array of pointers to triDOFs
    TriDOF **tridof = new (TriDOF*)[n_tridofs];

    double duration = (n_frames - 1) * frame_time;
    list<string>::iterator l = tridof_names.begin();

    // create individual triDOFs
    for (int i = 0; i < n_tridofs; i++, l++)
    {
    	// if tridof name constais ".rotation" create a rotational triDOF
    	// note that dofs in sampled motion are in ZXY order
        if (l->find(".rotation", 0) != string::npos)
            tridof[i] = new RotationalTriDOF(*l, n_frames,
                sampled_motion[3*i+1], sampled_motion[3*i+2],
                sampled_motion[3*i]);
        // else create a translational triDOF in XYZ order
        else
            tridof[i] = new TranslationalTriDOF(*l, n_frames,
                sampled_motion[3*i], sampled_motion[3*i+1],
                sampled_motion[3*i+2]);
    }

    // create motion name without path and extension
    string motion_name = filename;
    StripExtension(motion_name);
    string::size_type i = motion_name.find_last_of("/\\");
    if (i != string::npos)
        motion_name = motion_name.substr(i + 1, motion_name.length());

    // create a motion
    m_motion = new RecordedMotion(motion_name, duration, n_tridofs, tridof);
}



// create a simple one bone skeleton with one triDOF for testing purpose
// may be called from OnOpen instead of reading a chosen BVH file

bool create_test(Joint** skeleton, RecordedMotion** motion)
{
    // create skeleton
    Joint *root = new Joint("root", zero_vector, identity_quaternion, -1, 0);
    Joint *son = new Joint("son", Vector(0.0, 50.0, 0.0), identity_quaternion, -1, -1);
    root->AddChild(son);
    *skeleton = root;

    // create motion
    int n_frames = 10;
    double duration = 15.0;
    double sampled_motion[3][n_frames];
    for (int i = 0; i < n_frames; i++)
    {
        sampled_motion[0][i] = i * 40.0;
        sampled_motion[1][i] = 0;
        sampled_motion[2][i] = 0;
    }

    TriDOF **tridof = new (TriDOF*)[1];
/*    tridof[0] = new TranslationalTriDOF("test_3dof", n_frames,
                    sampled_motion[0], sampled_motion[1],
                    sampled_motion[2]);
*/

    tridof[0] = new RotationalTriDOF("test_3dof", n_frames,
                    sampled_motion[0], sampled_motion[1],
                    sampled_motion[2]);


    *motion = new RecordedMotion("test_motion", duration, n_frames, tridof);

    return true;
}

