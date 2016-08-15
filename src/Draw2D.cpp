#include<Draw2D.h>
#include<Draw2DShaders.h>
#include<geCore/ErrorPrinter.h>
#include<Font.h>

using namespace ge::gl;

std::shared_ptr<ge::gl::Texture>createFontTexture(){
  const uint32_t w=ge::res::font::width;
  const uint32_t h=ge::res::font::height;
  uint8_t bytes[w*h];
  auto result = std::make_shared<ge::gl::Texture>(GL_TEXTURE_2D,GL_R8,0,w,h);
  for(uint32_t i=0;i<w*h/8;++i)
    for(size_t k=0;k<8;++k)
      bytes[i*8+k]=255*((ge::res::font::data[i]>>k)&0x1);
  result->setData2D(bytes,GL_RED,GL_UNSIGNED_BYTE,0,0,0,w,h,w);
  result->generateMipmap();
  result->texParameteri(GL_TEXTURE_MIN_FILTER,GL_LINEAR_MIPMAP_LINEAR);
  result->texParameteri(GL_TEXTURE_MAG_FILTER,GL_LINEAR);
  result->texParameteri(GL_TEXTURE_WRAP_S,GL_CLAMP_TO_EDGE);
  result->texParameteri(GL_TEXTURE_WRAP_T,GL_CLAMP_TO_EDGE);
  return result;
}

class Viewport;

struct DrawData{
  std::shared_ptr<Program>lineProgram;
  std::shared_ptr<Program>pointProgram;
  std::shared_ptr<Program>circleProgram;
  std::shared_ptr<Program>triangleProgram;
  std::shared_ptr<Program>splineProgram;
  std::shared_ptr<Program>textProgram;
  std::shared_ptr<Texture>fontTexture;
  Context const&gl;
  glm::mat3 viewMatrix;
};

class TransformNode{
  public:
    glm::mat3 mat;
    std::vector<std::shared_ptr<TransformNode>>childs;
    std::map<size_t,std::shared_ptr<Primitive>>primitives;
    std::map<size_t,std::shared_ptr<Viewport>>viewports;
    std::shared_ptr<Buffer>lineBuffer;
    std::shared_ptr<VertexArray>lineVAO;
    size_t nofLines;
    std::shared_ptr<Buffer>pointBuffer;
    std::shared_ptr<VertexArray>pointVAO;
    size_t nofPoints;
    std::shared_ptr<Buffer>circleBuffer;
    std::shared_ptr<VertexArray>circleVAO;
    size_t nofCircles;
    std::shared_ptr<Buffer>triangleBuffer;
    std::shared_ptr<VertexArray>triangleVAO;
    size_t nofTriangles;
    std::shared_ptr<Buffer>splineBuffer;
    std::shared_ptr<VertexArray>splineVAO;
    size_t nofSplines;
    std::shared_ptr<Buffer>textBuffer;
    std::shared_ptr<VertexArray>textVAO;
    size_t nofCharacters;
    bool changed = true;
    void draw(glm::mat3 const&m,DrawData const&d);
    TransformNode(){
      this->mat = glm::mat3(1.f);
    }
};

class Layer{
  public:
    Layer(){}
    ~Layer(){}
    std::shared_ptr<TransformNode>root;
    void draw(glm::mat3 const&m,DrawData const&d){
      if(root)root->draw(m,d);
    }
};

class Viewport{
  public:
    glm::vec2 position;
    glm::vec2 size;
    std::vector<std::shared_ptr<Layer>>layers;
    void draw(glm::mat3 const&m,DrawData const&d){
      auto const&gl = d.gl;
      glm::vec3 wp[4];
      wp[0]=m*glm::vec3(position,1.f);
      wp[1]=m*glm::vec3(position+glm::vec2(size.x,0.f   ),1.f);
      wp[2]=m*glm::vec3(position+glm::vec2(0.f   ,size.y),1.f);
      wp[3]=m*glm::vec3(position+glm::vec2(size.x,size.y),1.f);
      glm::vec2 op=glm::vec2(
          glm::min(glm::min(wp[0].x,wp[1].x),glm::min(wp[2].x,wp[3].x)),
          glm::min(glm::min(wp[0].y,wp[1].y),glm::min(wp[2].y,wp[3].y)));
      glm::vec2 os=glm::vec2(
          glm::max(glm::max(wp[0].x,wp[1].x),glm::max(wp[2].x,wp[3].x)),
          glm::max(glm::max(wp[0].y,wp[1].y),glm::max(wp[2].y,wp[3].y)))-op;
      gl.glViewport(op.x,op.y,os.x,os.y);
      for(auto const&x:this->layers)
        x->draw(m,d);
      /*
         gl.glEnable(GL_STENCIL_TEST);
         gl.glStencilFunc(GL_ALWAYS,0,0);
         gl.glClear(GL_STENCIL_BUFFER_BIT);
         gl.glStencilOp(GL_KEEP,GL_KEEP,GL_INCR);

         gl.glDisable(GL_STENCIL_TEST);
         */


    }
};

