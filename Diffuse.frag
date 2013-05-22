varying vec3 vLightVector;
varying vec3 vNormal;
varying vec3 vEye;

uniform vec3 uDiffuse;
uniform vec3 uSpecular;
uniform float uShininess;

void main()
{
    vec3 vReflect = normalize(vLightVector) - 2.0 * dot(vNormal, normalize(vLightVector))*vNormal;

    vec3 AmbientColor = vec3(0.2, 0.2, 0.2);
    vec3 DiffuseColor = uDiffuse * clamp(dot(vNormal, normalize(vLightVector)), 0.0, 1.0);
    float spec = pow(max(0.0, dot(normalize(vEye), normalize(vReflect))), uShininess);
    vec3 SpecularColor = uSpecular * spec;
    
    
    gl_FragColor = vec4(AmbientColor + DiffuseColor + SpecularColor, 1.0);

    //gl_FragColor = vec4(abs(normalize(vNormal)), 1.0); // Show Normals
}

