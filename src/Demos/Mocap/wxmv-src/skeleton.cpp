///////////////////////////////////////////////////////////////////////////////
//
// skeleton.cpp
//
// Purpose:   Implementation of classes related to an articulated figure
//            (or skeleton) model.
//            Classes: Joint, Shot.
//
// Created:   Jaroslav Semancik, 13/10/2003
//
///////////////////////////////////////////////////////////////////////////////

// #define DEBUG

#include <wx/wx.h>
#include <wx/glcanvas.h>
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
#include "ogl.h"
#include "wxmv.h"


///////////////////////////////////////////////////////////////////////////////
//
// class Joint - public methods
//
///////////////////////////////////////////////////////////////////////////////

// Creates a joint with name nam, rest position rest_pos, rest rotation
// rest_rot and indices to motion table for position and rotation
// tridegree-of-freedom in pos_dof, rot_dof respectively. The joint is
// created with no children and no parent. They must be added by AddChild
// method.

Joint::Joint(const string& nam, const Vector& rest_pos,
    const Quaternion& rest_rot, int pos_dof, int rot_dof) throw()
{
#ifdef DEBUG
    cerr << "Creating joint " << nam << "...";
#endif

    name = nam;

    rest_position = rest_pos;
    rest_rotation = rest_rot;
    position_dof = pos_dof;
    rotation_dof = rot_dof;

    parent = NULL;
    n_children = 0;
    child = NULL;

#ifdef DEBUG
    cerr << "OK" << endl;
#endif
}


// Destructor - destroys recursively all joint's descendants first.

Joint::~Joint() throw()
{
#ifdef DEBUG
    cerr << "Destroying joint " << name << "(" << endl;
#endif

    for (int i = 0; i < n_children; i++)
        delete child[i];

#ifdef DEBUG
    cerr << ")";
#endif
}


// Add a joint chld as a direct descendant of the joint in the hierarchy
// and set the joint as a perent of the child.

void Joint::AddChild(Joint* chld) throw()
{
#ifdef DEBUG
    cerr << "Adding child " << chld->name << " to joint " << name << "...";
#endif

    n_children++;

    child = (Joint**) realloc(child, n_children * sizeof(Joint**));

    child[n_children - 1] = chld;

    chld->parent = this;

#ifdef DEBUG
    cerr << "OK" << endl;
#endif
}


// Search for a joint with name nam in the hierarchy and return a pointer
// to it.

Joint* Joint::LocateJoint(const string& nam)
{
    if (name == nam) return this;

    Joint *jnt = NULL;

    // search joint's subhierarchy
    for (int i = 0; i < n_children; i++)
        if ((jnt = child[i]->LocateJoint(nam)) != NULL) break;

    return jnt;
}


// Compute world coordinates of the joint and its subhierarchy in a given
// motion, time and position and rotation of its parent's coordinate
// frame. Argument world_pos is pointer to a class attribute where to
// store a result (world_position and next_position are usually used).
// Drawing or exporting a joint should always come after evaluating
// a current pose by this method.

void Joint::Evaluate(Motion *motion, double time, const Vector &par_pos,
    const Quaternion &par_rot, Vector Joint::*world_pos)
{
    // evaluate instant local position and rotation of the joint
    this->*world_pos = eval_instant_position(motion, time);
    Quaternion rot = eval_instant_rotation(motion, time);

    // transform joint position and rotation from relative to world
    // coordinates
    (this->*world_pos).rotate(par_rot);
    this->*world_pos += par_pos;
    rot = par_rot * rot;

    // draw joint's subhierarchy
    for (int i = 0; i < n_children; i++)
        child[i]->Evaluate(motion, time, this->*world_pos, rot, world_pos);
}


// Draw the joint and its subhierarchy using the world_position attribute.