class Scene2D{
  public:
    Context gl;
    std::shared_ptr<Program>lineProgram;
    std::shared_ptr<Program>pointProgram;
    std::shared_ptr<Program>circleProgram;
    std::shared_ptr<Program>triangleProgram;
    std::shared_ptr<Program>splineProgram;
    std::shared_ptr<Program>textProgram;
    std::shared_ptr<Texture>fontTexture;
    std::map<size_t,std::shared_ptr<Viewport>>viewports;
    std::map<size_t,std::shared_ptr<Layer>>layers;
    std::map<size_t,std::shared_ptr<TransformNode>>nodes;
    std::map<size_t,std::shared_ptr<Primitive>>primitives;
    std::map<size_t,std::set<size_t>>viewportParents;
    std::map<size_t,std::set<size_t>>layerParents;
    std::map<size_t,std::set<size_t>>nodeParentNodes;
    std::map<size_t,std::set<size_t>>nodeParentLayers;
    Scene2D(Context const&g):gl(g){
      assert(this!=nullptr);
      this->lineProgram = std::make_shared<Program>();
      this->lineProgram->link(
          {std::make_shared<Shader>(GL_VERTEX_SHADER,lineVS),
          std::make_shared<Shader>(GL_GEOMETRY_SHADER,lineGS),
          std::make_shared<Shader>(GL_FRAGMENT_SHADER,lineFS)});

      this->pointProgram = std::make_shared<Program>();
      this->pointProgram->link(
          {std::make_shared<Shader>(GL_VERTEX_SHADER,pointVS),
          std::make_shared<Shader>(GL_GEOMETRY_SHADER,pointGS),
          std::make_shared<Shader>(GL_FRAGMENT_SHADER,pointFS)});

      this->circleProgram = std::make_shared<Program>();
      this->circleProgram->link(
          {std::make_shared<Shader>(GL_VERTEX_SHADER,circleVS),
          std::make_shared<Shader>(GL_GEOMETRY_SHADER,circleGS),
          std::make_shared<Shader>(GL_FRAGMENT_SHADER,circleFS)});

      this->triangleProgram = std::make_shared<Program>();
      this->triangleProgram->link(
          {std::make_shared<Shader>(GL_VERTEX_SHADER,triangleVS),
          std::make_shared<Shader>(GL_FRAGMENT_SHADER,triangleFS)});

      this->splineProgram = std::make_shared<Program>();
      this->splineProgram->link(
          {std::make_shared<Shader>(GL_VERTEX_SHADER,splineVS),
          std::make_shared<Shader>(GL_TESS_CONTROL_SHADER,splineCS),
          std::make_shared<Shader>(GL_TESS_EVALUATION_SHADER,splineES),
          std::make_shared<Shader>(GL_GEOMETRY_SHADER,splineGS),
          std::make_shared<Shader>(GL_FRAGMENT_SHADER,splineFS)});

      this->fontTexture = createFontTexture();
      this->textProgram = std::make_shared<Program>();
      this->textProgram->link(
          {std::make_shared<Shader>(GL_VERTEX_SHADER,textVS),
          std::make_shared<Shader>(GL_GEOMETRY_SHADER,textGS),
          std::make_shared<Shader>(GL_FRAGMENT_SHADER,textFS)});
    }
};



