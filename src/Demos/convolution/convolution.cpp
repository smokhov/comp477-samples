// Demonstrate convolution using glConvolutionFilter2D

#define GLEW_STATIC
#include <GL/glew.h>
#include <GL/glut.h>
#include <GL/cugl.h>
#include <iostream>
#include <iomanip>
#include <sstream>
#include <string>
#include <cmath>

using namespace std;
using namespace cugl;

GLsizei  width = 512; // Window size to fit pixel maps
GLsizei  height = 512;
GLubyte  pixels[10000000]; // Storage for pixel maps

// Class for convolution matrices of various sizes.
template<int N>
class Mat
{
public:

   // Default constructor yields 'plain' filter.
   Mat()
   {
      for (int r = 0; r < N; ++r)
         for (int c = 0; c < N; ++c)
            data[r][c] = (r == N/2 && c == N/2) ? 1 : 0;
   }

   // Copy constructor.
   Mat(const Mat & m)
   {
      for (int r = 0; r < N; ++r)
         for (int c = 0; c < N; ++c)
            data[r][c] = m.data[r][c];
   }

   // Construct from given array.
   Mat(GLfloat vals[N][N])
   {
      for (int r = 0; r < N; ++r)
         for (int c = 0; c < N; ++c)
            data[r][c] = vals[r][c];
   }

   // Apply the convolution filter to the current display.
   void apply()
   {
      glConvolutionFilter2D(GL_CONVOLUTION_2D, GL_LUMINANCE, N, N, GL_LUMINANCE, GL_FLOAT, data);
   }

   // Show matrix values in CL window.
   // Showing in graphics window doesn't work: glutBitmapCharacter crashes!
   void showMatrix()
   {
      GLfloat *m =&data[0][0];
      ostringstream os;
      os << fixed << setprecision(2);
      for (int r = 0; r < N; ++r)
      {
         for (int c = 0; c < N; ++c)
            os << setw(6) << *m++;
         os << endl;
      }
      cout << os.str() << endl << endl;
   }

private:
   GLfloat data[N][N];
};

// Create an initialize 'plain' filters: 3x3, 5x5, and 7x7.
int dim = 3;
Mat<3> m3;
Mat<5> m5;
Mat<7> m7;

// Image index and store.
const int NUM_IMAGES = 6;
int imx = 0;
PixelMap images[NUM_IMAGES];

// Display and image and filter it according to filter dimensions.
void display(void)
{
   glClear(GL_COLOR_BUFFER_BIT);
   glLoadIdentity();
   glEnable(GL_CONVOLUTION_2D);

   switch (dim)
   {
      case 3:
         m3.apply();
         break;
      case 5:
         m5.apply();
         break;
      case 7:
         m7.apply();
         break;
      default:
         cerr << "Invalid dimension.\n";
         break;
   }

   glRasterPos2i(0,0);
   images[imx].draw();
   glFlush();
}

// Reshape doesn't have much effect because we are using pixel maps of fixed size.
void reshape(int w, int h)
{
   glViewport(0, 0, (GLsizei) w, (GLsizei) h);
   glMatrixMode(GL_PROJECTION);
   glLoadIdentity();
   glOrtho(0, w, 0, h, -1, 1);
   glMatrixMode(GL_MODELVIEW);
}

// Press space to cycle through images.
void keyboard(unsigned char key, int x, int y)
{
   switch (key)
   {
         // Next image
      case ' ':
         imx = (imx + 1) % NUM_IMAGES;
         break;

      case 'q':
      case 27:
         exit(0);
   }
   glutPostRedisplay();
}

// Varieties of filter.
enum Kind
{
   PLAIN3, LINEDET3, EDGEDET3, EMBOSS3, LAPLACE3, BLUR3, GRAD3, SPREAD3,
   X_EDGE_DET_5, Y_EDGE_DET_5, BLUR5, LAPLACE5, SHARP5, EMBOSS5,
   MOTBLUR7, BOX7, GAUSS7
};

