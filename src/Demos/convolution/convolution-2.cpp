#define GLEW_STATIC
#include <GL/glew.h>
#include <GL/glut.h>
#include <GL/cugl.h>
#include <iostream>
using namespace std;
using namespace cugl;

GLubyte  pixels[10000000];
GLsizei   width = 512;
GLsizei   height = 512;

GLfloat  big[3][3][4] =
{
   { {0,0,0,0},  {0,0,0,0}, {0,0,0,0} },
   { {0,0,0,0},  {1,1,1,1}, {0,0,0,0} },
   { {0,0,0,0},  {0,0,0,0}, {0,0,0,0} }
};

GLfloat  plain1[1][1] =
{ { 1 } };

GLfloat  plain3[3][3] =
{
   { 0,  0, 0 },
   { 0,  1, 0 },
   { 0,  0, 0 }
};

GLfloat  horizontal[3][3] =
{
   { 0, -0.8, 0 },
   { 0,  1, 0 },
   { 0,  0, 0 }
};

GLfloat  vertical[3][3] =
{
   {  0, 0, 0 },
   { -1, 1, 0 },
   {  0, 0, 0 }
};

GLfloat  laplacian[3][3] =
{
   { -0.125, -0.125, -0.125 },
   { -0.125,  1.0  , -0.125 },
   { -0.125, -0.125, -0.125 },
};

GLfloat b = 1.0 / 49.0;
GLfloat box[7][7] =
{
   {b,b,b, b, b,b,b},
   {b,b,b, b, b,b,b},
   {b,b,b, b, b,b,b},
   {b,b,b, b, b,b,b},
   {b,b,b, b, b,b,b},
   {b,b,b, b, b,b,b},
   {b,b,b, b, b,b,b}
};

GLfloat gauss[7][7] =
{
   { 0.00183 , 0.00497 , 0.00905 , 0.01106 , 0.00905 , 0.00497 , 0.00183 },
   { 0.00497 , 0.01350 , 0.02460 , 0.03005 , 0.02460 , 0.01350 , 0.00497 },
   { 0.00905 , 0.02460 , 0.04483 , 0.05476 , 0.04483 , 0.02460 , 0.00905 },
   { 0.01106 , 0.03005 , 0.05476 , 0.06688 , 0.05476 , 0.03005 , 0.01106 },
   { 0.00905 , 0.02460 , 0.04483 , 0.05476 , 0.04483 , 0.02460 , 0.00905 },
   { 0.00497 , 0.01350 , 0.02460 , 0.03005 , 0.02460 , 0.01350 , 0.00497 },
   { 0.00183 , 0.00497 , 0.00905 , 0.01106 , 0.00905 , 0.00497 , 0.00183 }
};

GLfloat lineDet[3][3] =
{
   {  -1, -1, -1 },
   {  -1,  8, -1 },
   {  -1, -1, -1 }
};

GLfloat edgeDet[3][3] =
{
   { 0,  1, 0 },
   { 1, -4, 1 },
   { 0,  1, 0 }
};

GLfloat emboss[3][3] =
{
   {  -2, -1, 0 },
   {  -1,  1, 1 },
   {  0, 1, 2 }
};

void check()
{
   GLenum error = glGetError();
   if (error > 0)
      cerr << "GL error " << error << endl;
}

PixelMap images[5];;

char mode = '0';
int imx = 0;

void display(void)
{
   glClear(GL_COLOR_BUFFER_BIT);
   glLoadIdentity();

   glEnable(GL_CONVOLUTION_2D);
   string desc;
   switch (mode)
   {
      case '0':
         glDisable(GL_CONVOLUTION_2D);
         desc = "No convolution";
         break;

      case '1' :
         glConvolutionFilter2D(GL_CONVOLUTION_2D, GL_LUMINANCE, 1, 1, GL_LUMINANCE, GL_FLOAT, plain1);
         desc = "1 x 1 plain";
         break;

      case '2' :
         glConvolutionFilter2D(GL_CONVOLUTION_2D, GL_LUMINANCE, 3, 3, GL_LUMINANCE, GL_FLOAT, plain3);
         desc = "3 x 3 plain";
         break;

      case '3' :
         glConvolutionFilter2D(GL_CONVOLUTION_2D, GL_LUMINANCE, 3, 3, GL_LUMINANCE, GL_FLOAT, horizontal);
         desc = "3 x 3 horizontal";
         break;

      case '4' :
         glConvolutionFilter2D(GL_CONVOLUTION_2D, GL_LUMINANCE, 3, 3, GL_LUMINANCE, GL_FLOAT, vertical);
         desc = "3 x 3 vertical";
         break;

      case '5' :
         glConvolutionFilter2D(GL_CONVOLUTION_2D, GL_LUMINANCE, 3, 3, GL_LUMINANCE, GL_FLOAT, laplacian);
         desc = "3 x 3 Laplacian";
         break;

      case '6' :
         glConvolutionFilter2D(GL_CONVOLUTION_2D, GL_LUMINANCE, 7, 7, GL_LUMINANCE, GL_FLOAT, box);
         desc = "7 x 7 box";
         break;

      case '7' :
         glConvolutionFilter2D(GL_CONVOLUTION_2D, GL_LUMINANCE, 7, 7, GL_LUMINANCE, GL_FLOAT, gauss);
         desc = "7 x 7 Gaussian";
         break;

      case '8' :
         glConvolutionFilter2D(GL_CONVOLUTION_2D, GL_LUMINANCE, 3, 3, GL_LUMINANCE, GL_FLOAT, emboss);
         desc = "3 x 3 experiment";
         break;
   }
   glRasterPos2i(0,0);
   images[imx].draw();
   check();
   glFlush();
   glutSetWindowTitle(desc.c_str());

}

void reshape(int w, int h)
{
   glViewport(0, 0, (GLsizei) w, (GLsizei) h);
   glMatrixMode(GL_PROJECTION);
   glLoadIdentity();
   glOrtho(0, w, 0, h, -200, 200);
   glMatrixMode(GL_MODELVIEW);
}

void keyboard(unsigned char key, int x, int y)
{
   switch (key)
   {
      case '1':
      case '2':
      case '3':
      case '4':
      case '5':
      case '6':
      case '7':
      case '8':
         mode = key;
         break;

      case ' ':
         imx = (imx + 1) % 5;
         break;

      case 'q':
      case 27:
         exit(0);
   }
   glutPostRedisplay();
}

int main(int argc, char** argv)
{
   glutInit(&argc, argv);
   glutInitDisplayMode(GLUT_SINGLE | GLUT_RGB);
   glutInitWindowSize(width, height);
   glutInitWindowPosition(100, 100);
   glutCreateWindow("Convolution");
   glutReshapeFunc(reshape);
   glutKeyboardFunc(keyboard);
   glutDisplayFunc(display);
   glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
   glPixelStorei(GL_PACK_ALIGNMENT, 1);
   glClearColor(0, 0, 0, 0);
   GLuint error = glewInit();
   images[0].read("house.bmp");
   images[1].read("machine.bmp");
   images[2].read("kayak.bmp");
   images[3].read("pepper.bmp");
   images[4].read("church.bmp");
   if (error)
      cout << error << " => " << glewGetErrorString(error) << endl;
   else
      glutMainLoop();
   return 0;
}

