out vec4 fColor;

in vec3 vPos;
in vec3 vNor;

uniform vec4 lightPosition = vec4(100,100,100,1);

void main(){
  vec3 L = normalize(lightPosition.xyz-vPos*lightPosition.w);
  vec3 N = normalize(vNor);
  float diffuseFactor = clamp(dot(L,N),0,1);
  fColor = vec4(diffuseFactor);
}