void TransformNode::draw(glm::mat3 const&m,DrawData const&d){
  glm::mat3 newMat = this->mat*m;
  if(changed){
    std::vector<float>lineData;
    std::vector<float>pointData;
    std::vector<float>circleData;
    std::vector<float>triangleData;
    std::vector<float>splineData;
    std::vector<float>textData;
    for(auto const&x:this->primitives){
      if(x.second->type == Primitive::LINE){
        auto l = std::dynamic_pointer_cast<Line>(x.second);
        lineData.push_back(l->color[0]);
        lineData.push_back(l->color[1]);
        lineData.push_back(l->color[2]);
        lineData.push_back(l->color[3]);
        lineData.push_back(l->points[0].x);
        lineData.push_back(l->points[0].y);
        lineData.push_back(l->points[1].x);
        lineData.push_back(l->points[1].y);
        lineData.push_back(l->width);
      }
      if(x.second->type == Primitive::POINT){
        auto l = std::dynamic_pointer_cast<Point>(x.second);
        pointData.push_back(l->color[0]);
        pointData.push_back(l->color[1]);
        pointData.push_back(l->color[2]);
        pointData.push_back(l->color[3]);
        pointData.push_back(l->point.x);
        pointData.push_back(l->point.y);
        pointData.push_back(l->size);
      }
      if(x.second->type == Primitive::CIRCLE){
        auto l = std::dynamic_pointer_cast<Circle>(x.second);
        circleData.push_back(l->color[0]);
        circleData.push_back(l->color[1]);
        circleData.push_back(l->color[2]);
        circleData.push_back(l->color[3]);
        circleData.push_back(l->point.x);
        circleData.push_back(l->point.y);
        circleData.push_back(l->size);
        circleData.push_back(l->width);
      }
      if(x.second->type == Primitive::TRIANGLE){
        auto l = std::dynamic_pointer_cast<Triangle>(x.second);
        for(size_t i=0;i<3;++i){
          triangleData.push_back(l->color[0]);
          triangleData.push_back(l->color[1]);
          triangleData.push_back(l->color[2]);
          triangleData.push_back(l->color[3]);
          triangleData.push_back(l->points[i].x);
          triangleData.push_back(l->points[i].y);
        }
      }
      if(x.second->type == Primitive::SPLINE){
        auto l = std::dynamic_pointer_cast<Spline>(x.second);
        splineData.push_back(l->color[0]);
        splineData.push_back(l->color[1]);
        splineData.push_back(l->color[2]);
        splineData.push_back(l->color[3]);
        for(size_t k=0;k<4;++k){
          splineData.push_back(l->points[k].x);
          splineData.push_back(l->points[k].y);
        }
        splineData.push_back(l->width);
      }
      if(x.second->type == Primitive::TEXT){
        auto l = std::dynamic_pointer_cast<Text>(x.second);
        int32_t c=0;
        for(auto const&x:l->data){
          textData.push_back(l->color[0]);
          textData.push_back(l->color[1]);
          textData.push_back(l->color[2]);
          textData.push_back(l->color[3]);
          textData.push_back(l->position[0]);
          textData.push_back(l->position[1]);
          textData.push_back(l->direction[0]);
          textData.push_back(l->direction[1]);
          textData.push_back(c++);
          textData.push_back(x);
          textData.push_back(l->size);
        }
      }
    }

    this->lineBuffer = std::make_shared<Buffer>(lineData.size()*sizeof(float),lineData.data());
    this->nofLines = lineData.size()/9;
    this->lineVAO = std::make_shared<VertexArray>();
    this->lineVAO->addAttrib(this->lineBuffer,
        0,4,GL_FLOAT,sizeof(float)*9,sizeof(float)*0);
    this->lineVAO->addAttrib(this->lineBuffer,
        1,2,GL_FLOAT,sizeof(float)*9,sizeof(float)*4);
    this->lineVAO->addAttrib(this->lineBuffer,
        2,2,GL_FLOAT,sizeof(float)*9,sizeof(float)*6);
    this->lineVAO->addAttrib(this->lineBuffer,
        3,1,GL_FLOAT,sizeof(float)*9,sizeof(float)*8);

    this->pointBuffer = std::make_shared<Buffer>(pointData.size()*sizeof(float),pointData.data());
    this->nofPoints = pointData.size()/7;
    this->pointVAO = std::make_shared<VertexArray>();
    this->pointVAO->addAttrib(this->pointBuffer,
        0,4,GL_FLOAT,sizeof(float)*7,sizeof(float)*0);
    this->pointVAO->addAttrib(this->pointBuffer,
        1,2,GL_FLOAT,sizeof(float)*7,sizeof(float)*4);
    this->pointVAO->addAttrib(this->pointBuffer,
        2,1,GL_FLOAT,sizeof(float)*7,sizeof(float)*6);

    this->circleBuffer = std::make_shared<Buffer>(circleData.size()*sizeof(float),circleData.data());
    this->nofCircles = circleData.size()/8;
    this->circleVAO = std::make_shared<VertexArray>();
    this->circleVAO->addAttrib(this->circleBuffer,
        0,4,GL_FLOAT,sizeof(float)*8,sizeof(float)*0);
    this->circleVAO->addAttrib(this->circleBuffer,
        1,2,GL_FLOAT,sizeof(float)*8,sizeof(float)*4);
    this->circleVAO->addAttrib(this->circleBuffer,
        2,1,GL_FLOAT,sizeof(float)*8,sizeof(float)*6);
    this->circleVAO->addAttrib(this->circleBuffer,
        3,1,GL_FLOAT,sizeof(float)*8,sizeof(float)*7);

    this->triangleBuffer = std::make_shared<Buffer>(triangleData.size()*sizeof(float),triangleData.data());
    this->nofTriangles = triangleData.size()/6/3;
    this->triangleVAO = std::make_shared<VertexArray>();
    this->triangleVAO->addAttrib(this->triangleBuffer,
        0,4,GL_FLOAT,sizeof(float)*6,sizeof(float)*0);
    this->triangleVAO->addAttrib(this->triangleBuffer,
        1,2,GL_FLOAT,sizeof(float)*6,sizeof(float)*4);

    this->splineBuffer = std::make_shared<Buffer>(splineData.size()*sizeof(float),splineData.data());
    this->nofSplines = splineData.size()/13;
    this->splineVAO = std::make_shared<VertexArray>();
    this->splineVAO->addAttrib(this->splineBuffer,
        0,4,GL_FLOAT,sizeof(float)*13,sizeof(float)*0 );
    this->splineVAO->addAttrib(this->splineBuffer,
        1,2,GL_FLOAT,sizeof(float)*13,sizeof(float)*4 );
    this->splineVAO->addAttrib(this->splineBuffer,
        2,2,GL_FLOAT,sizeof(float)*13,sizeof(float)*6 );
    this->splineVAO->addAttrib(this->splineBuffer,
        3,2,GL_FLOAT,sizeof(float)*13,sizeof(float)*8 );
    this->splineVAO->addAttrib(this->splineBuffer,
        4,2,GL_FLOAT,sizeof(float)*13,sizeof(float)*10);
    this->splineVAO->addAttrib(this->splineBuffer,
        5,1,GL_FLOAT,sizeof(float)*13,sizeof(float)*12);

    this->textBuffer = std::make_shared<Buffer>(textData.size()*sizeof(float),textData.data());
    this->nofCharacters = textData.size()/11;
    this->textVAO = std::make_shared<VertexArray>();
    this->textVAO->addAttrib(this->textBuffer,
        0,4,GL_FLOAT,sizeof(float)*11,sizeof(float)*0);
    this->textVAO->addAttrib(this->textBuffer,
        1,2,GL_FLOAT,sizeof(float)*11,sizeof(float)*4);
    this->textVAO->addAttrib(this->textBuffer,
        2,2,GL_FLOAT,sizeof(float)*11,sizeof(float)*6);
    this->textVAO->addAttrib(this->textBuffer,
        3,1,GL_FLOAT,sizeof(float)*11,sizeof(float)*8);
    this->textVAO->addAttrib(this->textBuffer,
        4,1,GL_FLOAT,sizeof(float)*11,sizeof(float)*9);
    this->textVAO->addAttrib(this->textBuffer,
        5,1,GL_FLOAT,sizeof(float)*11,sizeof(float)*10);

    changed = false;
  }

  glm::mat3 matrix = d.viewMatrix*newMat;
  d.lineProgram->use();
  d.lineProgram->setMatrix3fv("matrix",glm::value_ptr(matrix));
  this->lineVAO->bind();
  d.gl.glDrawArrays(GL_POINTS,0,this->nofLines);
  this->lineVAO->unbind();

  d.pointProgram->use();
  d.pointProgram->setMatrix3fv("matrix",glm::value_ptr(matrix));
  this->pointVAO->bind();
  d.gl.glDrawArrays(GL_POINTS,0,this->nofPoints);
  this->pointVAO->unbind();

  d.circleProgram->use();
  d.circleProgram->setMatrix3fv("matrix",glm::value_ptr(matrix));
  this->circleVAO->bind();
  d.gl.glDrawArrays(GL_POINTS,0,this->nofCircles);
  this->circleVAO->unbind();

  d.triangleProgram->use();
  d.triangleProgram->setMatrix3fv("matrix",glm::value_ptr(matrix));
  this->triangleVAO->bind();
  d.gl.glDrawArrays(GL_TRIANGLES,0,this->nofTriangles*3);
  this->triangleVAO->unbind();

  d.splineProgram->use();
  d.splineProgram->setMatrix3fv("matrix",glm::value_ptr(matrix));
  this->splineVAO->bind();
  d.gl.glPatchParameteri(GL_PATCH_VERTICES,1);
  d.gl.glDrawArrays(GL_PATCHES,0,this->nofSplines);
  this->splineVAO->unbind();

  d.textProgram->use();
  d.textProgram->setMatrix3fv("matrix",glm::value_ptr(matrix));
  this->textVAO->bind();
  d.fontTexture->bind(0);
  d.gl.glEnable(GL_BLEND);
  d.gl.glBlendFunc(GL_SRC_ALPHA,GL_ONE_MINUS_SRC_ALPHA);
  d.gl.glDrawArrays(GL_POINTS,0,this->nofCharacters);
  d.gl.glDisable(GL_BLEND);
  this->textVAO->unbind();

  for(auto const&x:this->viewports)
    x.second->draw(newMat,d);
}


