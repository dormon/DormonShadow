#pragma once

#include<memory>

namespace ge{
  namespace gl{
    class Program;
    class Framebuffer;
    class VertexArray;
  }
}

class RenderCommand{
  public:
    RenderCommand();
    virtual ~RenderCommand();
  protected:
    std::shared_ptr<Program>_program;
    std::shared_ptr<Framebuffer>_framebuffer;
    std::shared_ptr<VertexArray>_vao;

};