// Select filter from menu.
void menuHandler(int sel)
{
   switch (sel)
   {
      case PLAIN3:
         {
            dim = 3;
            m3 = Mat<3>();
            m3.showMatrix();
            break;
         }

      case LINEDET3:
         {
            GLfloat lineDet[3][3] =
            {
               {  -1, -1, -1 },
               {  -1,  8, -1 },
               {  -1, -1, -1 }
            };
            dim = 3;
            m3 = Mat<3>(lineDet);
            m3.showMatrix();
            break;
         }

      case EDGEDET3:
         {
            GLfloat edgeDet[3][3] =
            {
               { 0,  1, 0 },
               { 1, -4, 1 },
               { 0,  1, 0 }
            };
            dim = 3;
            m3 = Mat<3>(edgeDet);
            m3.showMatrix();
            break;
         }

      case EMBOSS3:
         {
            GLfloat emboss[3][3] =
            {
               {  -2, -1, 0 },
               {  -1,  1, 1 },
               {  0, 1, 2 }
            };
            dim = 3;
            m3 = Mat<3>(emboss);
            m3.showMatrix();
            break;
         }

      case LAPLACE3:
         {
            GLfloat  laplace[3][3] =
            {
               { -0.125, -0.125, -0.125 },
               { -0.125,  1.0  , -0.125 },
               { -0.125, -0.125, -0.125 },
            };
            dim = 3;
            m3 = Mat<3>(laplace);
            m3.showMatrix();
            break;
         }

      case BLUR3:
         {
            const double B = 1.0 / 5.0;
            GLfloat  blur[3][3] =
            {
               { 0, B, 0 },
               { B, B, B },
               { 0, B, 0 },
            };
            dim = 3;
            m3 = Mat<3>(blur);
            m3.showMatrix();
            break;
         }

      case GRAD3:
         {
            GLfloat  grad[3][3] =
            {
               { -1, -2, -1 },
               {  0,  0,  0 },
               {  1,  2,  1 },
            };
            dim = 3;
            m3 = Mat<3>(grad);
            m3.showMatrix();
            break;
         }

      case SPREAD3:
         {
            GLfloat  spread[3][3] =
            {
               { -0.627,  0.352, -0.627 },
               {  0.352,  2.923,  0.352 },
               { -0.627,  0.352, -0.627 }
            };
            dim = 3;
            m3 = Mat<3>(spread);
            m3.showMatrix();
            break;
         }

      case X_EDGE_DET_5:
         {
            GLfloat vEdgeDet[5][5] =
            {
               { 0, 0, -1, 0, 0 },
               { 0, 0, -1, 0, 0 },
               { 0, 0,  4, 0, 0 },
               { 0, 0, -1, 0, 0 },
               { 0, 0, -1, 0, 0 }
            };
            dim = 5;
            m5 = Mat<5>(vEdgeDet);
            m5.showMatrix();
            break;
         }

      case Y_EDGE_DET_5:
         {
            GLfloat hEdgeDet[5][5] =
            {
               {  0,   0, 0, 0, 0 },
               {  0,   0, 0, 0, 0 },
               { -1,  -1, 2, 0, 0 },
               {  0,   0, 0, 0, 0 },
               {  0,   0, 0, 0, 0 }
            };
            dim = 5;
            m5 = Mat<5>(hEdgeDet);
            m5.showMatrix();
            break;
         }

      case BLUR5:
         {
            const double B = 1.0 / 13.0;
            GLfloat blur[5][5] =
            {
               { 0, 0, B, 0, 0 },
               { 0, B, B, B, 0 },
               { B, B, B, B, B },
               { 0, B, B, B, 0 },
               { 0, 0, B, 0, 0 }
            };
            dim = 5;
            m5 = Mat<5>(blur);
            m5.showMatrix();
            break;
         }

      case LAPLACE5:
         {
            const double F = 17.0;
            GLfloat lap[5][5] =
            {
               {  0/F,  0/F, -1/F,  0/F,  0/F },
               {  0/F, -1/F, -2/F, -1/F,  0/F },
               { -1/F, -2/F, 17/F, -2/F, -1/F },
               {  0/F, -1/F, -2/F, -1/F,  0/F },
               {  0/F,  0/F, -1/F,  0/F,  0/F }
            };
            dim = 5;
            m5 = Mat<5>(lap);
            m5.showMatrix();
            break;
         }

      case SHARP5:
         {
            const double F = 21.0;
            GLfloat sharp[5][5] =
            {
               { -1/F, -3/F, -4/F, -3/F, -1/F },
               { -3/F,  0/F,  6/F,  0/F, -3/F },
               { -4/F,  6/F, 21/F,  6/F, -4/F },
               { -3/F,  0/F,  6/F,  0/F, -3/F },
               { -1/F, -3/F, -4/F, -3/F, -1/F }
            };
            dim = 5;
            m5 = Mat<5>(sharp);
            m5.showMatrix();
            break;
         }

      case EMBOSS5:
         {
            GLfloat emboss[5][5] =
            {
               { -1, -1, -1, -1,  0 },
               { -1, -1, -1,  0,  1 },
               { -1, -1,  0,  1,  1 },
               { -1,  0,  1,  1,  1 },
               {  0,  1,  1,  1,  1 }
            };
            dim = 5;
            m5 = Mat<5>(emboss);
            m5.showMatrix();
            break;
         }

      case MOTBLUR7:
         {
            const double B = 1.0 / 7.0;
            GLfloat motblur[7][7] =
            {
               { B, 0, 0, 0, 0, 0, 0 },
               { 0, B, 0, 0, 0, 0, 0 },
               { 0, 0, B, 0, 0, 0, 0 },
               { 0, 0, 0, B, 0, 0, 0 },
               { 0, 0, 0, 0, B, 0, 0 },
               { 0, 0, 0, 0, 0, B, 0 },
               { 0, 0, 0, 0, 0, 0, B },
            };
            dim = 7;
            m7 = Mat<7>(motblur);
            m7.showMatrix();
            break;
         }

      case BOX7:
         {
            const GLfloat B = 1.0 / 49.0;
            GLfloat box[7][7] =
            {
               {B,B,B, B, B,B,B},
               {B,B,B, B, B,B,B},
               {B,B,B, B, B,B,B},
               {B,B,B, B, B,B,B},
               {B,B,B, B, B,B,B},
               {B,B,B, B, B,B,B},
               {B,B,B, B, B,B,B}
            };
            dim = 7;
            m7 = Mat<7>(box);
            m7.showMatrix();
            break;
         }

      case GAUSS7:
         {
            GLfloat gauss[7][7];
            dim = 7;

            const double K = -0.1;
            for (int r = 0; r < dim; ++r)
            {
               double x = r - 3;
               for (int c = 0; c < dim; ++c)
               {
                  double y = c - 3;
                  gauss[r][c] = exp(K * (sqr(x) + sqr(y)));
               }
            }

            double sum = 0;
            for (int r = 0; r < dim; ++r)
               for (int c = 0; c < dim; ++c)
                  sum += gauss[r][c];

            for (int r = 0; r < dim; ++r)
               for (int c = 0; c < dim; ++c)
                  gauss[r][c] /= sum;

            m7 = Mat<7>(gauss);
            m7.showMatrix();
            break;
         }
   }
   glutPostRedisplay();
}