class Draw2DImpl{
  public:
    Context gl;
    struct WP{
      uint32_t w;
      uint32_t h;
      uint32_t x = 0;
      uint32_t y = 0;
    }viewport;
    std::shared_ptr<Viewport>base;
    std::map<size_t,std::shared_ptr<Primitive>>primitives;
    float pixelSize = 1.f;
    float x = 0;
    float y = 0;
    Scene2D scene;
    Draw2DImpl(Context const&g,uint32_t w,uint32_t h):gl(g),scene(g){
      assert(this!=nullptr);
      this->viewport.w = w;
      this->viewport.h = h;
      this->base = std::make_shared<Viewport>();
      this->base->position = glm::vec2(0.f);
      this->base->size = glm::vec2(w,h);
      this->base->layers.push_back(std::make_shared<Layer>());
      this->base->layers.at(0)->root = std::make_shared<TransformNode>();
    }
    ~Draw2DImpl(){
    }
    bool convertedForDrawing = false;
    std::shared_ptr<Buffer>lineBuffer;
    std::shared_ptr<VertexArray>lineVAO;
    std::shared_ptr<Program>lineProgram;
    size_t nofLines;
    std::shared_ptr<Buffer>pointBuffer;
    std::shared_ptr<VertexArray>pointVAO;
    std::shared_ptr<Program>pointProgram;
    size_t nofPoints;
    std::shared_ptr<Buffer>circleBuffer;
    std::shared_ptr<VertexArray>circleVAO;
    std::shared_ptr<Program>circleProgram;
    size_t nofCircles;
    std::shared_ptr<Buffer>triangleBuffer;
    std::shared_ptr<VertexArray>triangleVAO;
    std::shared_ptr<Program>triangleProgram;
    size_t nofTriangles;
    std::shared_ptr<Buffer>splineBuffer;
    std::shared_ptr<VertexArray>splineVAO;
    std::shared_ptr<Program>splineProgram;
    size_t nofSplines;
    std::shared_ptr<Buffer>textBuffer;
    std::shared_ptr<VertexArray>textVAO;
    std::shared_ptr<Program>textProgram;
    std::shared_ptr<Texture>fontTexture;
    size_t nofCharacters;
};

