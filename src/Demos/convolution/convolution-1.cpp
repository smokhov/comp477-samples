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
   { 0, -1, 0 },
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

GLfloat experiment[3][3] =
{
   {  0, 0, 0 },
   { 0.5, 0, 0.5 },
   {  0, 0, 0 }
};

PixelMap map;

void check()
{
   GLenum error = glGetError();
   if (error > 0)
      cerr << "GL error " << error << endl;
}

char mode = '0';
char shape = 'w';

void display(void)
{
   glDisable(GL_CONVOLUTION_2D);
   glClear(GL_COLOR_BUFFER_BIT);
   glLoadIdentity();

   glColor3d(1, 0, 0);
   glBegin(GL_LINES);
   for (int x = 0; x < 5; ++x)
   {
      glVertex3d(0.2 * x * width, 0, 0);
      glVertex3d(0.2 * x * width, height, 0);
   }
   for (int y = 0; y < 5; ++y)
   {
      glVertex3d(0,     0.2 * y * height, 0);
      glVertex3d(width, 0.2 * y * height, 0);
   }
   glEnd();

   glColor3d(0, 1, 0);
   glTranslated(0.5*width, 0.5*height, 0);
   switch (shape)
   {
      case 'h':
         glRasterPos2i(-256, -256);
         map.draw();
         break;
      case 's':
         glutSolidSphere(150, 50, 50);
         break;
      case 'w':
         glutWireSphere(150, 10, 10);
         break;
      case 't':
         glutSolidTeapot(100);
         break;
   }

   glReadPixels(0, 0, width, height, GL_RGB, GL_UNSIGNED_BYTE, pixels);
   check();
   glClear(GL_COLOR_BUFFER_BIT);
   glLoadIdentity();
   glRasterPos2i(0, 0);
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
         glConvolutionFilter2D(GL_CONVOLUTION_2D, GL_LUMINANCE, 3, 3, GL_LUMINANCE, GL_FLOAT, experiment);
         desc = "3 x 3 experiment";
         break;
   }
   glDrawPixels(width, height, GL_RGB, GL_UNSIGNED_BYTE, pixels);
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

      case 'h':
      case 's':
      case 't':
      case 'w':
         shape = key;
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
   map.read("house.bmp");
   if (error)
      cout << error << " => " << glewGetErrorString(error) << endl;
   else
      glutMainLoop();
   return 0;
}

