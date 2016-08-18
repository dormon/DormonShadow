#pragma once

#include<cstdint>
#include<geGL/geGL.h>

#include<glm/glm.hpp>
#include<glm/gtc/matrix_transform.hpp>
#include<glm/gtc/type_ptr.hpp>
#include<glm/gtc/matrix_access.hpp>

class Primitive{
  public:
    enum Type{
      LINE,
      POINT,
      CIRCLE,
      TRIANGLE,
      TEXT,
      SPLINE,
    }type;
    glm::vec4 color;
    Primitive(Type type,glm::vec4 const&color = glm::vec4(1.f)){
      assert(this!=nullptr);
      this->color = color;
      this->type = type;
    }
    virtual ~Primitive(){}
};

class Line: public Primitive{
  public:
    glm::vec2 points[2];
    float width;
    Line(glm::vec2 const&a,glm::vec2 const&b,float width,glm::vec4 const&color = glm::vec4(1.f)):Primitive(LINE,color){
      assert(this!=nullptr);
      this->points[0]=a;
      this->points[1]=b;
      this->width = width;
    }
    Line(float ax,float ay,float bx,float by,float width,glm::vec4 const&color = glm::vec4(1.f)):Line(glm::vec2(ax,ay),glm::vec2(bx,by),width,color){}
    virtual ~Line(){}
};

class Point: public Primitive{
  public:
    glm::vec2 point;
    float size;
    Point(glm::vec2 const&position,float radius,glm::vec4 const&color = glm::vec4(1.f)):Primitive(POINT,color){
      assert(this!=nullptr);
      this->point = position;
      this->size = radius;
    }
    Point(float x,float y,float r,glm::vec4 const&color = glm::vec4(1.f)):Point(glm::vec2(x,y),r,color){}
    virtual ~Point(){}
};

class Circle: public Primitive{
  public:
    glm::vec2 point;
    float size;
    float width;
    Circle(glm::vec2 const&position,float radius,float width,glm::vec4 const&color = glm::vec4(1.f)):Primitive(CIRCLE,color){
      assert(this!=nullptr);
      this->point = position;
      this->size = radius;
      this->width = width;
    }
    Circle(float x,float y,float r,float w,glm::vec4 const&color = glm::vec4(1.f)):Circle(glm::vec2(x,y),r,w,color){}
};

class Triangle: public Primitive{
  public:
    glm::vec2 points[3];
    Triangle(glm::vec2 const&a,glm::vec2 const&b,glm::vec2 const&c,glm::vec4 const&color = glm::vec4(1.f)):Primitive(TRIANGLE,color){
      assert(this!=nullptr);
      this->points[0]=a;
      this->points[1]=b;
      this->points[2]=c;
    }
    Triangle(float ax,float ay,float bx,float by,float cx,float cy,glm::vec4 const&color = glm::vec4(1.f)):Triangle(glm::vec2(ax,ay),glm::vec2(bx,by),glm::vec2(cx,cy),color){}
};

class Text: public Primitive{
  public:
    std::string data;
    float size;
    glm::vec2 position;
    glm::vec2 direction;
    Text(std::string const&data,float size,glm::vec2 const&position,glm::vec2 const&direction,glm::vec4 const&color = glm::vec4(1.f)):Primitive(TEXT,color){
      assert(this!=nullptr);
      this->data = data;
      this->size = size;
      this->position = position;
      this->direction = direction;
    }
    Text(std::string const&data,float size,glm::vec4 const&color = glm::vec4(1.f),glm::vec2 const&position = glm::vec2(0.f),glm::vec2 const&direction = glm::vec2(1.f,0.f)):Text(data,size,position,direction,color){}
};

class Spline: public Primitive{
  public:
    Spline(glm::vec2 const&a,glm::vec2 const&b,glm::vec2 const&c,glm::vec2 const&d,float width,glm::vec4 const&color = glm::vec4(1.f)):Primitive(SPLINE,color){
      assert(this!=nullptr);
      this->points[0] = a;
      this->points[1] = b;
      this->points[2] = c;
      this->points[3] = d;
      this->width = width;
    }
    glm::vec2 points[4];
    float width;
};



