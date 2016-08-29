#pragma once

#include<geGL/geGL.h>
#include<glm/glm.hpp>
#include<glm/gtc/matrix_transform.hpp>
#include<glm/gtc/type_ptr.hpp>
#include<glm/gtc/matrix_access.hpp>

#include<GDEButton.h>

namespace gde{
  class EditorImpl;
  class Editor{
    public:
      Editor(ge::gl::Context const&gl,glm::uvec2 const&size);
      ~Editor();
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