void Joint::Draw()
{
    // draw a segment from the parent unless it is a root or the bone has
    // zero length
    if (parent && world_position != parent->world_position)
        glCanvas->RenderLine(parent->world_position, world_position);

    // draw the joint
    glCanvas->RenderPoint(world_position);

    // draw joint's subhierarchy
    for (int i = 0; i < n_children; i++)
        child[i]->Draw();
}


/*

Old version - bones' transformations using matrices and OpenGL matrix stack.

void Joint::Draw(Motion *motion, double time)
{
    // evaluate instant position and rotation for the given motion and time
    Vector pos = eval_instant_position(motion, time);
    Quaternion rot = eval_instant_rotation(motion, time);

    // draw a segment from parent unless it is a root or the bone has zero
    // length
    if (parent && !ZERO(pos.length()))
        glCanvas->RenderLine(zero_vector, pos);

    // draw joint
    glCanvas->RenderPoint(pos);

    // draw joint's subhierarchy
    for (int i = 0; i < n_children; i++)
    {
        // save the parent's coordinate frame
        glPushMatrix();

        // transform from parent's to the joint's coordinate frame
        Matrix m(rot);          // rotation matrix from quaternion
        m.c[3][0] = pos.x;      // add translation
        m.c[3][1] = pos.y;
        m.c[3][2] = pos.z;
        glMultMatrixd((GLdouble*)m.c);

        child[i]->Draw(motion, time);

        // restore the parent's coordinate frame
        glPopMatrix();
    }
}
*/


// Export the joint and its subhierarchy (using the world_position
// attribute) to a file as POV-Ray blob components. 

void Joint::ExportToPov(ofstream& file)
{
    // export segment from parent unless the joint is a root or degenerated
    if (parent && world_position != parent->world_position)
    {
        file << "cylinder { <"
             << parent->world_position.x << ", "
             << parent->world_position.y << ", "
             << parent->world_position.z << ">, <"
             << world_position.x << ", "
             << world_position.y << ", "
             << world_position.z << ">, th, 1 }" << endl;

        // write blob adjustment in the joint
        if (n_children > 0)
            file << "sphere { <"
                 << world_position.x << ", "
                 << world_position.y << ", "
                 << world_position.z << ">, th, jnt }" << endl;
    }

    // export joint's subhierarchy
    for (int i = 0; i < n_children; i++)
        child[i]->ExportToPov(file);
}


// Write the names of the joint coordinates and its subhierarchy to
// a line in CSV file.

void Joint::HeaderToCsv(ofstream& file, const string& shotname)
{
    if (mainFrame->joints_to_csv.find(name + " ") != string::npos ||
        mainFrame->joints_to_csv.empty())
    {
        if (mainFrame->coords_to_csv.find_first_of("xX") != string::npos)
            file << shotname << '.' << name << ".x,";
        if (mainFrame->coords_to_csv.find_first_of("yY") != string::npos)
            file << shotname << '.' << name << ".y,";
        if (mainFrame->coords_to_csv.find_first_of("zZ") != string::npos)
            file << shotname << '.' << name << ".z,";
        if (mainFrame->coords_to_csv.find_first_of("vV") != string::npos)
            file << shotname << '.' << name << ".v,";
    }

    // export joint's subhierarchy
    for (int i = 0; i < n_children; i++)
        child[i]->HeaderToCsv(file, shotname);
}


// Write world coordinates of the joint and its subhierarchy to a CSV file.

void Joint::ExportToCsv(ofstream& file)
{
    // write the joint world coordinates
    if (mainFrame->joints_to_csv.find(name + " ") != string::npos ||
        mainFrame->joints_to_csv.empty())
    {
        if (mainFrame->coords_to_csv.find_first_of("xX") != string::npos)
            file << world_position.x << ",";
        if (mainFrame->coords_to_csv.find_first_of("yY") != string::npos)
            file << world_position.y << ",";
        if (mainFrame->coords_to_csv.find_first_of("zZ") != string::npos)
            file << world_position.z << ",";

        // write joint's velocity
        if (mainFrame->coords_to_csv.find_first_of("vV") != string::npos)
            file << (next_world_position - world_position).length()
                 / mainFrame->velocity_delta << ",";
    }

    // export joint's subhierarchy
    for (int i = 0; i < n_children; i++)
        child[i]->ExportToCsv(file);
}



