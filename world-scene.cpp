/*
   CPE 471 Assignment 1
   Lighting.cpp
 */

#ifdef __APPLE__
#include "GLUT/glut.h"
#include <OPENGL/gl.h>
#endif
#ifdef __unix__
#include <GL/glut.h>
#endif

#include <stdlib.h>
#include <vector>
#include <stdio.h>
#include "glm/glm.hpp"
#include "glm/gtc/matrix_transform.hpp"
#include "glm/gtc/type_ptr.hpp"
#include "GLSL_helper.h"
#include "MStackHelp.h"
#include "GeometryCreator.h"
#include <sstream>
#include <fstream>
#include <iostream>
#include <map>
#include <algorithm>

#include <math.h>
#define PI 3.14159

using namespace std;
using namespace glm;

GLint h_aPosition;
GLint h_aNormal;
GLint h_uModelMatrix;
GLint h_uViewMatrix;
GLint h_uProjMatrix;
GLint h_uNormalMatrix;

GLint h_uDiffuse;
GLint h_uSpecular;
GLint h_uShininess;
GLint h_uEyePosition;

Mesh* cube;
Mesh* grnd;
Mesh* torus;
Mesh* sphere;
Mesh* disc;

float accum = 0.0f;
unsigned int const StepSize = 10;

int ShadeProg;

float g_height;
float g_width;

float g_x;
float g_y;

float beta = 0;
float alpha = 0;
float obeta = 0;
float oalpha = 0;

vec3 eyePos(0,0,0);
vec3 lookAtPos(0,0,1);
vec3 up(0,1,0);

vec3 w(0,0,0);
vec3 u(0,0,0);
vec3 v(0,0,0);

vec3 spec(0.8, 0.8, 0.8);
float shininess = 200;

RenderingHelper ModelTrans;

void SetProjectionMatrix()
{
   glm::mat4 Projection = glm::perspective(90.0f, (float)g_width/g_height, 0.1f, 100.f);
   safe_glUniformMatrix4fv(h_uProjMatrix, glm::value_ptr(Projection));
}

void SetView()
{
   glm::mat4 lookAt = glm::lookAt(eyePos, lookAtPos, up);
   safe_glUniformMatrix4fv(h_uViewMatrix, glm::value_ptr(lookAt));
}

void SetModel()
{
   safe_glUniformMatrix4fv(h_uModelMatrix, glm::value_ptr(ModelTrans.modelViewMatrix));
   safe_glUniformMatrix4fv(h_uNormalMatrix, glm::value_ptr(glm::transpose(glm::inverse(ModelTrans.modelViewMatrix))));
}

int InstallShader(char* vShaderName, char* fShaderName)
{
   GLuint VS; //handles to shader object
   GLuint FS; //handles to frag shader object
   GLint vCompiled, fCompiled, linked; //status of shader

   VS = glCreateShader(GL_VERTEX_SHADER);
   FS = glCreateShader(GL_FRAGMENT_SHADER);

   char const *vSource = textFileRead(vShaderName);
   char const *fSource = textFileRead(fShaderName);
   glShaderSource(VS, 1, &vSource, NULL);
   glShaderSource(FS, 1, &fSource, NULL);

   //compile shader and print log
   glCompileShader(VS);
   /* check shader status requires helper functions */
   printOpenGLError();
   glGetShaderiv(VS, GL_COMPILE_STATUS, &vCompiled);
   printShaderInfoLog(VS);

   //compile shader and print log
   glCompileShader(FS);
   /* check shader status requires helper functions */
   printOpenGLError();
   glGetShaderiv(FS, GL_COMPILE_STATUS, &fCompiled);
   printShaderInfoLog(FS);

   if (!vCompiled || !fCompiled) {
      printf("Error compiling either shader\n\n%s\n\nor\n\n%s\n\n", vShaderName, fShaderName);
      return -1;
   }

   //create a program object and attach the compiled shader
   ShadeProg = glCreateProgram();
   glAttachShader(ShadeProg, VS);
   glAttachShader(ShadeProg, FS);

   glLinkProgram(ShadeProg);
   /* check shader status requires helper functions */
   printOpenGLError();
   glGetProgramiv(ShadeProg, GL_LINK_STATUS, &linked);
   //printProgramInfoLog(ShadeProg);

   glUseProgram(ShadeProg);

   /* get handles to attribute data */
   h_aPosition = safe_glGetAttribLocation(ShadeProg, "aPosition");
   h_aNormal = safe_glGetAttribLocation(ShadeProg, "aNormal");
   h_uProjMatrix = safe_glGetUniformLocation(ShadeProg, "uProjMatrix");
   h_uViewMatrix = safe_glGetUniformLocation(ShadeProg, "uViewMatrix");
   h_uModelMatrix = safe_glGetUniformLocation(ShadeProg, "uModelMatrix");
   h_uNormalMatrix = safe_glGetUniformLocation(ShadeProg, "uNormalMatrix");
   
   h_uDiffuse = safe_glGetUniformLocation(ShadeProg, "uDiffuse");
   h_uSpecular = safe_glGetUniformLocation(ShadeProg, "uSpecular");
   h_uShininess = safe_glGetUniformLocation(ShadeProg, "uShininess");
   h_uEyePosition = safe_glGetUniformLocation(ShadeProg, "uEyePosition");
   //printf("sucessfully installed shader %d\n", ShadeProg);
   return 1;
}

