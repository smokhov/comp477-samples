#include "camera.h"

const Point EYE(0, 0, 1);
const Point MODEL(0, 0, 0);
const Vector UP(0, 1, 0);

Camera::Camera() :
   eye(EYE), 
   eyeOld(EYE), 
   eyeNew(EYE), 
   model(MODEL), 
   modelOld(MODEL), 
   modelNew(MODEL), 
   up(UP),
   steps(0),
   maxSteps(50)
{ }

Camera::Camera(Point eye, Point model, Vector up) : 
   eye(eye), 
   eyeOld(eye), 
   eyeNew(eye), 
   model(model), 
   modelOld(model), 
   modelNew(model), 
   up(up.unit()),
   steps(0),
   maxSteps(50)
{ }

Camera::Camera(Point eye, Point model) : 
   eye(eye), 
   eyeOld(eye), 
   eyeNew(eye), 
   model(model), 
   modelOld(model), 
   modelNew(model), 
   up(J),
   steps(0),
   maxSteps(50)
{ }

Camera::Camera(Point eye) : 
   eye(eye), 
   eyeOld(eye), 
   eyeNew(eye), 
   model(MODEL), 
   modelOld(MODEL), 
   modelNew(MODEL), 
   up(J),
   steps(0),
   maxSteps(50)
{ }

void Camera::set(Point e, Point m, Vector u)
{
   eye = e;
   eyeOld = e;
   eyeNew = e;
   model = m;
   modelOld = m;
   modelNew = m;
   up = u.unit();
}

void Camera::set(Point e, Point m)
{
   eye = e;
   eyeOld = e;
   eyeNew = e;
   model = m;
   modelOld = m;
   modelNew = m;
   up = J;
}

void Camera::set(Point e)
{
   eye = e;
   eyeOld = e;
   eyeNew = e;
   model = MODEL;
   modelOld = MODEL;
   modelNew = MODEL;
   up = J;
}

void Camera::setSteps(int s)
{
   maxSteps = s;
}

void Camera::apply()
{
   gluLookAt
   (
      eye[0], eye[1], eye[2], 
      model[0], model[1], model[2], 
      up[0], up[1], up[2]
   );
}

void Camera::moveUp(GLfloat distance)
{
   Vector disp = distance * up;
   eyeNew = eyeOld + disp;
   modelNew = modelOld + disp;
   if (steps > 0)
   {
      eyeOld = eye;
      modelOld = model;
   }
   steps = maxSteps;
}

void Camera::moveForward(GLfloat distance)
{
   Vector disp = distance * (model - eye).unit();
   eyeNew = eyeOld + disp;
   modelNew = modelOld + disp;
   if (steps > 0)
   {
      eyeOld = eye;
      modelOld = model;
   }
   steps = maxSteps;
}

void Camera::moveLeft(GLfloat distance)
{
   Vector disp = distance * cross(up, model - eye).unit();
   eyeNew = eyeOld + disp;
   modelNew = modelOld + disp;
   if (steps > 0)
   {
      eyeOld = eye;
      modelOld = model;
   }
   steps = maxSteps;
}

void Camera::tiltUp(double angle)
{
   Vector axis = cross(eyeOld - modelOld, up).unit();
   Quaternion rot(axis, angle);
   eyeNew = eyeOld;
   modelNew = eyeOld + rot.apply(modelOld - eyeOld);
   if (steps > 0)
   {
      eyeOld = eye;
      modelOld = model;
   }
   steps = maxSteps;
}

void Camera::panLeft(double angle)
{
   Quaternion rot(up, -angle);
   eyeNew = eyeOld;
   modelNew = eyeOld + rot.apply(modelOld - eyeOld);
   if (steps > 0)
   {
      eyeOld = eye;
      modelOld = model;
   }
   steps = maxSteps;
}

void Camera::idle()
{
   if (steps > 0)
   {
      GLfloat t = GLfloat(steps) / GLfloat(maxSteps);
      eye = eyeNew + t * (eyeOld - eyeNew);
      model = modelNew + t * (modelOld - modelNew);
      steps--;
   }
   else
   {
      eyeOld = eyeNew;
      modelOld = modelNew;
      steps = 0;
   }
}

ostream & operator<<(ostream & os, Camera & c)
{
   return os << 
      "Eye: " << c.eye <<
      "  Model: " << c.model << 
      "  Up: " << c.up << endl;
}

