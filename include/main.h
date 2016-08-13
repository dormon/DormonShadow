#pragma once

#include<VariableRegisterManipulator.h>
#include<geDE/Kernel.h>
#include<geAd/SDLWindow/SDLWindow.h>
#include<geGL/geGL.h>

struct Application{
  ge::de::Kernel kernel;
  std::shared_ptr<ge::gl::Context>            gl                  = nullptr;
  std::shared_ptr<ge::ad::SDLMainLoop>        mainLoop            = nullptr;
  std::shared_ptr<ge::ad::SDLWindow>          window              = nullptr;
  std::shared_ptr<VariableRegisterManipulator>variableManipulator = nullptr;
  std::shared_ptr<ge::de::Statement>          idleScript          = nullptr;
  ~Application();
  bool init(int argc,char*argv[]);
  static void idle(void*);
  static bool eventHandler(SDL_Event const&,void*);
  template<bool DOWN>
    static bool key(SDL_Event const&,void*);
  template<bool DOWN>
    static bool mouseButton(SDL_Event const&,void*);
  static bool mouseMotion(SDL_Event const&,void*);
};


