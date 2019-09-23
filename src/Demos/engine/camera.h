#ifndef CAMERA_H
#define CAMERA_H

#include <iostream>
using namespace std;

#include <GL/cugl.h>
using namespace cugl;

/*
 * An instance of class \c Camera is used to control the view.
 * The function \c Camera::apply() calls \c gluLookAt() with
 * appropriate parameters.  The other functions of this class
 * set up these parameters.
 * Smooth transitions can be obtained by using \c Camera::idle()
 * together with the various movement functions.
 */
class Camera
{
public:

   /*
    * Construct a default camera.  The view is set as follows:
    * Eye = (0,0,1).  Model=(0,0,0).  Up=(0,1,0).
    */
   Camera();

   /*
    * Construct a camera for a particular view.
    * \param eye sets the eye coordinates for \c gluLookAt().
    * \param model sets the model coordinates for \c gluLookAt().
    * \param up sets the 'up' vector for \c gluLookAt().
    */
   Camera(Point eye, Point model, Vector up);

   /*
    * Construct a camera for a particular view.
    * The 'up' vector is set to (0,1,0).
    * \param eye sets the eye coordinates for \c gluLookAt().
    * \param model sets the model coordinates for \c gluLookAt().
    */
   Camera(Point eye, Point model);

   /*
    * Construct a camera for a particular view.
    * The 'up' vector is set to (0,1,0).
    * The model point is set to (0,0,0).
    * \param eye sets the eye coordinates for \c gluLookAt().
    */
   Camera(Point eye);

   /*
    * Set the camera for a particular view.
    * \param eye sets the eye coordinates for \c gluLookAt().
    * \param model sets the model coordinates for \c gluLookAt().
    * \param up sets the 'up' vector for \c gluLookAt().
    */
   void set(Point eye, Point model, Vector up);

   /*
    * Set the camera for a particular view.
    * The 'up' vector is set to (0,1,0).
    * \param eye sets the eye coordinates for \c gluLookAt().
    * \param model sets the model coordinates for \c gluLookAt().
    */
   void set(Point eye, Point model);

   /*
    * Set the camera for a particular view.
    * The model point is set to (0,0,0).
    * The 'up' vector is set to (0,1,0).
    * \param eye sets the eye coordinates for \c gluLookAt().
    */
   void set(Point eye);

   /*
    * Set the number of steps required for a camera movement.
    * Steps = 10 will give a rapid, possibly jerky, movement.
    * Steps = 100 will give a slow, smooth movement.
    */
   void setSteps(int s);

   /*
    * Update the camera position.
    * This function should be called from the GLUT idle() function
    * or its equivalent.  It performs one step of the motion until
    * the motion is complete, after which it has no effect.
    */
   void idle();

   /*
    * Apply the camera position to the model-view matrix.
    * This function calls \c gluLookAt() with appropriate parameters.
    * It should be called in the GLUT display function or its equivalent
    * after the model-view matrix has been initialized.
    */
   void apply();

   /*
    * Move the camera upwards (+Y direction).
    * \param distance gives the amount of movement.  Negative values move
    * the camera downwards.
    */
   void moveUp(GLfloat distance);

   /*
    * Move the camera forwards.
    * The camera moves towards the model.
    * \param distance gives the amount of movement.  Negative values move
    * the camera away from the model..
    */
   void moveForward(GLfloat distance);

   /*
    * Move the camera to its left.
    * \param distance gives the amount of movement.  Negative values move
    * the camera to its right.
    */
   void moveLeft(GLfloat distance);

   /*
    * Tilt the camera upwards.
    * \param angle gives the angle through which the camera should be moved.
    * Negative values give a downward tilt.
    */
   void tiltUp(double angle);

   /*
    * Pan the camera to the left.
    * \param angle gives the angle through which the camera should be panned.
    * Negative values give panning to the right.
    */
   void panLeft(double angle);

   /*
    * Write a representation of the camera to a stream.
    * This function displays the current settings of the eye point,
    * the model point, and the up vector.
    * \param os is a reference to an output stream.
    * \param c is a reference to a camera.
    */
   friend ostream & operator<<(ostream & os, Camera & c);

private:

   // Eye coordinates for gluLookAt()
   Point eye;
   Point eyeOld;
   Point eyeNew;

   // Model coordinates for gluLookAt()
   Point model;
   Point modelOld;
   Point modelNew;

   // Up vector for gluLookAt()
   Vector up;

   // Current value of step counter
   int steps;

   // Maximum number of steps for a smooth movement
   int maxSteps;
};

#endif