void drawSphere(vec3 color, vec3 spec, float shininess)
{
   SetModel();

   safe_glEnableVertexAttribArray(h_aPosition);
   glBindBuffer(GL_ARRAY_BUFFER, sphere->PositionHandle);
   safe_glVertexAttribPointer(h_aPosition, 3, GL_FLOAT, GL_FALSE, 0, 0);

   safe_glEnableVertexAttribArray(h_aNormal);
   glBindBuffer(GL_ARRAY_BUFFER, sphere->NormalHandle);
   safe_glVertexAttribPointer(h_aNormal, 3, GL_FLOAT, GL_FALSE, 0, 0);

   glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, sphere->IndexHandle);

   glUniform3f(h_uDiffuse, color.x, color.y, color.z);
   glUniform3f(h_uSpecular, spec.x, spec.y, spec.z);
   glUniform1f(h_uShininess, shininess);

   glDrawElements(GL_TRIANGLES, sphere->IndexBufferLength, GL_UNSIGNED_SHORT, 0);

   safe_glDisableVertexAttribArray(h_aPosition);
   safe_glDisableVertexAttribArray(h_aNormal);
}

void drawDisc(vec3 color, vec3 spec, float shininess)
{
   //ModelTrans.rotate(90, vec3(1.0f, 0, 0));
   SetModel();

   safe_glEnableVertexAttribArray(h_aPosition);
   glBindBuffer(GL_ARRAY_BUFFER, disc->PositionHandle);
   safe_glVertexAttribPointer(h_aPosition, 3, GL_FLOAT, GL_FALSE, 0, 0);

   safe_glEnableVertexAttribArray(h_aNormal);
   glBindBuffer(GL_ARRAY_BUFFER, disc->NormalHandle);
   safe_glVertexAttribPointer(h_aNormal, 3, GL_FLOAT, GL_FALSE, 0, 0);

   glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, disc->IndexHandle);

   glUniform3f(h_uDiffuse, color.x, color.y, color.z);
   glUniform3f(h_uSpecular, spec.x, spec.y, spec.z);
   glUniform1f(h_uShininess, shininess);

   glDrawElements(GL_TRIANGLES, disc->IndexBufferLength, GL_UNSIGNED_SHORT, 0);

   safe_glDisableVertexAttribArray(h_aPosition);
   safe_glDisableVertexAttribArray(h_aNormal);
}

void drawTorus(vec3 color, vec3 spec, float shininess)
{
   ModelTrans.rotate(90, vec3(1.0f, 0, 0));
   SetModel();

   safe_glEnableVertexAttribArray(h_aPosition);
   glBindBuffer(GL_ARRAY_BUFFER, torus->PositionHandle);
   safe_glVertexAttribPointer(h_aPosition, 3, GL_FLOAT, GL_FALSE, 0, 0);

   safe_glEnableVertexAttribArray(h_aNormal);
   glBindBuffer(GL_ARRAY_BUFFER, torus->NormalHandle);
   safe_glVertexAttribPointer(h_aNormal, 3, GL_FLOAT, GL_FALSE, 0, 0);

   glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, torus->IndexHandle);

   glUniform3f(h_uDiffuse, color.x, color.y, color.z);
   glUniform3f(h_uSpecular, spec.x, spec.y, spec.z);
   glUniform1f(h_uShininess, shininess);

   glDrawElements(GL_TRIANGLES, torus->IndexBufferLength, GL_UNSIGNED_SHORT, 0);

   safe_glDisableVertexAttribArray(h_aPosition);
   safe_glDisableVertexAttribArray(h_aNormal);
}

