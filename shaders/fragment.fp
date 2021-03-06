layout(location=0)out uvec4 fColor;
layout(location=1)out vec4  fPosition;
layout(location=2)out vec4  fNormal; 

in vec3 vPosition;
in vec3 vNormal;

void main(){
  vec3  diffuseColor   = vec3(.5,.5,.5,0);
  vec3  specularColor  = vec3(1);
  vec3  normal         = vNormal;
  float specularFactor = 1;

  uvec4 color  = uvec4(0);
  color.xyz   += uvec3(diffuseColor  *0xff);
  color.xyz   += uvec3(specularColor *0xff)<<8;
  color.w      = uint (specularFactor*0xff);

  fColor    = color;
  fPosition = vPosition;
  fNormal   = vec4(normal,-dot(vPosition,normal));
}