Draw2D::Draw2D(Context const&gl,uint32_t w,uint32_t h){
  assert(this!=nullptr);
  this->_impl = new Draw2DImpl(gl,w,h);
  this->_impl->lineProgram = std::make_shared<Program>();
  this->_impl->lineProgram->link(
      {std::make_shared<Shader>(GL_VERTEX_SHADER,lineVS),
      std::make_shared<Shader>(GL_GEOMETRY_SHADER,lineGS),
      std::make_shared<Shader>(GL_FRAGMENT_SHADER,lineFS)});

  this->_impl->pointProgram = std::make_shared<Program>();
  this->_impl->pointProgram->link(
      {std::make_shared<Shader>(GL_VERTEX_SHADER,pointVS),
      std::make_shared<Shader>(GL_GEOMETRY_SHADER,pointGS),
      std::make_shared<Shader>(GL_FRAGMENT_SHADER,pointFS)});

  this->_impl->circleProgram = std::make_shared<Program>();
  this->_impl->circleProgram->link(
      {std::make_shared<Shader>(GL_VERTEX_SHADER,circleVS),
      std::make_shared<Shader>(GL_GEOMETRY_SHADER,circleGS),
      std::make_shared<Shader>(GL_FRAGMENT_SHADER,circleFS)});

  this->_impl->triangleProgram = std::make_shared<Program>();
  this->_impl->triangleProgram->link(
      {std::make_shared<Shader>(GL_VERTEX_SHADER,triangleVS),
      std::make_shared<Shader>(GL_FRAGMENT_SHADER,triangleFS)});

  this->_impl->splineProgram = std::make_shared<Program>();
  this->_impl->splineProgram->link(
      {std::make_shared<Shader>(GL_VERTEX_SHADER,splineVS),
      std::make_shared<Shader>(GL_TESS_CONTROL_SHADER,splineCS),
      std::make_shared<Shader>(GL_TESS_EVALUATION_SHADER,splineES),
      std::make_shared<Shader>(GL_GEOMETRY_SHADER,splineGS),
      std::make_shared<Shader>(GL_FRAGMENT_SHADER,splineFS)});

  this->_impl->fontTexture = createFontTexture();
  this->_impl->textProgram = std::make_shared<Program>();
  this->_impl->textProgram->link(
      {std::make_shared<Shader>(GL_VERTEX_SHADER,textVS),
      std::make_shared<Shader>(GL_GEOMETRY_SHADER,textGS),
      std::make_shared<Shader>(GL_FRAGMENT_SHADER,textFS)});
}

Draw2D::~Draw2D(){
  assert(this!=nullptr);
  delete this->_impl;
}

void Draw2D::setViewportSize(uint32_t x,uint32_t y,uint32_t w,uint32_t h){
  assert(this!=nullptr);
  assert(this->_impl!=nullptr);
  this->_impl->viewport.x = x;
  this->_impl->viewport.y = y;
  this->_impl->viewport.w = w;
  this->_impl->viewport.h = h;
}