void drawCube(vec3 color, vec3 spec, float shininess)
{
   SetModel();

   safe_glEnableVertexAttribArray(h_aPosition);
   glBindBuffer(GL_ARRAY_BUFFER, cube->PositionHandle);
   safe_glVertexAttribPointer(h_aPosition, 3, GL_FLOAT, GL_FALSE, 0, 0);

   safe_glEnableVertexAttribArray(h_aNormal);
   glBindBuffer(GL_ARRAY_BUFFER, cube->NormalHandle);
   safe_glVertexAttribPointer(h_aNormal, 3, GL_FLOAT, GL_FALSE, 0, 0);

   glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, cube->IndexHandle);

   glUniform3f(h_uDiffuse, color.x, color.y, color.z);
   glUniform3f(h_uSpecular, spec.x, spec.y, spec.z);
   glUniform1f(h_uShininess, shininess);

   glDrawElements(GL_TRIANGLES, cube->IndexBufferLength, GL_UNSIGNED_SHORT, 0);

   safe_glDisableVertexAttribArray(h_aPosition);
   safe_glDisableVertexAttribArray(h_aNormal);
}

void drawGround()
{
   ModelTrans.pushMatrix();
   ModelTrans.translate(vec3(0,-0.5,0));
   SetModel();

   safe_glEnableVertexAttribArray(h_aPosition);
   glBindBuffer(GL_ARRAY_BUFFER, grnd->PositionHandle);
   safe_glVertexAttribPointer(h_aPosition, 3, GL_FLOAT, GL_FALSE, 0, 0);

   safe_glEnableVertexAttribArray(h_aNormal);
   glBindBuffer(GL_ARRAY_BUFFER, grnd->NormalHandle);
   safe_glVertexAttribPointer(h_aNormal, 3, GL_FLOAT, GL_FALSE, 0, 0);

   glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, grnd->IndexHandle);

   glUniform3f(h_uDiffuse, 0.0, 0.5, 0.0);
   glUniform3f(h_uSpecular, 0.0, 0.0, 0.0);
   glUniform1f(h_uShininess, 20.0);

   glDrawElements(GL_TRIANGLES, grnd->IndexBufferLength, GL_UNSIGNED_SHORT, 0);

   safe_glDisableVertexAttribArray(h_aPosition);
   safe_glDisableVertexAttribArray(h_aNormal);

   ModelTrans.popMatrix();
}

void drawSpinDisc(vec3 pos)
{
   ModelTrans.pushMatrix();
   ModelTrans.translate(pos);
   ModelTrans.translate(vec3(0,3,0));
   drawSphere(vec3(1.0, 0, 0), vec3(0.7, 0.8, 0.9), 100.0);
   
   ModelTrans.popMatrix();
   
   ModelTrans.pushMatrix();
   ModelTrans.translate(pos);
   ModelTrans.translate(vec3(0,3,0));
   ModelTrans.rotate(45 * accum, vec3(0,-1,0));
   drawDisc(vec3(1.0, 0, 0), vec3(0.2, 0.2, 0.9), 30.0);

   ModelTrans.popMatrix();
   
   ModelTrans.pushMatrix();
   ModelTrans.translate(pos);
   ModelTrans.translate(vec3(0,3,0));
   ModelTrans.rotate(45 * accum, vec3(0,1,0));
   drawDisc(vec3(1.0, 0, 0), vec3(0.2, 0.2, 0.9), 30.0);

   ModelTrans.popMatrix();
   
   ModelTrans.pushMatrix();
   ModelTrans.translate(pos);
   ModelTrans.translate(vec3(0,3,0));
   ModelTrans.rotate(90 * accum, vec3(0,-1,0));
   drawDisc(vec3(1.0, 0, 0), vec3(0.2, 0.2, 0.9), 30);

   ModelTrans.popMatrix();
   
   ModelTrans.pushMatrix();
   ModelTrans.translate(pos);
   ModelTrans.translate(vec3(0,3,0));
   ModelTrans.rotate(90 * accum, vec3(0,1,0));
   drawDisc(vec3(1.0, 0, 0), vec3(0.2, 0.2, 0.9), 30);

   ModelTrans.popMatrix();
}

