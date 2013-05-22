/*
 *  CPE 474 lab 0 - modern graphics test bed
 *  draws a partial cube using a VBO and IBO
 *  glut/OpenGL/GLSL application
 *  Uses glm and local matrix stack
 *  to handle matrix transforms for a view matrix, projection matrix and
 *  model transform matrix
 *
 *  zwood 9/12
 *  Copyright 2012 Cal Poly. All rights reserved.
 *
 *****************************************************************************/

#ifdef __APPLE__
#include "GLUT/glut.h"
#include <OPENGL/gl.h>
#endif

#ifdef __unix__
#include <GL/glut.h>
#endif

#include <iostream>
#include <string>
#include <math.h>

#include "glm/glm.hpp"
#include "glm/gtc/matrix_transform.hpp"
#include "glm/gtc/type_ptr.hpp"

#include "GLSL_helper.h"
#include "MStackHelp.h"

#include "GeometryCreator.h"

#define PI 3.14159

using namespace std;
using namespace glm;


////////////////
// Globals :( //
////////////////

// Parameters
unsigned int const StepSize = 10;
unsigned int WindowWidth = 1600, WindowHeight = 900;

// Meshes
unsigned int const MeshCount = 1;
Mesh * Meshes[MeshCount];
unsigned int CurrentMesh;

// Variable Handles
GLuint aPosition;
GLuint aNormal;
GLuint uModelMatrix;
GLuint uNormalMatrix;
GLuint uViewMatrix;
GLuint uProjMatrix;
GLuint uColor;

// Shader Handle
GLuint ShadeProg;


// Program Variables
float Accumulator;
float CameraHeight;

float obeta = (PI/2.0);
float oalpha = 0;
float beta = 0;
float alpha = 0;

int g_basex;
int g_basey;

vec3 eyePos(0,0,5);
vec3 lookAtPos(0,0,0);
vec3 olookAtPos(0,0,1);
vec3 up(0,1,0);
vec3 change(0,0,0);

vec3 w(0,0,0);
vec3 u(0,0,0);
vec3 v(0,0,0);


RenderingHelper ModelTrans;

void SetProjectionMatrix()
{
   glm::mat4 Projection = glm::perspective(80.0f, ((float) WindowWidth)/ ((float)WindowHeight), 0.1f, 100.f);
   safe_glUniformMatrix4fv(uProjMatrix, glm::value_ptr(Projection));
}

void SetView()
{
   glm::mat4 View = glm::lookAt(eyePos, lookAtPos, up);
   safe_glUniformMatrix4fv(uViewMatrix, glm::value_ptr(View));
}

void SetModel()
{
   safe_glUniformMatrix4fv(uModelMatrix, glm::value_ptr(ModelTrans.modelViewMatrix));
   safe_glUniformMatrix4fv(uNormalMatrix, glm::value_ptr(glm::transpose(glm::inverse(ModelTrans.modelViewMatrix))));
}