void Draw2D::draw(){
  assert(this!=nullptr);
  assert(this->_impl!=nullptr);
  auto const&gl=this->_impl->gl;
  auto const&viewport = this->_impl->viewport;
  gl.glViewport(viewport.x,viewport.y,viewport.w,viewport.h);
  gl.glClearColor(0,0,0,0);
  gl.glClear(GL_DEPTH_BUFFER_BIT|GL_COLOR_BUFFER_BIT);
  auto translate=glm::mat3(1.f);
  translate[2].x = this->_impl->x;
  translate[2].y = this->_impl->y;
  auto scale = glm::mat3(1.f);
  scale[0].x = this->_impl->pixelSize/this->_impl->viewport.w*2;
  scale[1].y = this->_impl->pixelSize/this->_impl->viewport.h*2;
  glm::mat3 viewMatrix = scale;
  glm::mat3 modelMatrix = translate;
  glm::mat3 matrix = viewMatrix*modelMatrix;

  DrawData dd = {
    this->_impl->lineProgram,
    this->_impl->pointProgram,
    this->_impl->circleProgram,
    this->_impl->triangleProgram,
    this->_impl->splineProgram,
    this->_impl->textProgram,
    this->_impl->fontTexture,
    this->_impl->gl,
    viewMatrix,
  };

  this->_impl->base->draw(glm::mat3(1.f),dd);

  if(!this->_impl->convertedForDrawing){
    std::vector<float>lineData;
    for(auto const&x:this->_impl->primitives){
      if(x.second->type == Primitive::LINE){
        auto l = std::dynamic_pointer_cast<Line>(x.second);
        lineData.push_back(l->color[0]);
        lineData.push_back(l->color[1]);
        lineData.push_back(l->color[2]);
        lineData.push_back(l->color[3]);
        lineData.push_back(l->points[0].x);
        lineData.push_back(l->points[0].y);
        lineData.push_back(l->points[1].x);
        lineData.push_back(l->points[1].y);
        lineData.push_back(l->width);
      }
    }
    this->_impl->lineBuffer = std::make_shared<Buffer>(lineData.size()*sizeof(float),lineData.data());
    this->_impl->nofLines = lineData.size()/9;
    this->_impl->lineVAO = std::make_shared<VertexArray>();
    this->_impl->lineVAO->addAttrib(this->_impl->lineBuffer,
        0,4,GL_FLOAT,sizeof(float)*9,sizeof(float)*0);
    this->_impl->lineVAO->addAttrib(this->_impl->lineBuffer,
        1,2,GL_FLOAT,sizeof(float)*9,sizeof(float)*4);
    this->_impl->lineVAO->addAttrib(this->_impl->lineBuffer,
        2,2,GL_FLOAT,sizeof(float)*9,sizeof(float)*6);
    this->_impl->lineVAO->addAttrib(this->_impl->lineBuffer,
        3,1,GL_FLOAT,sizeof(float)*9,sizeof(float)*8);

    std::vector<float>pointData;
    for(auto const&x:this->_impl->primitives){
      if(x.second->type == Primitive::POINT){
        auto l = std::dynamic_pointer_cast<Point>(x.second);
        pointData.push_back(l->color[0]);
        pointData.push_back(l->color[1]);
        pointData.push_back(l->color[2]);
        pointData.push_back(l->color[3]);
        pointData.push_back(l->point.x);
        pointData.push_back(l->point.y);
        pointData.push_back(l->size);
      }
    }
    this->_impl->pointBuffer = std::make_shared<Buffer>(pointData.size()*sizeof(float),pointData.data());
    this->_impl->nofPoints = pointData.size()/7;
    this->_impl->pointVAO = std::make_shared<VertexArray>();
    this->_impl->pointVAO->addAttrib(this->_impl->pointBuffer,
        0,4,GL_FLOAT,sizeof(float)*7,sizeof(float)*0);
    this->_impl->pointVAO->addAttrib(this->_impl->pointBuffer,
        1,2,GL_FLOAT,sizeof(float)*7,sizeof(float)*4);
    this->_impl->pointVAO->addAttrib(this->_impl->pointBuffer,
        2,1,GL_FLOAT,sizeof(float)*7,sizeof(float)*6);

    std::vector<float>circleData;
    for(auto const&x:this->_impl->primitives){
      if(x.second->type == Primitive::CIRCLE){
        auto l = std::dynamic_pointer_cast<Circle>(x.second);
        circleData.push_back(l->color[0]);
        circleData.push_back(l->color[1]);
        circleData.push_back(l->color[2]);
        circleData.push_back(l->color[3]);
        circleData.push_back(l->point.x);
        circleData.push_back(l->point.y);
        circleData.push_back(l->size);
        circleData.push_back(l->width);
      }
    }
    this->_impl->circleBuffer = std::make_shared<Buffer>(circleData.size()*sizeof(float),circleData.data());
    this->_impl->nofCircles = circleData.size()/8;
    this->_impl->circleVAO = std::make_shared<VertexArray>();
    this->_impl->circleVAO->addAttrib(this->_impl->circleBuffer,
        0,4,GL_FLOAT,sizeof(float)*8,sizeof(float)*0);
    this->_impl->circleVAO->addAttrib(this->_impl->circleBuffer,
        1,2,GL_FLOAT,sizeof(float)*8,sizeof(float)*4);
    this->_impl->circleVAO->addAttrib(this->_impl->circleBuffer,
        2,1,GL_FLOAT,sizeof(float)*8,sizeof(float)*6);
    this->_impl->circleVAO->addAttrib(this->_impl->circleBuffer,
        3,1,GL_FLOAT,sizeof(float)*8,sizeof(float)*7);


    std::vector<float>triangleData;
    for(auto const&x:this->_impl->primitives){
      if(x.second->type == Primitive::TRIANGLE){
        auto l = std::dynamic_pointer_cast<Triangle>(x.second);
        for(size_t i=0;i<3;++i){
          triangleData.push_back(l->color[0]);
          triangleData.push_back(l->color[1]);
          triangleData.push_back(l->color[2]);
          triangleData.push_back(l->color[3]);
          triangleData.push_back(l->points[i].x);
          triangleData.push_back(l->points[i].y);
        }
      }
    }
    this->_impl->triangleBuffer = std::make_shared<Buffer>(triangleData.size()*sizeof(float),triangleData.data());
    this->_impl->nofTriangles = triangleData.size()/6/3;
    this->_impl->triangleVAO = std::make_shared<VertexArray>();
    this->_impl->triangleVAO->addAttrib(this->_impl->triangleBuffer,
        0,4,GL_FLOAT,sizeof(float)*6,sizeof(float)*0);
    this->_impl->triangleVAO->addAttrib(this->_impl->triangleBuffer,
        1,2,GL_FLOAT,sizeof(float)*6,sizeof(float)*4);


    std::vector<float>splineData;
    for(auto const&x:this->_impl->primitives){
      if(x.second->type == Primitive::SPLINE){
        auto l = std::dynamic_pointer_cast<Spline>(x.second);
        splineData.push_back(l->color[0]);
        splineData.push_back(l->color[1]);
        splineData.push_back(l->color[2]);
        splineData.push_back(l->color[3]);
        for(size_t k=0;k<4;++k){
          splineData.push_back(l->points[k].x);
          splineData.push_back(l->points[k].y);
        }
        splineData.push_back(l->width);
      }
    }
    this->_impl->splineBuffer = std::make_shared<Buffer>(splineData.size()*sizeof(float),splineData.data());
    this->_impl->nofSplines = splineData.size()/13;
    this->_impl->splineVAO = std::make_shared<VertexArray>();
    this->_impl->splineVAO->addAttrib(this->_impl->splineBuffer,
        0,4,GL_FLOAT,sizeof(float)*13,sizeof(float)*0 );
    this->_impl->splineVAO->addAttrib(this->_impl->splineBuffer,
        1,2,GL_FLOAT,sizeof(float)*13,sizeof(float)*4 );
    this->_impl->splineVAO->addAttrib(this->_impl->splineBuffer,
        2,2,GL_FLOAT,sizeof(float)*13,sizeof(float)*6 );
    this->_impl->splineVAO->addAttrib(this->_impl->splineBuffer,
        3,2,GL_FLOAT,sizeof(float)*13,sizeof(float)*8 );
    this->_impl->splineVAO->addAttrib(this->_impl->splineBuffer,
        4,2,GL_FLOAT,sizeof(float)*13,sizeof(float)*10);
    this->_impl->splineVAO->addAttrib(this->_impl->splineBuffer,
        5,1,GL_FLOAT,sizeof(float)*13,sizeof(float)*12);

    std::vector<float>textData;
    for(auto const&x:this->_impl->primitives){
      if(x.second->type == Primitive::TEXT){
        auto l = std::dynamic_pointer_cast<Text>(x.second);
        int32_t c=0;
        for(auto const&x:l->data){
          textData.push_back(l->color[0]);
          textData.push_back(l->color[1]);
          textData.push_back(l->color[2]);
          textData.push_back(l->color[3]);
          textData.push_back(l->position[0]);
          textData.push_back(l->position[1]);
          textData.push_back(l->direction[0]);
          textData.push_back(l->direction[1]);
          textData.push_back(c++);
          textData.push_back(x);
          textData.push_back(l->size);
        }
      }
    }
    this->_impl->textBuffer = std::make_shared<Buffer>(textData.size()*sizeof(float),textData.data());
    this->_impl->nofCharacters = textData.size()/11;
    this->_impl->textVAO = std::make_shared<VertexArray>();
    this->_impl->textVAO->addAttrib(this->_impl->textBuffer,
        0,4,GL_FLOAT,sizeof(float)*11,sizeof(float)*0);
    this->_impl->textVAO->addAttrib(this->_impl->textBuffer,
        1,2,GL_FLOAT,sizeof(float)*11,sizeof(float)*4);
    this->_impl->textVAO->addAttrib(this->_impl->textBuffer,
        2,2,GL_FLOAT,sizeof(float)*11,sizeof(float)*6);
    this->_impl->textVAO->addAttrib(this->_impl->textBuffer,
        3,1,GL_FLOAT,sizeof(float)*11,sizeof(float)*8);
    this->_impl->textVAO->addAttrib(this->_impl->textBuffer,
        4,1,GL_FLOAT,sizeof(float)*11,sizeof(float)*9);
    this->_impl->textVAO->addAttrib(this->_impl->textBuffer,
        5,1,GL_FLOAT,sizeof(float)*11,sizeof(float)*10);

    this->_impl->convertedForDrawing = true;
  }
  this->_impl->lineProgram->use();
  this->_impl->lineProgram->setMatrix3fv("matrix",glm::value_ptr(matrix));
  this->_impl->lineVAO->bind();
  this->_impl->gl.glDrawArrays(GL_POINTS,0,this->_impl->nofLines);
  this->_impl->lineVAO->unbind();

  this->_impl->pointProgram->use();
  this->_impl->pointProgram->setMatrix3fv("matrix",glm::value_ptr(matrix));
  this->_impl->pointVAO->bind();
  this->_impl->gl.glDrawArrays(GL_POINTS,0,this->_impl->nofPoints);
  this->_impl->pointVAO->unbind();

  this->_impl->circleProgram->use();
  this->_impl->circleProgram->setMatrix3fv("matrix",glm::value_ptr(matrix));
  this->_impl->circleVAO->bind();
  this->_impl->gl.glDrawArrays(GL_POINTS,0,this->_impl->nofCircles);
  this->_impl->circleVAO->unbind();

  this->_impl->triangleProgram->use();
  this->_impl->triangleProgram->setMatrix3fv("matrix",glm::value_ptr(matrix));
  this->_impl->triangleVAO->bind();
  this->_impl->gl.glDrawArrays(GL_TRIANGLES,0,this->_impl->nofTriangles*3);
  this->_impl->triangleVAO->unbind();

  this->_impl->splineProgram->use();
  this->_impl->splineProgram->setMatrix3fv("matrix",glm::value_ptr(matrix));
  this->_impl->splineVAO->bind();
  this->_impl->gl.glPatchParameteri(GL_PATCH_VERTICES,1);
  this->_impl->gl.glDrawArrays(GL_PATCHES,0,this->_impl->nofSplines);
  this->_impl->splineVAO->unbind();

  this->_impl->textProgram->use();
  this->_impl->textProgram->setMatrix3fv("matrix",glm::value_ptr(matrix));
  this->_impl->textVAO->bind();
  this->_impl->fontTexture->bind(0);
  this->_impl->gl.glEnable(GL_BLEND);
  this->_impl->gl.glBlendFunc(GL_SRC_ALPHA,GL_ONE_MINUS_SRC_ALPHA);
  this->_impl->gl.glDrawArrays(GL_POINTS,0,this->_impl->nofCharacters);
  this->_impl->gl.glDisable(GL_BLEND);
  this->_impl->textVAO->unbind();
}

