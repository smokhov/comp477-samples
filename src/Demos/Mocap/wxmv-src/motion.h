///////////////////////////////////////////////////////////////////////////////
//
// motion.h
//
// Purpose:   Declaration of classes related to motion.
//            Classes: TriDOF, TranslationalTriDOF, RotationalTriDOF,
//            Motion, RecordedMotion, BlendedMotion, InterpolatedMotion,  
//
// Created:   Jaroslav Semancik, 8/10/2003
//
///////////////////////////////////////////////////////////////////////////////

#ifndef MOTION_H
#define MOTION_H

///////////////////////////////////////////////////////////////////////////////
//
// class TriDOF
//
// Class TriDOF represents a triplet of time-varying degrees-of-freedom (DOF)
// for a joint in an articulated figure as a continuous signal.
// TriDOF is an abstract class to derive classes for triplets of translational
// and rotational DOFs.
//
///////////////////////////////////////////////////////////////////////////////

class TriDOF
{
public:

    string name;        // name of the DOF

    // Creates a TriDOF with name nam.

    TriDOF(const string& nam) throw();

    // Destructor - empty.

    virtual ~TriDOF() throw();
};



///////////////////////////////////////////////////////////////////////////////
//
// class TranslationalTriDOF
//
// TranslationalTriDOF is a triplet of translational degrees-of-freedom.
// Continuous translational signal is represented as a piecewise linear
// 3D polygon on given uniform samples. Dense samples (e.g. from motion
// capture) are assumed. The signal can be evaluated using Evaluate method.
//
///////////////////////////////////////////////////////////////////////////////

class TranslationalTriDOF : public TriDOF
{
public:

    // Creates a translational tridegree-of-freedom with name nam, from
    // n_samples of uniformly sampled coordinates stored in arrays xx, yy
    // and zz.

    TranslationalTriDOF(const string& nam, int n_samp,
        const double* xx, const double* yy, const double* zz)
        throw(invalid_argument);


    // Destructor - frees the memory allocated for the signal representation.

    ~TranslationalTriDOF() throw();


    // Evaluates the triDOF at a given local time ltime in [0,1].

    virtual Vector Evaluate(double ltime) throw();

private:

    // Representation of a translational signal is currently a piecewise
    // linear 3D-polygon on the original samples.

    int n_samples;              // # of samples in the signal curve
    Vector *sample;             // array of samples - 3D translations
};



///////////////////////////////////////////////////////////////////////////////
//
// class RotationalTriDOF
//
// RotationalTriDOF is a triplet of rotational degrees-of-freedom. Rotations
// are represented and manipulated by quaternions. Continuous rotational
// signal is represented as a piecewise linear spherical polygon on given
// uniform samples. Consecutive samples are interpolated by spherical 'slerp'
// interpolation. The signal can be evaluated using Evaluate method. 
//
///////////////////////////////////////////////////////////////////////////////

class RotationalTriDOF : public TriDOF
{
public:

    // Creates a rotational tridegree-of-freedom with name nam, from
    // n_samples of uniformly sampled Euler angles (in degrees) stored in
    // arrays xx, yy and zz.

    RotationalTriDOF(const string& nam, int n_samp,
        const double* xx, const double* yy, const double* zz)
        throw(invalid_argument);


    // Destructor - frees the memory allocated for the signal representation.

    ~RotationalTriDOF() throw();

    
    // Evaluates the triDOF at a given local time ltime in [0,1].

    virtual Quaternion Evaluate(double ltime) throw();

private:

    // Continuous rotational signal is currently implemented by spherical
    // interpolation (slerp) of consecutive samples represented by unit
    // quaternions.

    int n_samples;              // # of samples in the signal curve
    Quaternion *sample;         // array of samples - quaternions rotations
};



///////////////////////////////////////////////////////////////////////////////
//
// class Motion
//
// Class Motion represents a set of TriDOFs varying in one motion sequence.
// The TriDOFs are accessed by index - that decouples models and motions
// allowing arbitrary linking of compatible skeleton models and motion
// sequences. The class Motion is an abstract class, that classes for various
// kinds of motion (recorded, blended) are derived from.
//
///////////////////////////////////////////////////////////////////////////////

class Motion
{
public:

    string name;            // name of the motion
    double duration;        // duration in seconds

    static bool wrap;       // state variable how to evaluate motion for
                            // time out of duration of the motion

    // Creates a motion with name nam and duration dur.

    Motion(const string& nam, double dur) throw();


    // Destructor - empty.

    virtual ~Motion() throw();

    
    // Convert global time to local time (in [0,1]) for the motion.

