// Example.cpp - An example of the Particle System API in OpenGL.
//
// Copyright 1999-2006 by David K. McAllister

#include "Particle/pAPI.h"
using namespace PAPI;

#include "GL/glut.h"

ParticleContext_t P;

// A fountain spraying up in the middle of the screen
void ComputeParticles()
{
    // Set up the state.
    P.Velocity(PDCylinder(pVec(0.0f, -0.01f, 0.25f), pVec(0.0f, -0.01f, 0.27f), 0.021f, 0.019f));
    P.Color(PDLine(pVec(0.8f, 0.9f, 1.0f), pVec(1.0f, 1.0f, 1.0f)));

    // Generate particles along a very small line in the nozzle.
    P.Source(100, PDLine(pVec(0, 0, 0), pVec(0, 0, 0.4f)));

    // Gravity.
    P.Gravity(pVec(0, 0, -0.01f));

    // Bounce particles off a disc of radius 5.
    P.Bounce(-0.05f, 0.35f, 0, PDDisc(pVec(0, 0, 0), pVec(0, 0, 1), 5));

    // Kill particles below Z=-3.
    P.Sink(false, PDPlane(pVec(0,0,-3), pVec(0,0,1)));

    // Move particles to their new positions.
    P.Move(true, false);
}

// Draw as points using vertex arrays
// To draw as textured point sprites just call
// glEnable(GL_POINT_SPRITE_ARB) before calling this function.
void DrawGroupAsPoints()
{
    size_t cnt = P.GetGroupCount();
    if(cnt < 1) return;

    float *ptr;
    size_t flstride, pos3Ofs, posB3Ofs, size3Ofs, vel3Ofs, velB3Ofs, color3Ofs, alpha1Ofs, age1Ofs, up3Ofs, rvel3Ofs, upB3Ofs, mass1Ofs, data1Ofs;

    cnt = P.GetParticlePointer(ptr, flstride, pos3Ofs, posB3Ofs,
        size3Ofs, vel3Ofs, velB3Ofs, color3Ofs, alpha1Ofs, age1Ofs,
        up3Ofs, rvel3Ofs, upB3Ofs, mass1Ofs, data1Ofs);
    if(cnt < 1) return;

    glEnableClientState(GL_COLOR_ARRAY);
    glColorPointer(4, GL_FLOAT, int(flstride) * sizeof(float), ptr + color3Ofs);

    glEnableClientState(GL_VERTEX_ARRAY);
    glVertexPointer(3, GL_FLOAT, int(flstride) * sizeof(float), ptr + pos3Ofs);

    glDrawArrays(GL_POINTS, 0, (GLsizei)cnt);
    glDisableClientState(GL_VERTEX_ARRAY);
    glDisableClientState(GL_COLOR_ARRAY);
}

void Draw()
{
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    // Set up the view.
    glLoadIdentity();
    gluLookAt(0, -8, 3, 0, 0, 0, 0, 0, 1);

    // Draw the ground.
    glBegin(GL_QUADS);
    glColor3ub(0, 115, 0);
    glVertex3f(-3.5,-3.5,0);
    glColor3ub(0, 5, 140);
    glVertex3f(-3.5,3.5,0);
    glColor3ub(0, 5, 140);
    glVertex3f(3.5,3.5,0);
    glColor3ub(0, 115, 0);
    glVertex3f(3.5,-3.5,0);
    glEnd();

    // Do what the particles do.
    ComputeParticles();

    // Draw the particles.
    DrawGroupAsPoints();

    glutSwapBuffers();
}

void Reshape(int w, int h)
{
    glViewport(0, 0, w, h);

    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    gluPerspective(40, w / double(h), 1, 100);
    glMatrixMode(GL_MODELVIEW);
}

int main(int argc, char **argv)
{
    // Initialize GLUT.
    glutInit(&argc, argv);

    // Make a normal 3D window.
    glutInitDisplayMode(GLUT_RGB | GLUT_DEPTH | GLUT_DOUBLE);
    glutInitWindowSize(512, 512);
    glutCreateWindow("Particle Example");

    glutDisplayFunc(Draw);
    glutIdleFunc(Draw);
    glutReshapeFunc(Reshape);

    // We want depth buffering, etc.
    glEnable(GL_DEPTH_TEST);
    glDepthFunc(GL_LESS);

    // Make a particle group
    int particle_handle = P.GenParticleGroups(1, 10000);

    P.CurrentGroup(particle_handle);

    try {
        glutMainLoop();
    }
    catch (PError_t &Er) {
        std::cerr << "Particle API exception: " << Er.ErrMsg << std::endl;
        throw Er;
    }

    return 0;
}
