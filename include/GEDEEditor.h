#pragma once

#include<Draw2D.h>

namespace gde{
  class Function{
    public:
      std::shared_ptr<Draw2D>draw2D;
      std::string functionName;
      std::vector<std::string>inputNames;
      std::string outputName;
      size_t node;
      Function(
          std::shared_ptr<Draw2D> const&draw2D,
          std::string             const&fce,
          std::vector<std::string>const&inputs,
          std::string             const&output){
        this->draw2D = draw2D;
        this->functionName = fce;
        this->inputNames = inputs;
        this->outputName = output;
      }
      void create();
      ~Function(){
        draw2D->deleteNode(this->node);
      }
  };

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
