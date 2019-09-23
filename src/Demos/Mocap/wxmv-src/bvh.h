///////////////////////////////////////////////////////////////////////////////
//
// bvh.h
//
// Purpose:   Declaration of classes for Biovision Hierarchy (BVH) file
//            format loading.
//            Classes: parse_error, BvhFile.
//
// Created:   Jaroslav Semancik, 13/10/2003
//
///////////////////////////////////////////////////////////////////////////////

#ifndef BVH_H
#define BVH_H

///////////////////////////////////////////////////////////////////////////////
//
// class parse_error
//
// Class parse_error is a kind of runtime_error for exceptions occured during
// parsing of an input motion file.
//
///////////////////////////////////////////////////////////////////////////////

class parse_error: public runtime_error
{
public:
    parse_error(const string& s): runtime_error(s) {}
};



///////////////////////////////////////////////////////////////////////////////
//
// class BvhFile
//
// Class BvhFile represents a motion file in Biovision Hierarchy (BVH) format
// and provides methods to load and parse it and to create corresponding
// skeleton and motion structures. A formal BVH format gramatics used for
// parsing can be found in the implementation file bvh.cpp.
//
///////////////////////////////////////////////////////////////////////////////

class BvhFile
{
public:

    string filename;

    // Creates an instance for a BVH file (with name fn).

    BvhFile() {};
    BvhFile(const string& fn) : filename(fn) {};

    // Destroys the temporary list of tridof names.

    ~BvhFile()
        { tridof_names.clear(); };
    
    
    // Assigns a filename to the BvhFile object.
    
    void Assign(const string& fn)
        { filename = fn; }

    // Loads an animation from the file. Creates a skeleton hierarchy of
    // joints and a recorded motion consisting of all degrees-of-freedom.
    // Pointers to the newly created skeleton and the motion are stored to
    // parameters' destination. Reports a message if an error occured and returns
    // a bool if the file has been loaded successfully.

    bool Load(Joint** skeleton, RecordedMotion** motion) throw();


private:

    // State variables of parsing.

    ifstream file;              // file stream for reading

    string token;               // current token
    int line;                   // current line

    // A temporary table of DOF names and their order is being created while
    // parsing HIERARCHY section. After reading of entire MOTION section
    // a recorded motion (class RecordedMotion) is created as an array of
    // DOFs (class DegreeOfFreedom) using the DOF names table and the motion
    // data.

    list<string> tridof_names;  // list of all triDOF names
    int n_tridofs;              // number of all triDOFs

    // Data of the joint currently being read are then used at once in
    // joint constructor call.

    string joint_name;          // name
    Vector joint_offset;        // OFFSET values

    int joint_position_tridof;  // indices to position tridegrees-of-freedom
    int joint_rotation_tridof;  // indices to rotation triDOFs

    // Data of the motion record, used then in constructor call.

    int n_frames;               // number of frames
    double frame_time;          // time difference between frames

    // Pointers to skeleton and motion structures 'returned' by the Load
    // method.

    Joint *m_skeleton;          // root of the skeleton read from the file
    RecordedMotion *m_motion;   // motion record read from the file


    void read_next_token() throw(parse_error);
    bool token_is(const string& tok) throw();
    void take_token(const string& tok) throw(parse_error);
    void token_to_string(string& str) throw(parse_error);
    void token_to_double(double& num) throw(parse_error);
    void token_to_int(int& num) throw(parse_error);

    void parse_bvh() throw(parse_error);
    void parse_hierarchy() throw(parse_error);
    void parse_root() throw(parse_error);
    void parse_joint(Joint *parent) throw(parse_error);
    void parse_joint_data(Joint *parent) throw(parse_error);
    void parse_end_site(Joint *parent) throw(parse_error);
    void parse_offset() throw(parse_error);
    void parse_channels() throw(parse_error);
    void parse_motion() throw(parse_error);
    void parse_motion_data() throw(parse_error);
};


bool create_test(Joint** skeleton, RecordedMotion** motion);

#endif