///////////////////////////////////////////////////////////////////////////////
//
// class Joint - private methods
//
///////////////////////////////////////////////////////////////////////////////

// Evaluates instant position of the joint in a given motion and time.

Vector Joint::eval_instant_position(Motion *motion, double time)
{
    if (position_dof >= 0)
    {
    	// motion must be evaluated in local time
    	double lt = motion->ToLocalTime(time);
        return motion->Eval_transl_tridof(position_dof, lt);
    }
    else
        return rest_position;
}


// Evaluates instant rotation of the joint in a given motion and time.

Quaternion Joint::eval_instant_rotation(Motion *motion, double time)
{
    if (rotation_dof >= 0)
    {
    	// motion must be evaluated in local time
    	double lt = motion->ToLocalTime(time);
            return motion->Eval_rot_tridof(rotation_dof, lt);
    }
    else
        return rest_rotation;
}



///////////////////////////////////////////////////////////////////////////////
//
// class Shot - public methods
//
///////////////////////////////////////////////////////////////////////////////

// Creates a shot of name nam with skeleton skel of rgb color (rr, gg, bb)
// associated with motion mot.

Shot::Shot(const string& nam, Joint* skel, Motion* mot,
    unsigned char rr, unsigned char gg, unsigned char bb) throw()
    : name(nam), skeleton(skel), motion(mot), r(rr), g(gg), b(bb)
{
#ifdef DEBUG
    cerr << "Creating shot " << nam << "...OK" << endl;
#endif
}


// Destructor - destroys associated skeleton and motion.

Shot::~Shot()
{
#ifdef DEBUG
    cerr << "Destroying shot " << name << "...";
#endif

//    delete skeleton;
//    delete motion;

#ifdef DEBUG
    cerr << "OK" << endl;
#endif
}


// Draw the shot in a given time.

void Shot::Draw(double time)
{
    glColor3ub(r, g, b);

    // draw the skeleton with applied motion and coordinate frame in origin
    skeleton->Evaluate(motion, time, zero_vector, identity_quaternion,
        &Joint::world_position);
    skeleton->Draw();
}


// Export the shot in a given time to a given file.

void Shot::ExportToPov(ofstream& file, double time)
{
    skeleton->Evaluate(motion, time, zero_vector, identity_quaternion,
        &Joint::world_position);
    skeleton->ExportToPov(file);
}


// Write NAMES of joint coordinates listed in mainFrame->joints_to_csv and
// mainFrame->coords_to_csv to a given CSV file.

void Shot::HeaderToCsv(ofstream& file)
{
    skeleton->HeaderToCsv(file, name);
}


// Write COORDINATES of joints listed in mainFrame->joints_to_csv and
// mainFrame->coords_to_csv for a given time to a CSV file.

void Shot::ExportToCsv(ofstream& file, double time)
{
    skeleton->Evaluate(motion, time, zero_vector, identity_quaternion,
        &Joint::world_position);
    skeleton->Evaluate(motion, time + mainFrame->velocity_delta, zero_vector,
        identity_quaternion, &Joint::next_world_position);
    skeleton->ExportToCsv(file);
}


// Write a batch (script) file to cut the motion to individual steps using
// bvhcopy utility. Step boundaries are in double-support phase.

