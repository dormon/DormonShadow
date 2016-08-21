#pragma once

#include<geGL/geGL.h>
#include<Viewport2d.h>
#include<Layer.h>
#include<Node2d.h>

class Edit{
  public:
    Edit(ge::gl::Context const&gl,glm::uvec2 const&size);
    ~Edit();
    void draw();
    void drawViewport(Viewport2d*viewport,glm::mat3 const&model,glm::mat3 const&viewProjection);
    void drawLayer(Layer*layer,glm::mat3 const&model,glm::mat3 const&viewProjection);
    void drawNode(Node2d*node,glm::mat3 const&model,glm::mat3 const&viewProjection);
    Viewport2d*rootViewport;
    Viewport2d*currentViewport;
    ge::gl::Context const&gl;
    std::shared_ptr<ge::gl::Program>lineProgram;
    std::shared_ptr<ge::gl::Program>pointProgram;
    std::shared_ptr<ge::gl::Program>circleProgram;
    std::shared_ptr<ge::gl::Program>triangleProgram;
    std::shared_ptr<ge::gl::Program>splineProgram;
    std::shared_ptr<ge::gl::Program>textProgram;
    std::shared_ptr<ge::gl::Texture>fontTexture;
    std::shared_ptr<ge::gl::Program>stencilProgram;
    std::shared_ptr<ge::gl::VertexArray>stencilVAO;
};