void Draw2D::setScale(float pixelSize){
  assert(this!=nullptr);
  assert(this->_impl!=nullptr);
  this->_impl->pixelSize = pixelSize;
}

void Draw2D::setPosition(float x,float y){
  assert(this!=nullptr);
  assert(this->_impl!=nullptr);
  this->_impl->x = x;
  this->_impl->y = y;
}

size_t Draw2D::addLine(float ax,float ay,float bx,float by,float w,float r,float g,float b,float a){
  assert(this!=nullptr);
  assert(this->_impl!=nullptr);
  size_t id=this->_impl->primitives.size();
  this->_impl->primitives[id] = std::make_shared<Line>(glm::vec2(ax,ay),glm::vec2(bx,by),w);
  this->_impl->base->layers.at(0)->root->primitives[id] = std::make_shared<Line>(glm::vec2(ax,ay),glm::vec2(bx,by),w);
  this->_impl->base->layers.at(0)->root->primitives[id]->color = glm::vec4(r,g,b,a);
  this->setColor(id,r,g,b,a);
  return id;
}

size_t Draw2D::addPoint(float x,float y,float rd,float r,float g,float b,float a){
  assert(this!=nullptr);
  assert(this->_impl!=nullptr);
  size_t id=this->_impl->primitives.size();
  this->_impl->primitives[id] = std::make_shared<Point>(glm::vec2(x,y),rd);
  this->_impl->base->layers.at(0)->root->primitives[id] = std::make_shared<Point>(glm::vec2(x,y),rd);
  this->_impl->base->layers.at(0)->root->primitives[id]->color = glm::vec4(r,g,b,a);
  this->setColor(id,r,g,b,a);
  return id;
}

