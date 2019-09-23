///////////////////////////////////////////////////////////////////////////////
//
// motion.cpp
//
// Purpose:   Implementation of classes related to motion.
//            Classes: TriDOF, TranslationalTriDOF, RotationalTriDOF,
//            Motion, RecordedMotion, BlendedMotion, InterpolatedMotion,
//
// Created:   Jaroslav Semancik, 8/10/2003
//
///////////////////////////////////////////////////////////////////////////////

// #define DEBUG

#include <string>
#include <sstream>
#include <iostream>
#include <stdexcept>
#include <vector>

using namespace std;

#include "base.h"
#include "algebra.h"
#include "motion.h"


// static class attributes

bool Motion::wrap = true;


///////////////////////////////////////////////////////////////////////////////
//
// class TriDOF - public methods
//
///////////////////////////////////////////////////////////////////////////////

// Creates a TriDOF with name nam.

TriDOF::TriDOF(const string& nam) throw()
    : name(nam)
{
#ifdef DEBUG
    cerr << "Creating triDOF " << name << "...OK" << endl;
#endif
}


// Destructor - empty.

TriDOF::~TriDOF() throw()
{
#ifdef DEBUG
    cerr << "Destroying triDOF " << name << "...OK" << endl;
#endif
}



///////////////////////////////////////////////////////////////////////////////
//
// class TranslationalTriDOF - public methods
//
///////////////////////////////////////////////////////////////////////////////

// Creates a translational tridegree-of-freedom with name nam, from
// n_samples of uniformly sampled coordinates stored in arrays xx, yy and zz.
// Checks whether n_samples >= 1, otherwise an invalid_argument exception
// is thrown.

TranslationalTriDOF::TranslationalTriDOF(const string& nam, int n_samp,
    const double* xx, const double* yy, const double* zz) throw(invalid_argument)
    : TriDOF(nam)
{
#ifdef DEBUG
    cerr << "Creating translational tridegree-of-freedom " << nam << "...";
#endif

    if (n_samp < 1)
    {
        stringstream error;
    	error << "Creating \"" << nam
              << "\" TranslationalTriDOF with no samples"
              << " (# of samples = " << n_samples << ")";
        throw invalid_argument(error.str());
    }

    n_samples = n_samp;
    sample = new Vector[n_samples];
    
    // create sampled translation signal
    for (int i = 0; i < n_samples; i++)
        sample[i] = Vector(xx[i], yy[i], zz[i]);
    
#ifdef DEBUG
    cerr << "OK" << endl;
#endif
}


// Destructor - frees the memory allocated for the signal representation.
         
TranslationalTriDOF::~TranslationalTriDOF() throw()
{
#ifdef DEBUG
    cerr << "Destroying translational tridegree-of-freedom " << name << "...";
#endif

    delete[] sample;

#ifdef DEBUG
    cerr << "OK" << endl;
#endif
}


// Evaluates the triDOF at a given local time ltime in [0,1].

Vector TranslationalTriDOF::Evaluate(double ltime) throw()
{
    double r, t;
    int i;
    
    if (n_samples == 1) return sample[0];

    r = ltime * (n_samples - 1);
    i = (int)r;

    if (i >= n_samples - 1) return sample[n_samples - 1];
    if (i < 0) return sample[0];

    t = r - i;
    
    return (1 - t) * sample[i] + t * sample[i + 1];
}



///////////////////////////////////////////////////////////////////////////////
//
// class RotationalTriDOF - public methods
//
///////////////////////////////////////////////////////////////////////////////

// Creates a rotational tridegree-of-freedom with name nam, from
// n_samples of uniformly sampled Euler angles (in degrees) stored in
// arrays xx, yy and zz.