int main(int argc, char** argv)
{
   cout <<
   "Convolution using OpenGL\n\n"
   "Use right mouse button and menu to select filter.\n"
   "Press space to cycle through images.\n\n";

   glutInit(&argc, argv);
   glutInitDisplayMode(GLUT_SINGLE | GLUT_RGB);
   glutInitWindowSize(width, height);
   glutInitWindowPosition(200, 0);
   glutCreateWindow("Convolution");
   glutReshapeFunc(reshape);
   glutKeyboardFunc(keyboard);
   glutDisplayFunc(display);

   // 3x3 filters.
   int m33 = glutCreateMenu(menuHandler);
   glutAddMenuEntry("Plain", PLAIN3);
   glutAddMenuEntry("Line detect", LINEDET3);
   glutAddMenuEntry("Edge detect", EDGEDET3);
   glutAddMenuEntry("Blur", BLUR3);
   glutAddMenuEntry("Laplace", LAPLACE3);
   glutAddMenuEntry("Emboss", EMBOSS3);
   glutAddMenuEntry("Gradient", GRAD3);
   glutAddMenuEntry("Spread", SPREAD3);

   // 5x5 filters.
   int m55 = glutCreateMenu(menuHandler);
   glutAddMenuEntry("X edge detect", X_EDGE_DET_5);
   glutAddMenuEntry("Y edge detect", Y_EDGE_DET_5);
   glutAddMenuEntry("Blur", BLUR5);
   glutAddMenuEntry("Laplace", LAPLACE5);
   glutAddMenuEntry("Emboss", EMBOSS5);
   glutAddMenuEntry("Sharpen", SHARP5);

   // 7x7 filters.
   int m77 = glutCreateMenu(menuHandler);
   glutAddMenuEntry("Motion blur", MOTBLUR7);
   glutAddMenuEntry("Box", BOX7);
   glutAddMenuEntry("Gaussian", GAUSS7);

   glutCreateMenu(menuHandler);
   glutAttachMenu(GLUT_RIGHT_BUTTON);
   glutAddSubMenu("3 x 3", m33);
   glutAddSubMenu("5 x 5", m55);
   glutAddSubMenu("7 x 7", m77);

   glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
   glPixelStorei(GL_PACK_ALIGNMENT, 1);
   glClearColor(0, 0, 0, 0);

   // Load images.
   images[0].read("house.bmp");
   images[1].read("machine.bmp");
   images[2].read("kayak.bmp");
   images[3].read("pepper.bmp");
   images[4].read("church.bmp");
   images[5].read("circuit.bmp");

   GLuint error = glewInit();
   if (error)
      cout << error << " => " << glewGetErrorString(error) << endl;
   else
      glutMainLoop();
   return 0;
}

