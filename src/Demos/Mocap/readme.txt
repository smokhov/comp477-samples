

                 W X   M o t i o n   V i e w e r

                    by Jaroslav Semancik, 2004

                    Charles University, Prague


WXMV is a program for viewing, blending, interpolating and analysis of
BVH (Biovision Hierarchy) motion files. It is written in C++ with use
of STL, wxWindows for graphical user interface and OpenGL for
visualization of skeletons. Thus the program is rather portable and can
be compiled for any common platform (maybe after some minor adjustments
of the source). Free multiplatform GUI toolkit wxWindows
(www.wxwindows.org) provides a native look on each platform. Biovision
Hierarchy file format is described in file bvh.txt.

Real-time visualization of the skeletons motion is performed by OpenGL.
The whole scene can be translated, rotated or scaled by mouse movements
and buttons.

Export to popular free raytracer POV-Ray (www.povray.org) is
recommended for final rendering of an animation. Export to a CSV file
(Comma-Separated Values) is useful for analysis - the file may be
imported by standard office packages and visualized as a graph. Export
to a batch file prepares a script for use with bvhcopy utility to
decompose a walking motion to individual steps.

Changes from previous versions are listed in a separate file changes.txt.


    /// Opening and closing BVH files ///

Any number of motion files can be opened and played at once. You can
select multiple motion files in the Open dialog to load. The files may
have different frame rate, but frames are interpolated to a continuous
signal and played at 30 fps. Note that BVH files are required to have
correct "Frames:" value and only files with one hierarchy and joint
with 3 (rotations in ZXY order) or 6 (translations in XYZ order
followed by rotation in ZXY order) channels are opened.

To close some motions select them in the file panel and choose
File->"Close selected".


    /// Motion blending and interpolation ///

Select motions to blend in the file panel and choose Animation->"Blend
selected". A new motion will be created averaging all selected motions
with uniform weights summing to 1. Note that also duration of motions
is averaged.

Select two motions to interpolate (if more motions are selected only
the first two are used) and choose Animation->"Interpolate two
selected". A new motion will be created linearly interpolating them. It
smoothly transforms from the first motion to the second one having
average duration of the two motions.

Note that only shots with identical skeleton can be blended and 
interpolated.

Joint rotations are represented and blended using quaternions.


    /// Exports ///

In menu Animation select Export and choose a desired format. All
motions checked in the file panel will be exported.

/* POV-Ray */

Enter name of a main include file of the animation. The main include
file is produced which you include in your POV-Ray scene. Besides, a
separate include files for each frame of each checked shot is generated,
they are included by the main file automatically when rendering
corresponding frame. The skeletons are exported as POV-Ray blobs
ensuring continuous surface and no cracks around joints. Blob threshold,
joint bulginess and textures can be modified in the main include file.
A camera, lights and other scene settings are all specified in your
scene.

Note that also a POV-Ray .ini file is generated to control the
animation. You must set it as an .ini file for the scene in your
POV-Ray environment before rendering. It contains some basic rendering
settings and you can edit it to your needs.

The exported animation will have length of the longest exported shot.
The other shots will be wrapped around or stopped beyond their duration
depending on the "Cycle animation" setting in menu.

/* Comma-separated values */

Exports coordinates and velocity of chosen joints. The joints and
coordinates to export specify in the View->"Settings for export" dialog.
Joints are separated by space. Possible coordinates are x, y, z or v
standing for velocity of the joint. World coordinates of the chosen
joints are written to the file in same units as in source BVH file. CSV
is a very simple self-explaining format. Standard office packages like
Open Office Calc or Microsoft Office Excel can be sed to import CSV
files analyze them and draw graphs.

/* Batch file */

WXMV is able to decompose walking motions to individual step motions.
Step boundaries are detected in the middle of a double-support phase
(interval when both feet are planted = fixed on the floor). A batch
file is produced which uses a bvhcopy utility to extract a respective
BVH file for each step from an original BVH file of the walking motion.
Time interval for each step as well as initial length and final length
of the step (distance between toes in the beginning and in the end of
the step) are written as comments to the batch file.

To generate the batch file set the View->"Settings for export" first.
Specify names of joints representing left and right toes end-effector,
and parameters to detect whether a foot is planted or not in a
particular time. A foot end-effector is considered as planted when its
velocity is lesser than Velocity limit and its height is lesser than
Height limit. Units are same as in the source BVH file. Velocity of a
joint is computed by forward reference using time interval of Velocity
delta legth. Then choose Animation->Export and batch file format.

To use the generated batch file, place it with a BVH file of the
walking motion and bvhcopy utility to same directory and run.


    /// Bvhcopy utility ///
    
The bvhcopy is a commandline utility to extract a part of a BVH file.
The part can be specified by frame numbers, seconds or percentage of the
whole motion. Run bvhcopy without arguments to see the usage.


    /// Source code and compilation comments ///

The source is organized as follows:

wxmv.h, wxmv.cpp         - main file, contains wxWindows user interface,
                           application and main frame classes
export.cpp               - export related methods of the main frame class
skeleton.h, skeleton.cpp - skeleton and shot data structures and methods
motion.h, motion.cpp     - motion data structures and methods
bvh.h, bvh.cpp           - loading and parsing of input .bvh file
ogl.h, ogl.cpp           - visualization using OpenGL
algebra.h, algebra.cpp   - classes for vectors quaternions and matrices,
                           with overloaded operators
base.h, base.cpp         - fundamental staff useful for all modules
wxmv.rc                  - resources description on MS Windows
makefile.wx              - makefile (for a wxWindows program)
bvh.txt                  - BVH file format description
resources/               - icons
dialogs/                 - .wxr sources of the "Setting for export" dialog
                           (unused in compiling)
bvhcopy/                 - the bvhcopy utility

To compile the source code you need to install and compile wxWindows
with enabled use of menus, glCanvas and timer (and also toolbar and
statusbar are recommended) in wxWindows setup.h of your platform. Then
make sure that compiler options are set to enable exceptions in an .env
file (in wxWindows ./src directory) that is dependent on compiler you
plan to use. Finally edit the makefile.wx for directories and to
include a proper makefile from wxWindows installation according to your
compiler.

Note that the standard C++ string class is used for strings in the
program preventing to take care about string lengths. The wxWindows
stuff is only in the application classes in wxmv.h, wxmv.cpp and
export.cpp files. All other files uses only ANSI C++, STL and OpenGL.

