#pragma once

#include<geGL/geGL.h>
#include<Viewport2d.h>
#include<Layer.h>
#include<Node2d.h>
#include<Primitives.h>

class RenderData{
  public:
    std::shared_ptr<ge::gl::Buffer>lineBuffer;
    std::shared_ptr<ge::gl::VertexArray>lineVAO;
    size_t nofLines = 0;
    std::shared_ptr<ge::gl::Buffer>pointBuffer;
    std::shared_ptr<ge::gl::VertexArray>pointVAO;
    size_t nofPoints = 0;
    std::shared_ptr<ge::gl::Buffer>circleBuffer;
    std::shared_ptr<ge::gl::VertexArray>circleVAO;
    size_t nofCircles = 0;
    std::shared_ptr<ge::gl::Buffer>triangleBuffer;
    std::shared_ptr<ge::gl::VertexArray>triangleVAO;
    size_t nofTriangles = 0;
    std::shared_ptr<ge::gl::Buffer>splineBuffer;
    std::shared_ptr<ge::gl::VertexArray>splineVAO;
    size_t nofSplines = 0;
    std::shared_ptr<ge::gl::Buffer>textBuffer;
    std::shared_ptr<ge::gl::VertexArray>textVAO;
    size_t nofCharacters = 0;
    bool changed = true;
};

class MouseMotionEvent{
  public:
    enum Type{
      MOUSE_ENTER,
      MOUSE_EXIT,
      MOUSE_MOVE,
    };
    std::function<void(std::shared_ptr<Node2d>const&,std::shared_ptr<void>const&,glm::vec2 const&,glm::vec2 const&)>callback = nullptr;
    std::shared_ptr<void>userData;
    glm::vec2 pos;
    glm::vec2 size;
    Type type;
    MouseMotionEvent(
        glm::vec2 const&p,
        glm::vec2 const&s,
        std::function<void(std::shared_ptr<Node2d>const&,std::shared_ptr<void>const&,glm::vec2 const&,glm::vec2 const&)>const&c,
        std::shared_ptr<void>const&data = nullptr,
        Type const&t = MOUSE_MOVE):callback(c),userData(data),pos(p),size(s),type(t){}
    MouseMotionEvent(
        std::function<void(std::shared_ptr<Node2d>const&,std::shared_ptr<void>const&,glm::vec2 const&,glm::vec2 const&)>const&c,
        std::shared_ptr<void>const&data = nullptr,
        Type const&t = MOUSE_MOVE):MouseMotionEvent(glm::vec2(0.f),glm::vec2(0.f),c,data,t){}
    MouseMotionEvent(
        std::function<void(std::shared_ptr<Node2d>const&,std::shared_ptr<void>const&)>const&c,
        std::shared_ptr<void>const&data = nullptr,
        Type const&t = MOUSE_MOVE):MouseMotionEvent(
          [c](std::shared_ptr<Node2d>const&n,std::shared_ptr<void>const&d,glm::vec2 const&,glm::vec2 const&){c(n,d);},data,t){}
    MouseMotionEvent(
        std::function<void(std::shared_ptr<void>const&)>const&c,
        std::shared_ptr<void>const&data = nullptr,
        Type const&t = MOUSE_MOVE):MouseMotionEvent(
          [c](std::shared_ptr<Node2d>const&,std::shared_ptr<void>const&d,glm::vec2 const&,glm::vec2 const&){c(d);},data,t){}
    MouseMotionEvent(
        std::function<void(void*)>const&c,
        void*data = nullptr,
        Type const&t = MOUSE_MOVE):MouseMotionEvent(
          [c](std::shared_ptr<Node2d>const&,std::shared_ptr<void>const&d,glm::vec2 const&,glm::vec2 const&){c(*(void**)d.get());},std::make_shared<void*>(data),t){}

    MouseMotionEvent(
        std::function<void(std::shared_ptr<Node2d>const&)>const&c,
        Type const&t = MOUSE_MOVE):MouseMotionEvent(
        [c](std::shared_ptr<Node2d>const&n,std::shared_ptr<void>const&,glm::vec2 const&,glm::vec2 const&){c(n);},nullptr,t){}
    MouseMotionEvent(
        std::function<void()>const&c,
        Type const& t = MOUSE_MOVE):MouseMotionEvent(
        [c](std::shared_ptr<Node2d>const&,std::shared_ptr<void>const&,glm::vec2 const&,glm::vec2 const&){c();},nullptr,t){}
    void operator()(std::shared_ptr<Node2d>const&node,glm::vec2 const&diff,glm::vec2 const&pos){
      assert(this->callback!=nullptr);
      this->callback(node,this->userData,diff,pos);
    }
    ~MouseMotionEvent(){}
};

class Edit{
  public:
    Edit(ge::gl::Context const&gl,glm::uvec2 const&size);
    ~Edit();
    void draw();
    void drawViewport(std::shared_ptr<Viewport2d>const&viewport,glm::mat3 const&model,glm::mat3 const&viewProjection);
    void drawLayer(std::shared_ptr<Layer>const&layer,glm::mat3 const&model,glm::mat3 const&viewProjection);
    void drawNode(std::shared_ptr<Node2d>const&node,glm::mat3 const&model,glm::mat3 const&viewProjection);
    void mouseMotion(int32_t xrel,int32_t yrel,size_t x,size_t y);
    bool mouseMotionViewport(std::shared_ptr<Viewport2d>const&viewport,glm::vec2 const&diff,glm::vec2 const&pos);
    bool mouseMotionLayer(std::shared_ptr<Layer>const&layer,glm::vec2 const&diff,glm::vec2 const&pos);
    bool mouseMotionNode(std::shared_ptr<Node2d>const&node,glm::vec2 const&diff,glm::vec2 const&pos);
    std::shared_ptr<Viewport2d>rootViewport;
    std::shared_ptr<Node2d>menuNode;
    std::shared_ptr<Viewport2d>editViewport;
    std::shared_ptr<Node2d>functionsNode;
    std::shared_ptr<Node2d>connectionsNode;
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
    static glm::mat3 translate(glm::vec2 const&pos);
    static glm::mat3 rotate(float angle);
    static glm::mat3 scale(float scale);

};

inline glm::mat3 Edit::translate(glm::vec2 const&pos){
  auto result = glm::mat3(1.f);
  result[2]=glm::vec3(pos,1);
  return result;
}

inline glm::mat3 Edit::rotate(float angle){
  auto result = glm::mat3(1.f);
  result[0].x =  glm::cos(angle);
  result[0].y = -glm::sin(angle);
  result[1].x =  glm::sin(angle);
  result[1].y =  glm::cos(angle);
  return result;
}

inline glm::mat3 Edit::scale(float scale){
  auto result = glm::mat3(1.f);
  result[0].x=scale;
  result[1].y=scale;
  return result;
}

