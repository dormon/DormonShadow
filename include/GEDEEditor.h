#pragma once

#include<Draw2D.h>

namespace gde{
  class EditorImpl;
  class Editor{
    public:
      Editor(ge::gl::Context const&gl,glm::uvec2 const&size);
      ~Editor();
      enum MouseButton{LEFT,MIDDLE,RIGHT};
      void mouseMotion(int32_t xrel,int32_t yrel,size_t x,size_t y);
      void mouseButtonDown(MouseButton b,size_t x,size_t y);
      void mouseButtonUp(MouseButton b,size_t x,size_t y);
      void mouseWheel(int32_t x,int32_t y);
      void resize(size_t w,size_t h);
      void draw();
    protected:
      EditorImpl*_impl;
  };
}