    double ToLocalTime(double time) throw();

    
    // Evaluates i-th translational triDOF of the motion for a given local
    // time.

    virtual Vector Eval_transl_tridof(int i, double ltime)
        throw(out_of_range) = 0;


    // Evaluates i-th rotational triDOF of the motion for a given local
    // time.

    virtual Quaternion Eval_rot_tridof(int i, double ltime)
        throw(out_of_range) = 0;
};



///////////////////////////////////////////////////////////////////////////////
//
// class RecordedMotion
//
// Recorded motion is a motion with recorded triDOFs, e.g. by motion capture.
// The recorded triDOFs are stored in an array of pointers to TriDOF objects.
//
///////////////////////////////////////////////////////////////////////////////


class RecordedMotion : public Motion
{
public:

    // Creates a recorded motion with name nam, duration dur, number ntd of
    // tridegrees-of-freedom and array of recorded triDOFs in td.

    RecordedMotion(const string& nam, double dur, int ntd, TriDOF** td)
        throw();


    // Destructor - destroys all tridegrees-of-freedom in the motion.

    ~RecordedMotion() throw();


    // Evaluates i-th translational or rotational triDOF (respectivelly) of
    // the recorded motion for a given local time.

    Vector Eval_transl_tridof(int i, double ltime) throw(out_of_range);

    Quaternion Eval_rot_tridof(int i, double ltime) throw(out_of_range);

private:

    int n_tridofs;          // number of triDOFs
    TriDOF **tridof;        // table of pointers to transl. and rot. triDOFs
};



///////////////////////////////////////////////////////////////////////////////
//
// class BlendedMotion
//
// Blended motion is a weighted sum of several motions. All the motions
// to blend must have the same number of triDOFs and all of them must be of
// the same kind (tranls./rot.). Duration of the motion also results from
// a weighted sum of durations.
// The class BlendedMotion extends the abstract motion class by an array
// of pointers to blended motions and an array of respective weights.
//
///////////////////////////////////////////////////////////////////////////////

class BlendedMotion : public Motion
{
public:

    // Creates a blended motion with name nam, number nm of motions
    // to blend, array of pointers to the motions in m and array
    // of the respective weights in w.

    BlendedMotion(const string& nam, int nm, Motion** m, double *w) throw();


    // Destructor - destroys arrays of pointers to motions and weights.

    ~BlendedMotion() throw();


    // Evaluates i-th translational or rotational triDOF (respectivelly) of
    // the blended motion for a given local time.
    // It evaluates all motions to blend at the given local time and
    // computes their weighted sum then.

    Vector Eval_transl_tridof(int i, double ltime) throw(out_of_range);

    Quaternion Eval_rot_tridof(int i, double ltime) throw(out_of_range);

protected:

    int n_motions;      // number of blended motions
    Motion **motion;    // array of motions to blend
    double *weight;     // array of respective weights

    double compute_duration() throw();
};



///////////////////////////////////////////////////////////////////////////////
//
// class InterpolatedMotion
//
// Interpolated motion blends motions weighted by time varying weights.
// Weights for a particular time are computed by interpolation of specified
// initial and final weights. All the source motions must have the same
// number and kind of all triDOFs. Duration of the motion is also computed
// from the weights and durations of source motions.
// The class InterpolatedMotion extends the blended motion class by arrays
// of initial and final weights.
//
///////////////////////////////////////////////////////////////////////////////

class InterpolatedMotion : public BlendedMotion
{
public:

    // Creates an interpolated motion with name nam, number nm of motions
    // to blend, array of pointers to the motions in m and arrays
    // of the respective initial (ws) and final (we) weights.

    InterpolatedMotion(const string& nam, int nm, Motion** m,
        double *ws, double *we) throw();


    // Destructor - destroys arrays of pointers to motions and weights.

    ~InterpolatedMotion() throw();


    // Evaluates i-th translational or rotational triDOF (respectivelly) of
    // the interpolated motion for a given local time.
    // It interpolates for current weights at given local time first, then
    // evaluates all motions to blend at that time and finally computes their
    // weighted sum.

    Vector Eval_transl_tridof(int i, double ltime) throw(out_of_range);

    Quaternion Eval_rot_tridof(int i, double ltime) throw(out_of_range);

private:

    double *weight_start;       // arrays of initial and final weights
    double *weight_end;

    double curr_weights_time;   // local time the current weights were last
                                //computed for

    double compute_duration() throw();
    void interpolate_weights(double ltime) throw();
};


///////////////////////////////////////////////////////////////////////////////
//
// global variables related to motion
//
///////////////////////////////////////////////////////////////////////////////


// array of all motions

extern "C++" vector<Motion*> motions;

#endif

