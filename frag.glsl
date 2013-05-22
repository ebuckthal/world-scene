uniform vec3 uSP;
uniform int uMode;

uniform vec3 uAmbient;
uniform vec3 uSpecular;
uniform vec3 uDiffuse;
uniform float uShininess;

varying vec3 n;
varying vec3 vColor;
varying vec3 v;

void main() {

  if(uMode == 0 || uMode == 1 || uMode == 3)
  {
    gl_FragColor = vec4(vColor.r, vColor.g, vColor.b,1.0);
  }
  else
  {
    vec3 e = normalize(-v);
    vec3 l = normalize(uSP-v);
    vec3 r = normalize(2.0*dot(n,l)*n - l);

    float d = max(0.0, dot(n,l));
    float s = pow(max(0.0, dot(e,r)), uShininess);

    gl_FragColor = vec4(uAmbient + uDiffuse*d + uSpecular*s, 1.0);
  }
}