RotationalTriDOF::RotationalTriDOF(const string& nam, int n_samp,
    const double* xx, const double* yy, const double* zz) throw(invalid_argument)
    : TriDOF(nam)
{
#ifdef DEBUG
    cerr << "Creating rotational tridegree-of-freedom " << nam << "...";
#endif

    if (n_samp < 1)
    {
        stringstream error;
    	error << "Creating \"" << nam
              << "\" RotationalTriDOF with no samples"
              << " (# of samples = " << n_samples << ")";
        throw invalid_argument(error.str());
    }

    n_samples = n_samp;
    sample = new Quaternion[n_samples];

    // create sampled rotation signal
    for (int i = 0; i < n_samples; i++)
    {
        // create quaternions equivalent to individual Euler rotations
        Quaternion qx(x_vector, DEG2RAD(xx[i]));
        Quaternion qy(y_vector, DEG2RAD(yy[i]));
        Quaternion qz(z_vector, DEG2RAD(zz[i]));

        // compose to one rotation, BVH files use ZXY order of Euler angles
        sample[i] = qz * qx * qy;
    }

#ifdef DEBUG
    cerr << "OK" << endl;
#endif
}


// Destructor - frees the memory allocated for the signal representation.

RotationalTriDOF::~RotationalTriDOF() throw()
{
#ifdef DEBUG
    cerr << "Destroying rotational tridegree-of-freedom " << name << "...";
#endif

    delete[] sample;

#ifdef DEBUG
    cerr << "OK" << endl;
#endif
}


// Evaluates the triDOF at a given local time ltime in [0,1].

Quaternion RotationalTriDOF::Evaluate(double ltime) throw()
{
    double r, t;
    int i;

    if (n_samples == 1) return sample[0];

    r = ltime * (n_samples - 1);
    i = (int)r;

    if (i >= n_samples - 1) return sample[n_samples - 1];
    if (i < 0) return sample[0];

    t = r - i;

    return slerp(sample[i], sample[i + 1], t);
}



///////////////////////////////////////////////////////////////////////////////
//
// class Motion - public methods
//
///////////////////////////////////////////////////////////////////////////////

// Creates a motion with name nam and duration dur.

Motion::Motion(const string& nam, double dur) throw()
    : name(nam), duration(dur)
{
#ifdef DEBUG
    cerr << "Creating motion " << name << "...OK" << endl;
#endif
}


// Destructor - empty.

Motion::~Motion() throw()
{
#ifdef DEBUG
    cerr << "Destroying motion " << name << "...OK" << endl;
#endif
}


// Convert global time to local time of the motion, ie. transforms
// a [0,duration] interval to [0,1] interval. For time out of [0,duration]
// interval wraps according to Motion::wrap state. 

double Motion::ToLocalTime(double time) throw()
{
    if (ZERO(duration)) return 0;

    double lt = time / duration;
    if (wrap)
        lt = lt - floor(lt);
    else
    {
        if (lt > 1) lt = 1;
        else if (lt < 0) lt = 0;
    }
    return lt;
}



///////////////////////////////////////////////////////////////////////////////
//
// class RecordedMotion - public methods
//
///////////////////////////////////////////////////////////////////////////////

// Creates a recorded motion with name nam, duration dur, number ntd of
// tridegrees-of-freedom and array of recorded triDOFs in td.

RecordedMotion::RecordedMotion(const string& nam, double dur, int ntd,
    TriDOF** td) throw()
    : Motion(nam, dur), n_tridofs(ntd), tridof(td)
{
#ifdef DEBUG
    cerr << "Creating recorded motion " << name << "...OK" << endl;
#endif
}


// Destructor - destroys all tridegrees-of-freedom in the motion.

RecordedMotion::~RecordedMotion() throw()
{
#ifdef DEBUG
    cerr << "Destroying recorded motion " << name << "...";
#endif

    for (int i = 0; i < n_tridofs; i++)
        delete tridof[i];
    delete[] tridof;

#ifdef DEBUG
    cerr << "OK" << endl;
#endif
}


// Evaluates i-th translational triDOF of the recorded motion for a given
// local time.