void drawCubeStack(vec3 pos)
{
   ModelTrans.pushMatrix();
   ModelTrans.translate(pos);
   ModelTrans.translate(vec3(0,0.75,0));
   ModelTrans.scale(1.4);
   ModelTrans.rotate(45 * accum, vec3(0,-1,0));
   drawCube(vec3(0, 1.0, 0), vec3(0.8, 0.8, 0.8), 200.0);

      ModelTrans.pushMatrix();
      ModelTrans.translate(vec3(0,1,0));
      ModelTrans.scale(0.65);
      ModelTrans.rotate(90 * accum, vec3(0, 1, 0));
      drawCube(vec3(1.0, 0, 0), vec3(0.8, 0.8, 0.8), 200.0);

         ModelTrans.pushMatrix();
         ModelTrans.translate(vec3(0,1, 0));
         ModelTrans.scale(0.65);
         ModelTrans.rotate(90 * accum, vec3(0,-1,0));
         drawCube(vec3(0,0,1.0), vec3(0.8, 0.8, 0.8), 200.0);

            ModelTrans.pushMatrix();
            ModelTrans.translate(vec3(0,1, 0));
            ModelTrans.scale(0.65);
            ModelTrans.rotate(135 * accum, vec3(0,1,0));
            drawCube(vec3(0,0,1.0), vec3(0.8, 0.8, 0.8), 200.0);
            
               ModelTrans.pushMatrix();
               ModelTrans.translate(vec3(0,1, 0));
               ModelTrans.scale(0.65);
               ModelTrans.rotate(135 * accum, vec3(0,-1,0));
               drawCube(vec3(0,0,1.0), vec3(0.8, 0.8, 0.8), 200.0);

               ModelTrans.popMatrix();

            ModelTrans.popMatrix();

         ModelTrans.popMatrix();

      ModelTrans.popMatrix();
   
   ModelTrans.popMatrix();
   
   ModelTrans.pushMatrix();
   ModelTrans.translate(pos);
   ModelTrans.translate(vec3(0,0.5,0));

      ModelTrans.pushMatrix();
      ModelTrans.translate(vec3(0,0.5,0));
      ModelTrans.translate(vec3(0,.33*(sin(accum)+1),0));
      ModelTrans.scale(0.8f);
      
         ModelTrans.pushMatrix();
         ModelTrans.translate(vec3(0,0.5,0));
         ModelTrans.translate(vec3(0,.33*(sin(accum)+1),0));
         ModelTrans.scale(0.8f);
         
            ModelTrans.pushMatrix();
            ModelTrans.translate(vec3(0,0.5,0));
            ModelTrans.translate(vec3(0,.33*(sin(accum)+1),0));
            ModelTrans.scale(0.8f);
            
               ModelTrans.pushMatrix();
               ModelTrans.translate(vec3(0,0.5,0));
               ModelTrans.translate(vec3(0,.33*(sin(accum)+1),0));
               ModelTrans.scale(0.8f);
         
               drawTorus(vec3(1.0, 0, 0), spec, shininess);
               ModelTrans.popMatrix();
         
            drawTorus(vec3(1.0, 0, 0), spec, shininess);
            ModelTrans.popMatrix();

         drawTorus(vec3(1.0, 0, 0), spec, shininess);
         ModelTrans.popMatrix();

      drawTorus(vec3(0, 1.0, 0), spec, shininess);
      ModelTrans.popMatrix();

   drawTorus(vec3(0, 0, 1.0), spec, shininess);
   ModelTrans.popMatrix();

}