void Shot::ExportToBat(ofstream& file)
{
    double time, it, ft, isl, fsl;
    int i, ii, nf, start, end;
    unsigned int step_n, n;
    Joint *jnt;
    string filename;
    char cs[11];

    // check is both toes names exist in th hierarchy
    jnt = skeleton->LocateJoint(mainFrame->ltoes_name);
    if (jnt)
        jnt = skeleton->LocateJoint(mainFrame->rtoes_name);

    if (!jnt)
    {
        stringstream error;
        error << "Joint " << mainFrame->ltoes_name << " or "
              << mainFrame->rtoes_name << " does not exist in shot " << name
              << ". The shot will not be exported to batch file.";
        ErrorMessage(error.str());
        return;
    }

    // number of frames of the motion
    nf = (int)ROUND(motion->duration / mainFrame->frametime) + 1;

    // create tables for coordinates and velocity of left and right toes
    // during the motion
    Vector *ltoes = new Vector[nf];
    Vector *rtoes = new Vector[nf];
    double *ltoes_v = new double[nf];
    double *rtoes_v = new double[nf];

    // fill tables by toes coordinates and velocity
    for (i = 0, time = 0.0; i < nf; i++, time += mainFrame->frametime)
    {
        skeleton->Evaluate(motion, time, zero_vector, identity_quaternion,
            &Joint::world_position);
        skeleton->Evaluate(motion, time + mainFrame->velocity_delta,
            zero_vector, identity_quaternion, &Joint::next_world_position);

        jnt = skeleton->LocateJoint(mainFrame->ltoes_name);
        ltoes[i] = jnt->world_position;
        ltoes_v[i] = (jnt->next_world_position - jnt->world_position).length()
                   / mainFrame->velocity_delta;

        jnt = skeleton->LocateJoint(mainFrame->rtoes_name);
        rtoes[i] = jnt->world_position;
        rtoes_v[i] = (jnt->next_world_position - jnt->world_position).length()
                   / mainFrame->velocity_delta;
    }

    // create a table of marks whether both feet are planted in a particular
    // frame or not
    bool *planted = new bool[nf];

    // mark frames where both feet are planted
    for (i = 0; i < nf; i++)
    {
        if (ltoes_v[i] < mainFrame->velocity_limit
         && ltoes[i].y < mainFrame->height_limit
         && rtoes_v[i] < mainFrame->velocity_limit
         && rtoes[i].y < mainFrame->height_limit)

            planted[i] = true;
        else
            planted[i] = false;
    }

    // start of the first step is the start of the motion
    start = 0;
    step_n = 0;

    // step boundaries will be centers of the intervals where both feet are
    // planted
    i = 0;
    while (i < nf)
    {
        // skip all not planted frames
        while (i < nf && !planted[i]) i++;

        if (i == nf) end = nf - 1;
        else
        {
            // find center of a planted interval
            ii = i;
            while (i < nf && planted[i]) i++;
            end = (ii + i) / 2;
        }

        // prepare filename for the step
        sprintf(cs, "%03d", step_n);
        filename = name;
        n = filename.rfind(".", filename.length() - 1);
        if (n == string::npos) n = filename.length();
        filename.insert(n, cs);

        // compute initial and final time and step lengths
        it = motion->duration * start / (nf - 1);
        ft = motion->duration * end / (nf - 1);
        isl = (ltoes[start] - rtoes[start]).length();
        fsl = (ltoes[end] - rtoes[end]).length();

        // write comments and a line to cut a step from start to end frame
        file << endl;
        file << "REM Step " << step_n << ", time interval "
             << it << "s--" << ft << "s, "
             << "initial length " << isl << ", final length " << fsl << endl;
        file << "bvhcopy -s " << start << " -e " << end << " -o " << filename
             << ".bvh " << name << ".bvh" << endl;

        start = end;
        step_n++;
    }

    delete[] planted;
    delete[] rtoes_v;
    delete[] ltoes_v;
    delete[] rtoes;
    delete[] ltoes;
}



///////////////////////////////////////////////////////////////////////////////
//
// global variables related to articulated figures
//
///////////////////////////////////////////////////////////////////////////////


// array of all skeletons

vector<Joint*> skeletons;


// array of all shots

vector<Shot*> shots;