bool InstallShader(char* vShaderName, char* fShaderName)
{
   GLuint VS; // handles to shader object
   GLuint FS; // handles to frag shader object
   GLint vCompiled, fCompiled, linked; // status of shader

   VS = glCreateShader(GL_VERTEX_SHADER);
   FS = glCreateShader(GL_FRAGMENT_SHADER);

   // load the source
   char const * vSource = textFileRead(vShaderName);
   char const * fSource = textFileRead(fShaderName);
   glShaderSource(VS, 1, & vSource, NULL);
   glShaderSource(FS, 1, & fSource, NULL);

   // compile shader and print log
   glCompileShader(VS);
   printOpenGLError();
   glGetShaderiv(VS, GL_COMPILE_STATUS, & vCompiled);
   printShaderInfoLog(VS);

   // compile shader and print log
   glCompileShader(FS);
   printOpenGLError();
   glGetShaderiv(FS, GL_COMPILE_STATUS, & fCompiled);
   printShaderInfoLog(FS);

   if (! vCompiled || ! fCompiled)
   {
      std::cerr << "Error compiling either shader " << vShaderName << " or " << fShaderName << std::endl;
      return false;
   }

   // create a program object and attach the compiled shader
   ShadeProg = glCreateProgram();
   glAttachShader(ShadeProg, VS);
   glAttachShader(ShadeProg, FS);

   glLinkProgram(ShadeProg);

   // check shader status requires helper functions
   printOpenGLError();
   glGetProgramiv(ShadeProg, GL_LINK_STATUS, &linked);
   printProgramInfoLog(ShadeProg);

   glUseProgram(ShadeProg);

   // get handles to attribute data
   aPosition   = safe_glGetAttribLocation(ShadeProg, "aPosition");
   aNormal     = safe_glGetAttribLocation(ShadeProg, "aNormal");

   uColor          = safe_glGetUniformLocation(ShadeProg, "uColor");
   uProjMatrix     = safe_glGetUniformLocation(ShadeProg, "uProjMatrix");
   uViewMatrix     = safe_glGetUniformLocation(ShadeProg, "uViewMatrix");
   uModelMatrix    = safe_glGetUniformLocation(ShadeProg, "uModelMatrix");
   uNormalMatrix   = safe_glGetUniformLocation(ShadeProg, "uNormalMatrix");

   std::cout << "Sucessfully installed shader " << ShadeProg << std::endl;
   return true;
}

void Initialize()
{
   glClearColor(0.8f, 0.8f, 1.0f, 1.0f);

   glClearDepth(1.0f);
   glDepthFunc(GL_LEQUAL);
   glEnable(GL_DEPTH_TEST);

   glEnable(GL_CULL_FACE);
   glCullFace(GL_BACK);

   ModelTrans.useModelViewMatrix();
   ModelTrans.loadIdentity();
}

void Draw()
{
   glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

   glUseProgram(ShadeProg);

   SetProjectionMatrix();
   SetView();

   ModelTrans.loadIdentity();

   ModelTrans.pushMatrix();

   ModelTrans.rotate(Accumulator * 35.f, vec3(0, 1, 0));
   SetModel();

   safe_glEnableVertexAttribArray(aPosition);
   glBindBuffer(GL_ARRAY_BUFFER, Meshes[CurrentMesh]->PositionHandle);
   safe_glVertexAttribPointer(aPosition, 3, GL_FLOAT, GL_FALSE, 0, 0);

   safe_glEnableVertexAttribArray(aNormal);
   glBindBuffer(GL_ARRAY_BUFFER, Meshes[CurrentMesh]->NormalHandle);
   safe_glVertexAttribPointer(aNormal, 3, GL_FLOAT, GL_FALSE, 0, 0);

   glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, Meshes[CurrentMesh]->IndexHandle);

   glUniform3f(uColor, 0.1f, 0.78f, 0.9f);

   glDrawElements(GL_TRIANGLES, Meshes[CurrentMesh]->IndexBufferLength, GL_UNSIGNED_SHORT, 0);

   safe_glDisableVertexAttribArray(aPosition);
   safe_glDisableVertexAttribArray(aNormal);

   ModelTrans.popMatrix();

   glUseProgram(0);
   glutSwapBuffers();
   glutPostRedisplay();
   printOpenGLError();
}

void Reshape(int width, int height)
{
   WindowWidth = width;
   WindowHeight = height;
   glViewport(0, 0, width, height);
}

void computeBasis()
{
   vec3 temp = lookAtPos - eyePos;
   w = normalize(-temp);
   u = normalize(cross(up,w));
   v = normalize(cross(w,u));
}

void keyboard(unsigned char key, int x, int y)
{
   computeBasis();
   switch(key)
   {
   case('w'):
      eyePos = eyePos - w;
      lookAtPos = lookAtPos - w;
      break;
   case('a'):
      eyePos = eyePos - u;
      lookAtPos = lookAtPos - u;
      break;
   case('s'):
      eyePos = eyePos + w;
      lookAtPos = lookAtPos + w;
      break;
   case('d'):
      eyePos = eyePos + u;
      lookAtPos = lookAtPos + u;
      break;
   }
   glutPostRedisplay();
}

