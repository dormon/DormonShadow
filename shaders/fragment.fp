out vec4 fColor;

in vec3 vPos;
in vec3 vNor;

uniform vec4 lightPosition = vec4(100,100,100,1);

layout(binding=0)uniform sampler2D fontTexture;
const uint fontWidth = 1330;
const uint fontHeight = 28;
const uint nofCharacters = 95;
const uint characterWidth = fontWidth/nofCharacters;
const uint characterHeight = fontHeight;
const float characterWidthNormalized = 1.f/nofCharacters;

#define CH_space 32

float font(int ch,vec2 coord){
  int id = ch-CH_space;
  return texture(fontTexture,vec2((id+coord.x)*characterWidthNormalized,coord.y)).r;
}

void main(){
  vec2 cc = gl_FragCoord.xy/vec2(1024,768);
  vec3 L = normalize(lightPosition.xyz-vPos*lightPosition.w);
  vec3 N = normalize(vNor);
  float diffuseFactor = clamp(dot(L,N),0,1);
  fColor = vec4(diffuseFactor);//*font(97,cc));
  //fColor = vec4(0,texture(fontTexture,cc*vec2(1,95./2*768/1024.)).r,0,1);
}

