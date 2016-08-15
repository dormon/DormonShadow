#pragma once

#include<cstdint>
#include<geGL/geGL.h>

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
    void erase(size_t id);
    void clear();
  protected:
    Draw2DImpl*_impl;
};