void motion(int x, int y)
{
   int height = glutGet(GLUT_WINDOW_HEIGHT);
   int width = glutGet(GLUT_WINDOW_WIDTH);

   y = height - y;


   float xdiff = (x - ((float)width/2))/((float)width/2);
   float ydiff = (y - ((float)height/2))/((float)height/2);

   if(ydiff >= 1 || ydiff <= -1) { return;}
   if(xdiff >= 1 || xdiff <= -1) { return;}

   beta = xdiff*PI;

   alpha = ydiff*PI;

   //if(alpha > 0.87266) { alpha = 0.87266; }
   //if(alpha < -0.87266) { alpha = -0.87266; }


   float vx = 1*cos(alpha)*cos(beta);
   float vy = 1*sin(alpha);
   float vz = 1*cos(alpha)*cos((PI/2.0)-beta);

   change = vec3(vx, vy, vz);
   //lookAtPos = vec3(change.x+eyePos.x,change.y+eyePos.y,change.z+eyePos.z);

   //cout << lookAtPos.x << " " << lookAtPos.y << " " << lookAtPos.z << endl;

   glutPostRedisplay();
}

void Timer(int param)
{
   Accumulator += StepSize * 0.001f;
   glutTimerFunc(StepSize, Timer, 1);

   //lookAtPos = lookAtPos + change;
    
}

int main(int argc, char *argv[])
{
   // Initialize Global Variables
   Accumulator = 0.f;
   CameraHeight = 0.f;
   CurrentMesh = 0;

   // Glut Setup
   glutInit(& argc, argv);
   glutInitWindowPosition(100, 100);
   glutInitWindowSize(WindowWidth, WindowHeight);
   glutInitDisplayMode(GLUT_RGB | GLUT_DOUBLE | GLUT_DEPTH);
   glutCreateWindow("Geometry Creator Demo");
   glutDisplayFunc(Draw);
   glutReshapeFunc(Reshape);
   glutKeyboardFunc(keyboard);
   glutPassiveMotionFunc(motion);
   glutTimerFunc(StepSize, Timer, 1);

   // OpenGL Setup
   Initialize();
   getGLversion();

   // Shader Setup
   if (! InstallShader((char *)"Diffuse.vert", (char *)"Diffuse.frag"))
   {
      printf("Error installing shader!\n");
      return 1;
   }

   Meshes[0] = GeometryCreator::CreateCube();
   /*Meshes[1]= GeometryCreator::CreateCube(vec3(0.5f, 2.f, 4.f));
     Meshes[2] = GeometryCreator::CreateCylinder(1.f, 1.f, 1.5f, 8, 8);
     Meshes[3] = GeometryCreator::CreateCylinder(1.5f, 0.5f, 2.0f, 16, 5);
     Meshes[4] = GeometryCreator::CreateCylinder(0.8f, 0.f, 1.75f, 32, 1);
     Meshes[5] = GeometryCreator::CreateDisc(0.5f, 1.5f, 1.75f, 32, 3);
     Meshes[6] = GeometryCreator::CreateDisc(0.1f, 1.f, 0.f, 8, 0);
     Meshes[7] = GeometryCreator::CreateSphere(glm::vec3(1.5f));
     Meshes[8] = GeometryCreator::CreateSphere(glm::vec3(1.5f, 2.5f, 0.5f), 128, 32);
     Meshes[9] = GeometryCreator::CreateSphere(glm::vec3(2.0f), 4, 2);
     Meshes[10] = GeometryCreator::CreateSphere(glm::vec3(2.0f), 6, 3);
     Meshes[11] = GeometryCreator::CreateTorus(1.f, 2.f, 12, 24);
     Meshes[12] = GeometryCreator::CreateTorus(0.5f, 2.5f, 8, 12);
     Meshes[13] = GeometryCreator::CreateTorus(2.f, 2.5f, 48, 64);*/
   glutMainLoop();

   return 0;
}
