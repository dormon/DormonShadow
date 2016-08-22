#pragma once

#include<iostream>
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