Vector RecordedMotion::Eval_transl_tridof(int i, double ltime)
    throw(out_of_range)
{
    if (i >= 0 and i < n_tridofs and tridof)
        return ((TranslationalTriDOF*)tridof[i])->Evaluate(ltime);

    else
    {
        stringstream error;
    	error << "Evaluating " << i << "-th triDOF in recorded motion " << name
              << "with " << n_tridofs << " triDOFs.";
        throw out_of_range(error.str());
    }
}


// Evaluates i-th rotational triDOF of the recorded motion for a given
// local time.

Quaternion RecordedMotion::Eval_rot_tridof(int i, double ltime)
    throw(out_of_range)
{
    if (i >= 0 and i < n_tridofs and tridof)
        return ((RotationalTriDOF*)tridof[i])->Evaluate(ltime);

    else
    {
        stringstream error;
    	error << "Evaluating " << i << "-th triDOF in recorded motion " << name
              << " with " << n_tridofs << " triDOFs.";
        throw out_of_range(error.str());
    }
}



///////////////////////////////////////////////////////////////////////////////
//
// class BlendedMotion - public methods
//
///////////////////////////////////////////////////////////////////////////////

// Creates a blended motion with name nam, number nm of motions
// to blend, array of pointers to the motions in m and array
// of the respective weights in w.

BlendedMotion::BlendedMotion(const string& nam, int nm, Motion** m,
    double *w) throw()
    : Motion(nam, 0), n_motions(nm), motion(m), weight(w)
{
#ifdef DEBUG
    cerr << "Creating blended motion " << name << "...";
#endif

    if (n_motions > 0)
        duration = compute_duration();

#ifdef DEBUG
    cerr << "OK" << endl;
#endif
}


// Destructor - destroys arrays of pointers to motions and weights.

BlendedMotion::~BlendedMotion() throw()
{
#ifdef DEBUG
    cerr << "Destroying blended motion " << name << "...";
#endif

    delete[] weight;
    delete[] motion;

#ifdef DEBUG
    cerr << "OK" << endl;
#endif
}


// Evaluates i-th translational triDOF of the blended motion for a given
// local time.
// It evaluates i-th triDOF of all motions to blend at the given local time
// and computes their weighted sum then.

Vector BlendedMotion::Eval_transl_tridof(int i, double ltime)
    throw(out_of_range)
{
    Vector res(0.0f, 0.0f, 0.0f);
    for (int j = 0; j < n_motions; j++)
        if (!ZERO(weight[j]))
            res += weight[j] * motion[j]->Eval_transl_tridof(i, ltime);
    return res;
}


// Evaluates i-th rotational triDOF of the blended motion for a given
// local time.
// Any unit quaternion represents the same rotation as do its negation. Thus
// the quaternion of the first motion with nonzero weight is set as a pivot.
// For all remaining motions with nozero weight, the one from two equivalent
// quaternions q or -q is chosen that has smaller angle (<PI/2) with the
// pivot and added to the weighted sum.

Quaternion BlendedMotion::Eval_rot_tridof(int i, double ltime)
    throw(out_of_range)
{
    Quaternion q, pivot, res(0.0f, 0.0f, 0.0f, 1.0f);
    bool pivot_found = false;

    // for all motions with nonzero weight
    for (int j = 0; j < n_motions; j++)
    {
        if (ZERO(weight[j])) continue;

        q = motion[j]->Eval_rot_tridof(i, ltime);

        // first quaternion with nonzero weight will be a pivot
        if (!pivot_found)
        {
            pivot = q;
            pivot_found = true;
            res = weight[j] * q;
        }
        // the one of the q or -q with small angle to the pivot add to the sum
        else
        {
            if (dot(pivot, q) >= 0.0f)
                res += weight[j] * q;
            else
                res -= weight[j] * q;
        }
    }

    return res;
}



