#pragma once

#include<geGL/geGL.h>
#include<Viewport2d.h>
#include<Layer.h>
#include<Node2d.h>

class MouseMotionEvent{
  public:
    std::function<void(Node2d*,void*,glm::vec2 const&,glm::vec2 const&)>callback = nullptr;
    void*userData = nullptr;
    glm::vec2 pos;
    glm::vec2 size;
    MouseMotionEvent(
        glm::vec2 const&p,
        glm::vec2 const&s,
        std::function<void(Node2d*,void*,glm::vec2 const&,glm::vec2 const&)>const&c,
        void*data = nullptr):callback(c),userData(data),pos(p),size(s){}
    MouseMotionEvent(
        std::function<void(Node2d*,void*,glm::vec2 const&,glm::vec2 const&)>const&c,
        void*data = nullptr):MouseMotionEvent(glm::vec2(0.f),glm::vec2(0.f),c,data){}
    MouseMotionEvent(
        std::function<void(Node2d*,void*)>const&c,
        void*data = nullptr):MouseMotionEvent([c](Node2d*n,void*d,glm::vec2 const&,glm::vec2 const&){c(n,d);},data){}
    MouseMotionEvent(
        std::function<void(void*)>const&c,
        void*data = nullptr):MouseMotionEvent([c](Node2d*,void*d,glm::vec2 const&,glm::vec2 const&){c(d);},data){}
    MouseMotionEvent(
        std::function<void(Node2d*)>const&c,
        void*data = nullptr):MouseMotionEvent([c](Node2d*n,void*,glm::vec2 const&,glm::vec2 const&){c(n);},data){}
    MouseMotionEvent(
        std::function<void()>const&c,
        void*data = nullptr):MouseMotionEvent([c](Node2d*,void*,glm::vec2 const&,glm::vec2 const&){c();},data){}




    void operator()(Node2d*node,glm::vec2 const&diff,glm::vec2 const&pos){
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
    void drawViewport(Viewport2d*viewport,glm::mat3 const&model,glm::mat3 const&viewProjection);
    void drawLayer(Layer*layer,glm::mat3 const&model,glm::mat3 const&viewProjection);
    void drawNode(Node2d*node,glm::mat3 const&model,glm::mat3 const&viewProjection);
    void mouseMotion(int32_t xrel,int32_t yrel,size_t x,size_t y);
    bool mouseMotionViewport(Viewport2d*viewport,glm::vec2 const&diff,glm::vec2 const&pos);
    bool mouseMotionLayer(Layer*layer,glm::vec2 const&diff,glm::vec2 const&pos);
    bool mouseMotionNode(Node2d*node,glm::vec2 const&diff,glm::vec2 const&pos);
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
    glm::mat3 translate(glm::vec2 const&pos);
    glm::mat3 rotate(float angle);
    glm::mat3 scale(float scale);

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

