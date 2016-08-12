#include<RegisterMouse.h>
#include<geDE/Kernel.h>
#include<SDL2/SDL.h>

std::string mouse::buttonName(int key){
  if(key==SDL_BUTTON_LEFT)return"left";
  if(key==SDL_BUTTON_MIDDLE)return"middle";
  if(key==SDL_BUTTON_RIGHT)return"right";
  if(key==SDL_BUTTON_X1)return"x1";
  if(key==SDL_BUTTON_X2)return"x2";
  return "";
}

std::string mouse::fullButtonName(int key){
  std::stringstream ss;
  ss<<"mouse."<<buttonName(key);
  return ss.str();
}

void mouse::registerMouse(ge::de::Kernel*kernel){
  int buttons[]={
    SDL_BUTTON_LEFT,
    SDL_BUTTON_MIDDLE,
    SDL_BUTTON_RIGHT,
    SDL_BUTTON_X1,
    SDL_BUTTON_X2,
  };
  for(auto const&x:buttons)
    kernel->addVariable(fullButtonName(x),(bool)false);

  kernel->addVariable("mouse.x",(int32_t)0);
  kernel->addVariable("mouse.y",(int32_t)0);
  kernel->addVariable("mouse.xrel",(int32_t)0);
  kernel->addVariable("mouse.yrel",(int32_t)0);
}