///////////////////////////////////////////////////////////////////////////////
//
// class BlendedMotion - private methods
//
///////////////////////////////////////////////////////////////////////////////

// Computes duration of the blended motion. It is a weighted sum of
// durations of all motions to blend.

double BlendedMotion::compute_duration() throw()
{
    double d = 0;
    for (int i = 0; i < n_motions; i++)
        d += weight[i] * motion[i]->duration;
    return d;
}



///////////////////////////////////////////////////////////////////////////////
//
// class InterpolatedMotion - public methods
//
///////////////////////////////////////////////////////////////////////////////

// Creates an interpolated motion with name nam, number nm of motions
// to blend, array of pointers to the motions in m and arrays
// of the respective initial (ws) and final (we) weights.

InterpolatedMotion::InterpolatedMotion(const string& nam, int nm, Motion** m,
    double *ws, double *we) throw()
    : BlendedMotion(nam, nm, m, ws), weight_start(ws), weight_end(we)
{
#ifdef DEBUG
    cerr << "Creating interpolated motion " << name << "...";
#endif

    if (n_motions > 0)
    {
    	// create an array for current weights, ws was passed to BlendedMotion
    	// just for its correct construction
        weight = new double[n_motions];
        interpolate_weights(0.0);

        duration = compute_duration();
    }

#ifdef DEBUG
    cerr << "OK" << endl;
#endif
}


// Destructor - destroys arrays of pointers to motions and weights.

InterpolatedMotion::~InterpolatedMotion() throw()
{
#ifdef DEBUG
    cerr << "Destroying interpolated motion " << name << "...";
#endif

    delete[] weight_start;
    delete[] weight_end;

#ifdef DEBUG
    cerr << "OK" << endl;
#endif
}


// Evaluates i-th translational triDOF of the interpolated motion for a given
// local time.
// It interpolates for current weights at given local time first, then
// evaluates a blended motion with those weights.

Vector InterpolatedMotion::Eval_transl_tridof(int i, double ltime)
    throw(out_of_range)
{
    // interpolate weights unless they are already computed for time ltime
    if (!EQUAL(ltime, curr_weights_time))
        interpolate_weights(ltime);

    return BlendedMotion::Eval_transl_tridof(i, ltime);
}


// Evaluates i-th rotational triDOF of the interpolated motion for a given
// local time.
// It interpolates for current weights at given local time first, then
// evaluates a blended motion with those weights.

Quaternion InterpolatedMotion::Eval_rot_tridof(int i, double ltime)
    throw(out_of_range)
{
    // interpolate weights unless they are already computed for time ltime
    if (!EQUAL(ltime, curr_weights_time))
        interpolate_weights(ltime);

    return BlendedMotion::Eval_rot_tridof(i, ltime);
}



///////////////////////////////////////////////////////////////////////////////
//
// class InterpolatedMotion - private methods
//
///////////////////////////////////////////////////////////////////////////////

// Computes duration of the interpolated motion. It is a weighted sum of
// source motion durations with weights integrated over interval [0,1].
// In case of linear interpolation the weights integrated over [0,1] are
// average of the initial and final weights.

double InterpolatedMotion::compute_duration() throw()
{
    double d = 0;
    for (int i = 0; i < n_motions; i++)
        d += (weight_start[i] + weight_end[i]) * motion[i]->duration;
    return d / 2;
}


// Linearly interpolates initial and final weights to current weights
// of all motions for local time ltime.

void InterpolatedMotion::interpolate_weights(double ltime) throw()
{
    for (int i = 0; i < n_motions; i++)
        weight[i] = (1 - ltime) * weight_start[i] + ltime * weight_end[i];

    // update time the weights were last computed for
    curr_weights_time = ltime;
}


///////////////////////////////////////////////////////////////////////////////
//
// global variables related to motion
//
///////////////////////////////////////////////////////////////////////////////


// array of all motions

vector<Motion*> motions;