class Scene2D;
class Draw2D{
  public:
    Draw2D(ge::gl::Context const&gl);
    ~Draw2D();
    void draw();
    glm::uvec2 getViewportSize()const;
    glm::vec2  getCameraPosition()const;
    float      getCameraScale()const;
    float      getCameraAngle()const;
    void setViewportSize(glm::uvec2 const&size);
    void setCameraPosition(glm::vec2 const&cameraPosition = glm::vec2(0.f));
    void setCameraScale(float cameraScale = 1.f);
    void setCameraAngle(float cameraAngle = 0.f);

    size_t getRootViewport()const;
    void   setRootViewport(size_t viewport);
    bool   hasRootViewport()const;
    void   eraseRootViewport();

    bool       isViewport               (size_t viewport)const;
    glm::uvec2 getViewportSize          (size_t viewport)const;
    glm::vec2  getViewportCameraPosition(size_t viewport)const;
    float      getViewportCameraScale   (size_t viewport)const;
    float      getViewportCameraAngle   (size_t viewport)const;
    size_t     getNofLayers             (size_t viewport)const;
    size_t     getLayer                 (size_t viewport,size_t i)const;
    void       setViewportSize          (size_t viewport,glm::uvec2 const&size);
    void       setViewportCameraPosition(size_t viewport,glm::vec2 const&cameraPosition = glm::vec2(0.f));
    void       setViewportCameraScale   (size_t viewport,float cameraScale = 1.f);
    void       setViewportCameraAngle   (size_t viewport,float cameraAngle = 0.f);
    void       insertLayer              (size_t viewport,size_t layer);
    void       eraseLayer               (size_t viewport,size_t layer);
    void       deleteViewport           (size_t viewport);
    size_t     createViewport(glm::uvec2 const&size,glm::vec2 const&cameraPosition = glm::vec2(0.f),float cameraScale = 1,float cameraAngle = 0);

    bool   isLayer     (size_t layer)const;
    size_t getNode     (size_t layer)const;
    bool   hasNode     (size_t layer)const;
    void   setLayerNode(size_t layer,size_t node);
    void   eraseNode   (size_t layer);
    void   deleteLayer (size_t layer);
    size_t createLayer();

    bool      isNode          (size_t node)const;
    size_t    getNofNodes     (size_t node)const;
    size_t    getNode         (size_t node,size_t i)const;
    size_t    getNofViewports (size_t node)const;
    size_t    getViewport     (size_t node,size_t i)const;
    size_t    getNofPrimitives(size_t node)const;
    size_t    getPrimitive    (size_t node,size_t i)const;
    glm::mat3 getNodeMatrix   (size_t node)const;
    void      setNodeMatrix   (size_t node,glm::mat3 const&mat);
    void      insertViewport  (size_t node,size_t viewport);
    void      insertNode      (size_t node,size_t childNode);
    void      insertPrimitive (size_t node,size_t primitive);
    void      eraseViewport   (size_t node,size_t viewport);
    void      eraseNode       (size_t node,size_t childNode);
    void      erasePrimitive  (size_t node,size_t primitive);
    void      deleteNode      (size_t node);
    size_t createNode(glm::mat3 const&mat = glm::mat3(1.f));

    bool                      isPrimitive     (size_t primitive)const;
    std::shared_ptr<Primitive>getPrimitiveData(size_t primitive)const;
    void                      primitiveChanged(size_t primitive);
    void                      deletePrimitive (size_t primitive);
    size_t createPrimitive(std::shared_ptr<Primitive>const&primitive);

    static glm::mat3 translate(glm::vec2 const&pos);
    static glm::mat3 rotate(float angle);
    static glm::mat3 scale(float scale);

    void clear();
  protected:
    Scene2D*_impl;
};

inline glm::mat3 Draw2D::translate(glm::vec2 const&pos){
  auto result = glm::mat3(1.f);
  result[2]=glm::vec3(pos,1);
  return result;
}

inline glm::mat3 Draw2D::rotate(float angle){
  auto result = glm::mat3(1.f);
  result[0].x =  glm::cos(angle);
  result[0].y = -glm::sin(angle);
  result[1].x =  glm::sin(angle);
  result[1].y =  glm::cos(angle);
  return result;
}

inline glm::mat3 Draw2D::scale(float scale){
  auto result = glm::mat3(1.f);
  result[0].x=scale;
  result[1].y=scale;
  return result;
}

