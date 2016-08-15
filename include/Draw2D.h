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
    Line(glm::vec2 const&a,glm::vec2 const&b,float width):Primitive(LINE){
      assert(this!=nullptr);
      this->points[0]=a;
      this->points[1]=b;
      this->width = width;
    }
    virtual ~Line(){}
};

class Point: public Primitive{
  public:
    glm::vec2 point;
    float size;
    Point(glm::vec2 const&position,float radius):Primitive(POINT){
      assert(this!=nullptr);
      this->point = position;
      this->size = radius;
    }
    virtual ~Point(){}
};

class Circle: public Primitive{
  public:
    glm::vec2 point;
    float size;
    float width;
    Circle(glm::vec2 const&position,float radius,float width):Primitive(CIRCLE){
      assert(this!=nullptr);
      this->point = position;
      this->size = radius;
      this->width = width;
    }
};

class Triangle: public Primitive{
  public:
    glm::vec2 points[3];
    Triangle(glm::vec2 const&a,glm::vec2 const&b,glm::vec2 const&c):Primitive(TRIANGLE){
      assert(this!=nullptr);
      this->points[0]=a;
      this->points[1]=b;
      this->points[2]=c;
    }
};

class Text: public Primitive{
  public:
    std::string data;
    float size;
    glm::vec2 position;
    glm::vec2 direction;
    Text(std::string const&data,float size,glm::vec2 const&position,glm::vec2 const&direction):Primitive(TEXT){
      assert(this!=nullptr);
      this->data = data;
      this->size = size;
      this->position = position;
      this->direction = direction;
    }
};

class Spline: public Primitive{
  public:
    Spline(glm::vec2 const&a,glm::vec2 const&b,glm::vec2 const&c,glm::vec2 const&d,float width):Primitive(SPLINE){
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



class Draw2DImpl;
class Draw2D{
  public:
    Draw2D(ge::gl::Context const&gl,uint32_t w,uint32_t h);
    ~Draw2D();
    void setViewportSize(uint32_t x,uint32_t y,uint32_t w,uint32_t h);
    void draw();
    void setScale(float pixelSize);
    void setPosition(float x = 0,float y = 0);
    size_t addLine(float ax,float ay,float bx,float by,float w = 1,float r = 1,float g = 1,float b = 1,float a = 1);
    size_t addPoint(float x,float y,float rd,float r = 1,float g = 1,float b = 1,float a = 1);
    size_t addCircle(float x,float y,float rd,float w,float r = 1,float g = 1,float b = 1,float a = 1);
    size_t addTriangle(float ax,float ay,float bx,float by,float cx,float cy,float r = 1,float g = 1,float b = 1,float a = 1);
    size_t addText(std::string const&data,float size,float x,float y,float vx,float vy,float r = 1,float g = 1,float b = 1,float a = 1);
    size_t addSpline(float ax,float ay,float bx,float by,float cx,float cy,float dx,float dy,float width,float r = 1,float g = 1,float b = 1,float a = 1);
    void setColor(size_t id,float r = 1,float g = 1,float b = 1,float a = 1);


    size_t getRootViewport()const;
    size_t getNofLayers(size_t viewport)const;
    size_t getLayer(size_t viewport,size_t i)const;
    size_t getNofNodes(size_t node)const;
    size_t getNode(size_t node,size_t i)const;
    size_t getNode(size_t layer)const;
    size_t getNofViewports(size_t node)const;
    size_t getViewport(size_t node,size_t i)const;
    size_t getNofPrimitives(size_t node)const;
    size_t getPrimitive(size_t node,size_t i)const;
    std::shared_ptr<Primitive>getPrimitiveData(size_t primitive)const;

    size_t createLayer();
    size_t createViewport(glm::vec2 const&position,glm::vec2 const&size);
    size_t createNode(glm::mat3 const&mat = glm::mat3(1.f));
    size_t createPrimitive(std::shared_ptr<Primitive>const&primitive);

    void insertLayer(size_t viewport,size_t layer);
    void insertViewport(size_t node,size_t viewport);
    void insertNode(size_t toNode,size_t node);
    void setLayerNode(size_t layer,size_t node);
    void insertPrimitive(size_t node,size_t primitive);

    void eraseLayer(size_t viewport,size_t layer);
    void eraseViewport(size_t node,size_t viewport);
    void eraseNode(size_t fromNode,size_t node);
    void eraseNode(size_t layer);
    void erasePrimitive(size_t node,size_t primitive);

    void deleteLayert(size_t layer);
    void deleteViewport(size_t viewport);
    void deleteNode(size_t node);
    void deletePrimitive(size_t node);

    void clear();
  protected:
    Draw2DImpl*_impl;
};
