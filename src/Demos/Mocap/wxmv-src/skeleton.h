///////////////////////////////////////////////////////////////////////////////
//
// skeleton.h
//
// Purpose:   Declaration of classes related to an articulated figure
//            (or skeleton) model.
//            Classes: Joint, Shot.
//
// Created:   Jaroslav Semancik, 13/10/2003
//
///////////////////////////////////////////////////////////////////////////////

#ifndef SKELETON_H
#define SKELETON_H

///////////////////////////////////////////////////////////////////////////////
//
// class Joint
//
// Class Joint represents a joint in an articulated figure (or skeleton).
// Joints are arranged in a tree-like hierarchy. Position and orientation of
// each joint is relative to its parent in the hierarchy. Any of position or
// rotation may be a triplet of degrees-of-freedom - it varies in time. Then
// its actual value for a particular time depends on a particular motion
// applied to the skeleton.
//
///////////////////////////////////////////////////////////////////////////////


class Joint
{
public:

    string name;        // name of the joint

// ? -> to private ?
    Joint *parent;      // pointer to a parent joint
    int n_children;     // number of children in a skeleton hierarchy
    Joint **child;      // array of pointers to childen in the hierarchy

    Vector world_position;      // joint position in world coordinates
    Vector next_world_position; // world_position in (time + VELOCITY_DELTA)
                                // (for calulating velocity of the joint)

    // Creates a joint with name nam, rest position rest_pos, rest rotation
    // rest_rot and indices to motion table for position and rotation
    // tridegree-of-freedom in pos_dof, rot_dof respectively. The joint is
    // created with no children and no parent. They must be added by AddChild
    // method.

    Joint(const string& nam, const Vector& rest_pos, const Quaternion& rest_rot,
          int pos_dof, const int rot_dof) throw();


    // Destructor - destroys recursively all joint's descendants first.

    ~Joint() throw();


    // Add a joint chld as a direct descendant of the joint in the hierarchy
    // and set the joint as a perent of the child.

    void AddChild(Joint* chld) throw();


    // Search for a joint with name nam in the hierarchy and return a pointer
    // to it.

    Joint* LocateJoint(const string& nam);


    // Compute world coordinates of the joint and its subhierarchy in a given
    // motion, time and position and rotation of its parent's coordinate
    // frame. Argument world_pos is pointer to a class attribute where to
    // store a result (world_position and next_position are usually used).
    // Drawing or exporting a joint should always come after evaluating
    // a current pose by this method.

    void Evaluate(Motion *motion, double time, const Vector &par_pos,
        const Quaternion &par_rot, Vector Joint::*world_pos);


    // Draw the joint and its subhierarchy using the world_position attribute.

    void Draw();


    // Export the joint and its subhierarchy (using the world_position
    // attribute) to a file as POV-Ray blob components.

    void ExportToPov(ofstream& file);


    // Write the names of the joint coordinates and its subhierarchy to
    // a line in CSV file. Shotname is stored as prefix to joint name.

    void HeaderToCsv(ofstream& file, const string& shotname);


    // Write world coordinates of the joint and its subhierarchy to a CSV
    // file.

    void ExportToCsv(ofstream& file);

private:

    // Position and rotation of the joint in a rest pose.

    Vector rest_position;       // position in the parent's coordinate frame
    Quaternion rest_rotation;   // orientation relative to parent's rotation

    // Indices of the respective triDOF signals in the motion table for
    // position and rotation of the joint. Those signals overide the rest
    // pose values. Index value -1 means that the joint position or rotation
    // is fixed.

    int position_dof;       // index of triDOF for position
    int rotation_dof;       // index of triDOF for rotation

    Vector eval_instant_position(Motion *motion, double time);
    Quaternion eval_instant_rotation(Motion *motion, double time);
};



///////////////////////////////////////////////////////////////////////////////
//
// class Shot
//
// Class Shot represents an animation of one character performing a motion.
// It associates a particular skeleton with a particular motion.
// Responsibility to associate a compatible skeleton-motion pair lies on the
// user of the class.
//
///////////////////////////////////////////////////////////////////////////////

class Shot
{
public:

    string name;                // name of the shot

    Joint  *skeleton;           // root of the skeleton
    Motion *motion;             // motion applied to the skeleton

    unsigned char r, g, b;      // color of the skeleton


    // Creates a shot of name nam with skeleton skel of rgb color (rr, gg, bb)
    // associated with motion mot.

    Shot(const string& nam, Joint* skel, Motion* mot,
        unsigned char rr, unsigned char gg, unsigned char bb) throw();


    // Destructor - destroys associated skeleton and motion.

    ~Shot();


    // Draw the shot in a given time.

    void Draw(double time);


    // Export the shot in a given time to a given POV file.

    void ExportToPov(ofstream& file, double time);


    // Write NAMES of joint coordinates listed in Joint::joints_exported_to_csv
    // and Joint::coodinates_exported_to_csv to a given CSV file.

    void HeaderToCsv(ofstream& file);


    // Write COORDINATES of joints listed in Joint::joints_exported_to_csv
    // and Joint::coodinates_exported_to_csv for a given time to a CSV file.

    void ExportToCsv(ofstream& file, double time);


    // Write a batch (script) file to cut the motion to individual steps using
    // bvhcopy utility. Step boundaries are in double-support phase.

    void ExportToBat(ofstream& file);
};



///////////////////////////////////////////////////////////////////////////////
//
// global variables related to articulated figures
//
///////////////////////////////////////////////////////////////////////////////


// array of all skeletons

extern "C++" vector<Joint*> skeletons;


// array of all shots

extern "C++" vector<Shot*> shots;

#endif