void Draw(void)
{
   glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

   glUseProgram(ShadeProg);

   SetProjectionMatrix();
   SetView();

   glUniform3f(h_uEyePosition, eyePos.x, eyePos.y, eyePos.z);

   drawGround();
   drawCubeStack(vec3(-10,0,7));
   drawCubeStack(vec3(50,0,3));
   drawCubeStack(vec3(-3,0,15));
   drawCubeStack(vec3(45,0,37));
   drawCubeStack(vec3(-20,0,-20));
   drawCubeStack(vec3(0,0,0));
   drawCubeStack(vec3(9,0,4));
   drawCubeStack(vec3(13,0,16));
   drawCubeStack(vec3(-9,0,26));
   drawCubeStack(vec3(-5,0,-4));
   drawCubeStack(vec3(-15,0,30));
   drawSpinDisc(vec3(5,0,12));
   drawSpinDisc(vec3(14,0,9));
   drawSpinDisc(vec3(35,0,-17));
   drawSpinDisc(vec3(-10,0,-5));
   drawSpinDisc(vec3(-8,0,9));
   drawSpinDisc(vec3(1,0,-19));
   drawSpinDisc(vec3(-15,0,4));
   drawSpinDisc(vec3(-13,0,-20));
   drawSpinDisc(vec3(34,0,3));
   drawSpinDisc(vec3(3,0,20));
   drawSpinDisc(vec3(3,0,2));

   glUseProgram(0);

   glutSwapBuffers();
   glutPostRedisplay();
}

void ReshapeGL(int width, int height)
{
   g_width = (float)width;
   g_height = (float)height;
   glViewport(0, 0, (GLsizei)(width), (GLsizei)(height));
}

void Initialize()
{
   glClearColor(0.8f, 0.8f, 1.0, 1.0f);

   glClearDepth(1.0f);
   glDepthFunc(GL_LEQUAL);
   glEnable(GL_DEPTH_TEST);
   glEnable(GL_CULL_FACE);
   glCullFace(GL_BACK);


   ModelTrans.useModelViewMatrix();
   ModelTrans.loadIdentity();
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


   float xdiff = (g_x-x);
   float ydiff = (g_y-y);

   beta = obeta + ((float)xdiff/width)*PI;

   alpha = oalpha + ((float)ydiff/height)*PI;
   
   if(alpha > 0.87) { alpha = 0.87; }
   if(alpha < -0.87) { alpha = -0.87; }

   float vx = 1*cos(alpha)*cos(beta);
   float vy = 1*sin(alpha);
   float vz = 1*cos(alpha)*cos((PI/2.0)-beta);

   lookAtPos = vec3(vx, vy, vz) + eyePos;
   //cout << change.x << " " << change.y << " " << change.z << endl;

   glutPostRedisplay();
}

void mouse(int button, int state, int x, int y)
{
   int height = glutGet(GLUT_WINDOW_HEIGHT);

   if(state == 0){

      g_x = x;
      g_y = height - y;
   }
   if(state == 1) {
      obeta = beta;
      oalpha = alpha;
   }

   glutPostRedisplay();
}

void Timer(int param)
{
   accum += StepSize * 0.001f;
   glutTimerFunc(StepSize, Timer, 1);
}

int main(int argc, char** argv)
{
   glutInit(&argc, argv);
   glutInitWindowPosition(20, 20);
   glutInitWindowSize(1000, 1000);
   glutInitDisplayMode(GLUT_RGB | GLUT_DOUBLE | GLUT_DEPTH);
   glutCreateWindow("Lighting");
   glutReshapeFunc(ReshapeGL);
   glutDisplayFunc(Draw);
   glutKeyboardFunc(keyboard);
   glutTimerFunc(StepSize, Timer, 1);
   glutMouseFunc(mouse);
   glutMotionFunc(motion);
   Initialize();

   getGLversion();

   if(!(InstallShader((char*)"Diffuse.vert", (char*)"Diffuse.frag")))
   {
      perror("Error installing shader");
      exit(-1);
   }

   cube = GeometryCreator::CreateCube();
   grnd = GeometryCreator::CreateCube(vec3(500.0f, 1.0f, 500.0f));
   torus = GeometryCreator::CreateTorus(1.8f, 2.3f, 48, 64);
   disc = GeometryCreator::CreateDisc(1.5f, 2.0f, 0.3f, 32, 3);
   sphere = GeometryCreator::CreateSphere();

   glutMainLoop();
   return 0;
}



