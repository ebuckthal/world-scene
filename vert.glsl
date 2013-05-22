uniform mat4 uProjMatrix;
uniform mat4 uViewMatrix;
uniform mat4 uModelMatrix;
uniform vec3 uSP;

uniform vec3 uAmbient;
uniform vec3 uSpecular;
uniform vec3 uDiffuse;
uniform float uShininess;

uniform int uMode; 

attribute vec3 aPosition;
attribute vec3 aNormal;

varying vec3 n;
varying vec3 vColor;
varying vec3 v;

void main() {
 
  vec4 modelPos = uModelMatrix * vec4(aPosition.x, aPosition.y, aPosition.z, 1);
  vec4 eyePos = uViewMatrix * modelPos;
  gl_Position = uProjMatrix * eyePos;

  mat4 eyeMatrix = uModelMatrix * uViewMatrix;

  v = vec3(eyePos.xyz);
  vec3 e = normalize(-v);
  vec3 l = normalize(uSP-v);
  n = normalize((uModelMatrix*uViewMatrix*vec4(normalize(aNormal),0)).xyz);

  vec3 r = normalize(2.0*dot(n,l)*n - l);
  float d = max(0.0, dot(n,l));
  float s = (pow(max(0.0, dot(r,e)), uShininess));

  vColor = vec3(0.0);
  
  if(uMode == 0) //normal
  {
    vColor = n;
  }
  
  if (uMode == 1) //gouraud
  {
    vColor = uAmbient + uDiffuse*d + uSpecular*s;
  }

  if (uMode == 3) //no shading
  {
    vColor = vec3(1,0,0);
  }
}