size_t Draw2D::addCircle(float x,float y,float rd,float w,float r,float g,float b,float a){
  assert(this!=nullptr);
  assert(this->_impl!=nullptr);
  size_t id=this->_impl->primitives.size();
  this->_impl->primitives[id] = std::make_shared<Circle>(glm::vec2(x,y),rd,w);
  this->_impl->base->layers.at(0)->root->primitives[id] = std::make_shared<Circle>(glm::vec2(x,y),rd,w);
  this->_impl->base->layers.at(0)->root->primitives[id]->color = glm::vec4(r,g,b,a);
  this->setColor(id,r,g,b,a);
  return id;
}

size_t Draw2D::addTriangle(float ax,float ay,float bx,float by,float cx,float cy,float r,float g,float b,float a){
  assert(this!=nullptr);
  assert(this->_impl!=nullptr);
  size_t id=this->_impl->primitives.size();
  this->_impl->primitives[id] = std::make_shared<Triangle>(glm::vec2(ax,ay),glm::vec2(bx,by),glm::vec2(cx,cy));
  this->_impl->base->layers.at(0)->root->primitives[id] = std::make_shared<Triangle>(glm::vec2(ax,ay),glm::vec2(bx,by),glm::vec2(cx,cy));
  this->_impl->base->layers.at(0)->root->primitives[id]->color = glm::vec4(r,g,b,a);
  this->setColor(id,r,g,b,a);
  return id;
}

size_t Draw2D::addText(std::string const&data,float size,float x,float y,float vx,float vy,float r,float g,float b,float a){
  assert(this!=nullptr);
  assert(this->_impl!=nullptr);
  size_t id=this->_impl->primitives.size();
  this->_impl->primitives[id] = std::make_shared<Text>(data,size,glm::vec2(x,y),glm::vec2(vx,vy));
  this->_impl->base->layers.at(0)->root->primitives[id] = std::make_shared<Text>(data,size,glm::vec2(x,y),glm::vec2(vx,vy));
  this->_impl->base->layers.at(0)->root->primitives[id]->color = glm::vec4(r,g,b,a);
  this->setColor(id,r,g,b,a);
  return id;
}

size_t Draw2D::addSpline(float ax,float ay,float bx,float by,float cx,float cy,float dx,float dy,float width,float r,float g,float b,float a){
  assert(this!=nullptr);
  assert(this->_impl!=nullptr);
  size_t id=this->_impl->primitives.size();
  this->_impl->primitives[id] = std::make_shared<Spline>(glm::vec2(ax,ay),glm::vec2(bx,by),glm::vec2(cx,cy),glm::vec2(dx,dy),width);
  this->_impl->base->layers.at(0)->root->primitives[id] = std::make_shared<Spline>(glm::vec2(ax,ay),glm::vec2(bx,by),glm::vec2(cx,cy),glm::vec2(dx,dy),width);
  this->_impl->base->layers.at(0)->root->primitives[id]->color = glm::vec4(r,g,b,a);
  this->setColor(id,r,g,b,a);
  return id;
}


void Draw2D::setColor(size_t id,float r,float g,float b,float a){
  assert(this!=nullptr);
  assert(this->_impl!=nullptr);
  auto ii=this->_impl->primitives.find(id);
  if(ii==this->_impl->primitives.end()){
    ge::core::printError(GE_CORE_FCENAME,"no such primitive",id,r,g,b,a);
    return;
  }
  ii->second->color = glm::vec4(r,g,b,a);
  this->_impl->convertedForDrawing = false;
}

void Draw2D::clear(){
  assert(this!=nullptr);
  assert(this->_impl!=nullptr);
  this->_impl->primitives.clear();
}
