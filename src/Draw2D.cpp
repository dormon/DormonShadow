#include<Draw2D.h>
#include<Draw2DShaders.h>
#include<geCore/ErrorPrinter.h>
#include<Font.h>
#include<algorithm>

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
class Layer;
class Node;
class Primitive;

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
    std::map<size_t,std::shared_ptr<Node>>nodes;
    std::map<size_t,std::shared_ptr<Primitive>>primitives;
    size_t viewportCounter = 0;
    size_t layerCounter = 0;
    size_t nodeCounter = 0;
    size_t primitiveCounter = 0;
    std::map<size_t,std::set<size_t>>viewportParents;
    std::map<size_t,std::set<size_t>>layerParents;
    std::map<size_t,std::set<size_t>>nodeParentNodes;
    std::map<size_t,std::set<size_t>>nodeParentLayers;
    std::map<size_t,std::set<size_t>>primitiveParents;
    size_t rootViewport = -1;
    void addParent(std::map<size_t,std::set<size_t>>&parents,size_t child,size_t parent){
      if(parents.count(child)==0)
        parents[child] = std::set<size_t>();
      parents.at(child).insert(parent);
    }
    void removeParent(std::map<size_t,std::set<size_t>>&parents,size_t child,size_t parent){
      if(parents.count(child)==0)return;
      parents.at(child).erase(parent);
      if(parents.at(child).size()==0)
        parents.erase(child);
    }
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

class Node{
  public:
    glm::mat3 mat;
    std::vector<size_t>childs;
    std::vector<size_t>primitives;
    std::vector<size_t>viewports;
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
    void draw(glm::mat3 const&modelMatrix,glm::mat3 const&projectionMatrix,Scene2D const*scene);
    Node(glm::mat3 const&mat = glm::mat3(1.f)){
      this->mat = mat;
    }
};

class Layer{
  public:
    Layer(){}
    ~Layer(){}
    size_t root;
    void draw(glm::mat3 const&modelMatrix,glm::mat3 const&projectionMatrix,Scene2D const*scene){
      auto ii = scene->nodes.find(root);
      if(ii==scene->nodes.end())return;
      ii->second->draw(modelMatrix,projectionMatrix,scene);
    }
};

class Viewport{
  public:
    glm::uvec2 size;
    glm::vec2 cameraPosition;
    float cameraScale;
    float cameraAngle;
    std::vector<size_t>layers;
    Viewport(glm::uvec2 const&size,glm::vec2 const&cameraPosition,float cameraScale,float cameraAngle){
      assert(this!=nullptr);
      this->size = size;
      this->cameraPosition = cameraPosition;
      this->cameraScale = cameraScale;
      this->cameraAngle = cameraAngle;
    }
    void draw(glm::mat3 const&modelMatrix,glm::mat3 const&projectionMatrix,Scene2D const*scene){
      assert(this!=nullptr);
      auto const&gl = scene->gl;
      glm::vec3 wp[4];
      wp[0]=modelMatrix*glm::vec3(0.f,0.f,1.f);
      wp[1]=modelMatrix*glm::vec3(glm::vec2(size.x,0.f   ),1.f);
      wp[2]=modelMatrix*glm::vec3(glm::vec2(0.f   ,size.y),1.f);
      wp[3]=modelMatrix*glm::vec3(glm::vec2(size.x,size.y),1.f);
      glm::vec2 op=glm::vec2(
          glm::min(glm::min(wp[0].x,wp[1].x),glm::min(wp[2].x,wp[3].x)),
          glm::min(glm::min(wp[0].y,wp[1].y),glm::min(wp[2].y,wp[3].y)));
      glm::vec2 os=glm::vec2(
          glm::max(glm::max(wp[0].x,wp[1].x),glm::max(wp[2].x,wp[3].x)),
          glm::max(glm::max(wp[0].y,wp[1].y),glm::max(wp[2].y,wp[3].y)))-op;
      gl.glViewport(op.x,op.y,os.x,os.y);
      auto viewTranslate=glm::mat3(1.f);
      viewTranslate[2] = glm::vec3(-cameraPosition,1.f);
      auto viewRotation = glm::mat3(1.f);
      viewRotation[0].x = glm::cos(this->cameraAngle);
      viewRotation[0].y = -glm::sin(this->cameraAngle);
      viewRotation[1].x = glm::sin(this->cameraAngle);
      viewRotation[1].y = glm::cos(this->cameraAngle);
      auto viewScale = glm::mat3(1.f);
      viewScale[0].x = cameraScale;
      viewScale[1].y = cameraScale;
      glm::mat3 viewMatrix = viewScale*viewRotation*viewTranslate;

      for(auto const&x:this->layers){
        auto ii = scene->layers.find(x);
        assert(ii!=scene->layers.end());
        ii->second->draw(viewMatrix*modelMatrix,projectionMatrix,scene);
      }
      /*
         gl.glEnable(GL_STENCIL_TEST);
         gl.glStencilFunc(GL_ALWAYS,0,0);
         gl.glClear(GL_STENCIL_BUFFER_BIT);
         gl.glStencilOp(GL_KEEP,GL_KEEP,GL_INCR);

         gl.glDisable(GL_STENCIL_TEST);
         */


    }
};



void Node::draw(glm::mat3 const&modelMatrix,glm::mat3 const&projectionMatrix,Scene2D const*scene){
  glm::mat3 newModelMatrix = this->mat*modelMatrix;
  glm::mat3 matrix = projectionMatrix*newModelMatrix;
  if(changed){
    std::vector<float>lineData;
    std::vector<float>pointData;
    std::vector<float>circleData;
    std::vector<float>triangleData;
    std::vector<float>splineData;
    std::vector<float>textData;
    for(auto const&xx:this->primitives){
      auto ii = scene->primitives.find(xx);
      assert(ii != scene->primitives.end());
      auto x = ii->second;
      if(x->type == Primitive::LINE){
        auto l = std::dynamic_pointer_cast<Line>(x);
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
      if(x->type == Primitive::POINT){
        auto l = std::dynamic_pointer_cast<Point>(x);
        pointData.push_back(l->color[0]);
        pointData.push_back(l->color[1]);
        pointData.push_back(l->color[2]);
        pointData.push_back(l->color[3]);
        pointData.push_back(l->point.x);
        pointData.push_back(l->point.y);
        pointData.push_back(l->size);
      }
      if(x->type == Primitive::CIRCLE){
        auto l = std::dynamic_pointer_cast<Circle>(x);
        circleData.push_back(l->color[0]);
        circleData.push_back(l->color[1]);
        circleData.push_back(l->color[2]);
        circleData.push_back(l->color[3]);
        circleData.push_back(l->point.x);
        circleData.push_back(l->point.y);
        circleData.push_back(l->size);
        circleData.push_back(l->width);
      }
      if(x->type == Primitive::TRIANGLE){
        auto l = std::dynamic_pointer_cast<Triangle>(x);
        for(size_t i=0;i<3;++i){
          triangleData.push_back(l->color[0]);
          triangleData.push_back(l->color[1]);
          triangleData.push_back(l->color[2]);
          triangleData.push_back(l->color[3]);
          triangleData.push_back(l->points[i].x);
          triangleData.push_back(l->points[i].y);
        }
      }
      if(x->type == Primitive::SPLINE){
        auto l = std::dynamic_pointer_cast<Spline>(x);
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
      if(x->type == Primitive::TEXT){
        auto l = std::dynamic_pointer_cast<Text>(x);
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

  scene->lineProgram->use();
  scene->lineProgram->setMatrix3fv("matrix",glm::value_ptr(matrix));
  this->lineVAO->bind();
  scene->gl.glDrawArrays(GL_POINTS,0,this->nofLines);
  this->lineVAO->unbind();

  scene->pointProgram->use();
  scene->pointProgram->setMatrix3fv("matrix",glm::value_ptr(matrix));
  this->pointVAO->bind();
  scene->gl.glDrawArrays(GL_POINTS,0,this->nofPoints);
  this->pointVAO->unbind();

  scene->circleProgram->use();
  scene->circleProgram->setMatrix3fv("matrix",glm::value_ptr(matrix));
  this->circleVAO->bind();
  scene->gl.glDrawArrays(GL_POINTS,0,this->nofCircles);
  this->circleVAO->unbind();

  scene->triangleProgram->use();
  scene->triangleProgram->setMatrix3fv("matrix",glm::value_ptr(matrix));
  this->triangleVAO->bind();
  scene->gl.glDrawArrays(GL_TRIANGLES,0,this->nofTriangles*3);
  this->triangleVAO->unbind();

  scene->splineProgram->use();
  scene->splineProgram->setMatrix3fv("matrix",glm::value_ptr(matrix));
  this->splineVAO->bind();
  scene->gl.glPatchParameteri(GL_PATCH_VERTICES,1);
  scene->gl.glDrawArrays(GL_PATCHES,0,this->nofSplines);
  this->splineVAO->unbind();

  scene->textProgram->use();
  scene->textProgram->setMatrix3fv("matrix",glm::value_ptr(matrix));
  this->textVAO->bind();
  scene->fontTexture->bind(0);
  scene->gl.glEnable(GL_BLEND);
  scene->gl.glBlendFunc(GL_SRC_ALPHA,GL_ONE_MINUS_SRC_ALPHA);
  scene->gl.glDrawArrays(GL_POINTS,0,this->nofCharacters);
  scene->gl.glDisable(GL_BLEND);
  this->textVAO->unbind();

  for(auto const&x:this->viewports){
    auto ii = scene->viewports.find(x);
    assert(ii!=scene->viewports.end());
    ii->second->draw(newModelMatrix,projectionMatrix,scene);
  }
}


Draw2D::Draw2D(Context const&gl){
  assert(this!=nullptr);
  this->_impl = new Scene2D(gl);
}

Draw2D::~Draw2D(){
  assert(this!=nullptr);
  delete this->_impl;
}

void Draw2D::draw(){
  assert(this!=nullptr);
  assert(this->_impl!=nullptr);
  auto const&gl=this->_impl->gl;
  auto*s = this->_impl;
  gl.glClearColor(0,0,0,0);
  gl.glClear(GL_DEPTH_BUFFER_BIT|GL_COLOR_BUFFER_BIT);

  if(s->viewports.count(s->rootViewport)!=0){
    auto&v=s->viewports.at(s->rootViewport);
    auto viewProjection = glm::mat3(1.f);
    viewProjection[0].x = 2./(float)v->size.x;
    viewProjection[1].y = 2./(float)v->size.y;
    viewProjection[2] = glm::vec3(-1,-1,1);
    v->draw(glm::mat3(1.f),viewProjection,s);
  }
}

glm::uvec2 Draw2D::getViewportSize()const{
  assert(this!=nullptr);
  assert(this->_impl!=nullptr);
  auto*s = this->_impl;
  if(!this->hasRootViewport()){
    ge::core::printError(GE_CORE_FCENAME,"there is no rootViewport");
    return glm::uvec2(0);
  }
  assert(s->viewports.at(s->rootViewport)!=nullptr);
  return s->viewports.at(s->rootViewport)->size;
}

glm::vec2  Draw2D::getCameraPosition()const{
  assert(this!=nullptr);
  assert(this->_impl!=nullptr);
  auto*s = this->_impl;
  if(!this->hasRootViewport()){
    ge::core::printError(GE_CORE_FCENAME,"there is no rootViewport");
    return glm::vec2(0);
  }
  assert(s->viewports.at(s->rootViewport)!=nullptr);
  return s->viewports.at(s->rootViewport)->cameraPosition;
}

float      Draw2D::getCameraScale()const{
  assert(this!=nullptr);
  assert(this->_impl!=nullptr);
  auto*s = this->_impl;
  if(!this->hasRootViewport()){
    ge::core::printError(GE_CORE_FCENAME,"there is no rootViewport");
    return 1.f;
  }
  assert(s->viewports.at(s->rootViewport)!=nullptr);
  return s->viewports.at(s->rootViewport)->cameraScale;
}

float      Draw2D::getCameraAngle()const{
  assert(this!=nullptr);
  assert(this->_impl!=nullptr);
  auto*s = this->_impl;
  if(!this->hasRootViewport()){
    ge::core::printError(GE_CORE_FCENAME,"there is no rootViewport");
    return 1.f;
  }
  assert(s->viewports.at(s->rootViewport)!=nullptr);
  return s->viewports.at(s->rootViewport)->cameraAngle;
}

void Draw2D::setViewportSize(glm::uvec2 const&size){
  assert(this!=nullptr);
  assert(this->_impl!=nullptr);
  auto*s = this->_impl;
  if(!this->hasRootViewport()){
    ge::core::printError(GE_CORE_FCENAME,"there is no rootViewport");
    return;
  }
  assert(s->viewports.at(s->rootViewport)!=nullptr);
  s->viewports.at(s->rootViewport)->size = size;
}

void Draw2D::setCameraPosition(glm::vec2 const&cameraPosition){
  assert(this!=nullptr);
  assert(this->_impl!=nullptr);
  auto*s = this->_impl;
  if(!this->hasRootViewport()){
    ge::core::printError(GE_CORE_FCENAME,"there is no rootViewport");
    return;
  }
  assert(s->viewports.at(s->rootViewport)!=nullptr);
  s->viewports.at(s->rootViewport)->cameraPosition = cameraPosition;
}

void Draw2D::setCameraScale(float cameraScale){
  assert(this!=nullptr);
  assert(this->_impl!=nullptr);
  auto*s = this->_impl;
  if(!this->hasRootViewport()){
    ge::core::printError(GE_CORE_FCENAME,"there is no rootViewport",cameraScale);
    return;
  }
  assert(s->viewports.at(s->rootViewport)!=nullptr);
  s->viewports.at(s->rootViewport)->cameraScale = cameraScale;
}

void Draw2D::setCameraAngle(float cameraAngle){
  assert(this!=nullptr);
  assert(this->_impl!=nullptr);
  auto*s = this->_impl;
  if(!this->hasRootViewport()){
    ge::core::printError(GE_CORE_FCENAME,"there is no rootViewport",cameraAngle);
    return;
  }
  assert(s->viewports.at(s->rootViewport)!=nullptr);
  s->viewports.at(s->rootViewport)->cameraAngle = cameraAngle;
}

void Draw2D::clear(){
  assert(this!=nullptr);
  assert(this->_impl!=nullptr);
  this->_impl->viewports.clear();
  this->_impl->layers.clear();
  this->_impl->nodes.clear();
  this->_impl->primitives.clear();
  this->_impl->viewportCounter = 0;
  this->_impl->layerCounter = 0;
  this->_impl->nodeCounter = 0;
  this->_impl->primitiveCounter = 0;
  this->_impl->viewportParents.clear();
  this->_impl->layerParents.clear();
  this->_impl->nodeParentNodes.clear();
  this->_impl->nodeParentLayers.clear();
  this->_impl->primitiveParents.clear();
  this->_impl->rootViewport = -1;
}

bool Draw2D::isViewport(size_t viewport)const{
  assert(this!=nullptr);
  assert(this->_impl!=nullptr);
  auto const*s=this->_impl;
  return s->viewports.count(viewport)!=0;
}

bool Draw2D::isLayer(size_t layer)const{
  assert(this!=nullptr);
  assert(this->_impl!=nullptr);
  auto const*s=this->_impl;
  return s->layers.count(layer)!=0;
}

bool Draw2D::isNode(size_t node)const{
  assert(this!=nullptr);
  assert(this->_impl!=nullptr);
  auto const*s=this->_impl;
  return s->nodes.count(node)!=0;
}

bool Draw2D::isPrimitive(size_t primitive)const{
  assert(this!=nullptr);
  assert(this->_impl!=nullptr);
  auto const*s=this->_impl;
  return s->primitives.count(primitive)!=0;
}

size_t Draw2D::getRootViewport()const{
  assert(this!=nullptr);
  assert(this->_impl!=nullptr);
  if(!this->isViewport(this->_impl->rootViewport)){
    ge::core::printError(GE_CORE_FCENAME,"there is no root viewport");
    return 0;
  }
  return this->_impl->rootViewport;
}

void Draw2D::setRootViewport(size_t viewport){
  assert(this!=nullptr);
  assert(this->_impl!=nullptr);
  if(!this->isViewport(viewport)){
    ge::core::printError(GE_CORE_FCENAME,"there is no such viewport",viewport);
    return;
  }
  this->_impl->rootViewport = viewport;
}

bool Draw2D::hasRootViewport()const{
  assert(this!=nullptr);
  assert(this->_impl!=nullptr);
  return this->isViewport(this->_impl->rootViewport);
}

void Draw2D::eraseRootViewport(){
  assert(this!=nullptr);
  assert(this->_impl!=nullptr);
  this->_impl->rootViewport=-1;
}

size_t Draw2D::getNofLayers(size_t viewport)const{
  assert(this!=nullptr);
  assert(this->_impl!=nullptr);
  auto const*s=this->_impl;
  if(!this->isViewport(viewport)){
    ge::core::printError(GE_CORE_FCENAME,"there is no such viewport",viewport);
    return 0;
  }
  auto ii = s->viewports.find(viewport);
  assert(ii->second!=nullptr);
  return ii->second->layers.size();
}

size_t Draw2D::getLayer(size_t viewport,size_t i)const{
  assert(this!=nullptr);
  assert(this->_impl!=nullptr);
  auto const*s=this->_impl;
  if(!this->isViewport(viewport)){
    ge::core::printError(GE_CORE_FCENAME,"there is no such viewport",viewport,i);
    return 0;
  }
  auto ii = s->viewports.find(viewport);
  assert(ii->second!=nullptr);
  if(i>=ii->second->layers.size()){
    ge::core::printError(GE_CORE_FCENAME,"index out of range",viewport,i);
    return 0;
  }
  return ii->second->layers.at(i);
}

size_t Draw2D::getNofNodes(size_t node)const{
  assert(this!=nullptr);
  assert(this->_impl!=nullptr);
  auto const*s=this->_impl;
  if(!this->isNode(node)){
    ge::core::printError(GE_CORE_FCENAME,"there is no such node",node);
    return 0;
  }
  auto ii = s->nodes.find(node);
  assert(ii->second!=nullptr);
  return ii->second->childs.size();
}

size_t Draw2D::getNode(size_t node,size_t i)const{
  assert(this!=nullptr);
  assert(this->_impl!=nullptr);
  auto const*s=this->_impl;
  if(!this->isNode(node)){
    ge::core::printError(GE_CORE_FCENAME,"there is no such node",node,i);
    return 0;
  }
  auto ii = s->nodes.find(node);
  assert(ii->second!=nullptr);
  if(i>=ii->second->childs.size()){
    ge::core::printError(GE_CORE_FCENAME,"index out of range",node,i);
    return 0;
  }
  return ii->second->childs.at(i);
}

size_t Draw2D::getNode(size_t layer)const{
  assert(this!=nullptr);
  assert(this->_impl!=nullptr);
  auto const*s=this->_impl;
  if(!this->hasNode(layer)){
    ge::core::printError(GE_CORE_FCENAME,"that layer does not have node",layer);
    return 0;
  }
  assert(s->layers.find(layer)!=s->layers.end());
  assert(s->layers.find(layer)->second!=nullptr);
  return s->layers.find(layer)->second->root;
}

bool   Draw2D::hasNode(size_t layer)const{
  assert(this!=nullptr);
  assert(this->_impl!=nullptr);
  auto const*s=this->_impl;
  if(!this->isLayer(layer)){
    ge::core::printError(GE_CORE_FCENAME,"there is no such layer",layer);
    return false;
  }
  auto ii = s->layers.find(layer);
  assert(ii->second!=nullptr);
  return this->isNode(ii->second->root);
}

size_t Draw2D::getNofViewports(size_t node)const{
  assert(this!=nullptr);
  assert(this->_impl!=nullptr);
  auto const*s=this->_impl;
  if(!this->isNode(node)){
    ge::core::printError(GE_CORE_FCENAME,"there is no such node",node);
    return 0;
  }
  auto ii = s->nodes.find(node);
  assert(ii->second!=nullptr);
  return ii->second->viewports.size();
}

size_t Draw2D::getViewport(size_t node,size_t i)const{
  assert(this!=nullptr);
  assert(this->_impl!=nullptr);
  auto const*s=this->_impl;
  if(!this->isNode(node)){
    ge::core::printError(GE_CORE_FCENAME,"there is no such node",node,i);
    return 0;
  }
  auto ii = s->nodes.find(node);
  assert(ii->second!=nullptr);
  if(i>=ii->second->viewports.size()){
    ge::core::printError(GE_CORE_FCENAME,"index out of range",node,i);
    return 0;
  }
  return ii->second->viewports.at(i);
}

size_t Draw2D::getNofPrimitives(size_t node)const{
  assert(this!=nullptr);
  assert(this->_impl!=nullptr);
  auto const*s=this->_impl;
  if(!this->isNode(node)){
    ge::core::printError(GE_CORE_FCENAME,"there is no such node",node);
    return 0;
  }
  auto ii = s->nodes.find(node);
  assert(ii->second!=nullptr);
  return ii->second->primitives.size();
}

size_t Draw2D::getPrimitive(size_t node,size_t i)const{
  assert(this!=nullptr);
  assert(this->_impl!=nullptr);
  auto const*s=this->_impl;
  if(!this->isNode(node)){
    ge::core::printError(GE_CORE_FCENAME,"there is no such node",node,i);
    return 0;
  }
  auto ii = s->nodes.find(node);
  assert(ii->second!=nullptr);
  if(i>=ii->second->primitives.size()){
    ge::core::printError(GE_CORE_FCENAME,"index out of range",node,i);
    return 0;
  }
  return ii->second->primitives.at(i);
}

std::shared_ptr<Primitive>Draw2D::getPrimitiveData(size_t primitive)const{
  assert(this!=nullptr);
  assert(this->_impl!=nullptr);
  auto const*s=this->_impl;
  if(!this->isPrimitive(primitive)){
    ge::core::printError(GE_CORE_FCENAME,"there is no such primitive",primitive);
    return nullptr;
  }
  return s->primitives.at(primitive);
}


void Draw2D::primitiveChanged(size_t primitive){
  assert(this!=nullptr);
  assert(this->_impl!=nullptr);
  auto*s=this->_impl;
  if(!this->isPrimitive(primitive)){
    ge::core::printError(GE_CORE_FCENAME,"there is no such primitive",primitive);
    return;
  }
  if(s->primitiveParents.count(primitive)==0)return;
  for(auto const&x:s->primitiveParents.at(primitive))
    s->nodes.at(x)->changed = true;
}

glm::mat3 Draw2D::getNodeMatrix(size_t node)const{
  assert(this!=nullptr);
  assert(this->_impl!=nullptr);
  auto*s=this->_impl;
  if(!this->isNode(node)){
    ge::core::printError(GE_CORE_FCENAME,"there is no such node",node);
    return glm::mat3(1.f);
  }
  assert(s->nodes.at(node)!=nullptr);
  return s->nodes.at(node)->mat;
}

void Draw2D::setNodeMatrix(size_t node,glm::mat3 const&mat){
  assert(this!=nullptr);
  assert(this->_impl!=nullptr);
  auto*s=this->_impl;
  if(!this->isNode(node)){
    ge::core::printError(GE_CORE_FCENAME,"there is no such node",node);
    return;
  }
  assert(s->nodes.at(node)!=nullptr);
  s->nodes.at(node)->mat = mat;
}

glm::uvec2 Draw2D::getViewportSize(size_t viewport)const{
  assert(this!=nullptr);
  assert(this->_impl!=nullptr);
  auto*s=this->_impl;
  if(!this->isViewport(viewport)){
    ge::core::printError(GE_CORE_FCENAME,"there is no such viewport",viewport);
    return glm::uvec2(0);
  }
  assert(s->viewports.at(viewport)!=nullptr);
  return s->viewports.at(viewport)->size;
}

glm::vec2 Draw2D::getViewportCameraPosition(size_t viewport)const{
  assert(this!=nullptr);
  assert(this->_impl!=nullptr);
  auto*s=this->_impl;
  if(!this->isViewport(viewport)){
    ge::core::printError(GE_CORE_FCENAME,"there is no such viewport",viewport);
    return glm::vec2(0.f);
  }
  assert(s->viewports.at(viewport)!=nullptr);
  return s->viewports.at(viewport)->cameraPosition;
}

float Draw2D::getViewportCameraScale(size_t viewport)const{
  assert(this!=nullptr);
  assert(this->_impl!=nullptr);
  auto*s=this->_impl;
  if(!this->isViewport(viewport)){
    ge::core::printError(GE_CORE_FCENAME,"there is no such viewport",viewport);
    return 1.f;
  }
  assert(s->viewports.at(viewport)!=nullptr);
  return s->viewports.at(viewport)->cameraScale;
}

float Draw2D::getViewportCameraAngle(size_t viewport)const{
  assert(this!=nullptr);
  assert(this->_impl!=nullptr);
  auto*s=this->_impl;
  if(!this->isViewport(viewport)){
    ge::core::printError(GE_CORE_FCENAME,"there is no such viewport",viewport);
    return 0.f;
  }
  assert(s->viewports.at(viewport)!=nullptr);
  return s->viewports.at(viewport)->cameraAngle;
}

void Draw2D::setViewportSize(size_t viewport,glm::uvec2 const&size){
  assert(this!=nullptr);
  assert(this->_impl!=nullptr);
  auto*s=this->_impl;
  if(!this->isViewport(viewport)){
    ge::core::printError(GE_CORE_FCENAME,"there is no such viewport",viewport);
    return;
  }
  assert(s->viewports.at(viewport)!=nullptr);
  s->viewports.at(viewport)->size = size;
}

void Draw2D::setViewportCameraPosition(size_t viewport,glm::vec2 const&cameraPosition){
  assert(this!=nullptr);
  assert(this->_impl!=nullptr);
  auto*s=this->_impl;
  if(!this->isViewport(viewport)){
    ge::core::printError(GE_CORE_FCENAME,"there is no such viewport",viewport);
    return;
  }
  assert(s->viewports.at(viewport)!=nullptr);
  s->viewports.at(viewport)->cameraPosition = cameraPosition;
}

void Draw2D::setViewportCameraScale(size_t viewport,float cameraScale){
  assert(this!=nullptr);
  assert(this->_impl!=nullptr);
  auto*s=this->_impl;
  if(!this->isViewport(viewport)){
    ge::core::printError(GE_CORE_FCENAME,"there is no such viewport",viewport);
    return;
  }
  assert(s->viewports.at(viewport)!=nullptr);
  s->viewports.at(viewport)->cameraScale = cameraScale;
}

void Draw2D::setViewportCameraAngle(size_t viewport,float cameraAngle){
  assert(this!=nullptr);
  assert(this->_impl!=nullptr);
  auto*s=this->_impl;
  if(!this->isViewport(viewport)){
    ge::core::printError(GE_CORE_FCENAME,"there is no such viewport",viewport);
    return;
  }
  assert(s->viewports.at(viewport)!=nullptr);
  s->viewports.at(viewport)->cameraAngle = cameraAngle;
}



size_t Draw2D::createLayer(){
  assert(this!=nullptr);
  assert(this->_impl!=nullptr);
  auto*s=this->_impl;
  while(this->isLayer(s->layerCounter))s->layerCounter++;
  s->layers[s->layerCounter] = std::make_shared<Layer>();
  return s->layerCounter++;
}

size_t Draw2D::createViewport(glm::uvec2 const&size,glm::vec2 const&cameraPosition,float cameraScale,float cameraAngle){
  assert(this!=nullptr);
  assert(this->_impl!=nullptr);
  auto*s=this->_impl;
  while(this->isViewport(s->viewportCounter))s->viewportCounter++;
  s->viewports[s->viewportCounter] = std::make_shared<Viewport>(size,cameraPosition,cameraScale,cameraAngle);
  return s->viewportCounter++;
}

size_t Draw2D::createNode(glm::mat3 const&mat){
  assert(this!=nullptr);
  assert(this->_impl!=nullptr);
  auto*s=this->_impl;
  while(this->isNode(s->nodeCounter))s->nodeCounter++;
  s->nodes[s->nodeCounter] = std::make_shared<Node>(mat);
  return s->nodeCounter++;
}

size_t Draw2D::createPrimitive(std::shared_ptr<Primitive>const&primitive){
  assert(this!=nullptr);
  assert(this->_impl!=nullptr);
  auto*s=this->_impl;
  while(this->isPrimitive(s->primitiveCounter))s->primitiveCounter++;
  s->primitives[s->primitiveCounter] = primitive;
  return s->primitiveCounter++;
}

void Draw2D::insertLayer(size_t viewport,size_t layer){
  assert(this!=nullptr);
  assert(this->_impl!=nullptr);
  auto*s=this->_impl;
  if(!this->isViewport(viewport)){
    ge::core::printError(GE_CORE_FCENAME,"there is no such viewport",viewport,layer);
    return;
  }
  if(!this->isLayer(layer)){
    ge::core::printError(GE_CORE_FCENAME,"there is no such layer",viewport,layer);
    return;
  }
  assert(s->viewports.at(viewport)!=nullptr);
  auto&layers=s->viewports.at(viewport)->layers;
  if(std::find(layers.begin(),layers.end(),layer)!=layers.end())return;
  layers.push_back(layer);

  s->addParent(s->layerParents,layer,viewport);
}

void Draw2D::insertViewport(size_t node,size_t viewport){
  assert(this!=nullptr);
  assert(this->_impl!=nullptr);
  auto*s=this->_impl;
  if(!this->isNode(node)){
    ge::core::printError(GE_CORE_FCENAME,"there is no such node",viewport,node);
    return;
  }
  if(!this->isViewport(viewport)){
    ge::core::printError(GE_CORE_FCENAME,"there is no such viewport",viewport,node);
    return;
  }

  assert(s->nodes.at(node)!=nullptr);
  auto&viewports=s->nodes.at(node)->viewports;
  if(std::find(viewports.begin(),viewports.end(),viewport)!=viewports.end())return;
  viewports.push_back(viewport);

  s->addParent(s->viewportParents,viewport,node);
}

void Draw2D::insertNode(size_t toNode,size_t node){
  assert(this!=nullptr);
  assert(this->_impl!=nullptr);
  auto*s=this->_impl;
  if(!this->isNode(toNode)){
    ge::core::printError(GE_CORE_FCENAME,"there is no such toNode",toNode,node);
    return;
  }
  if(!this->isNode(node)){
    ge::core::printError(GE_CORE_FCENAME,"there is no such node",toNode,node);
    return;
  }

  assert(s->nodes.at(toNode)!=nullptr);
  auto&nodes=s->nodes.at(toNode)->childs;
  if(std::find(nodes.begin(),nodes.end(),node)!=nodes.end())return;
  nodes.push_back(node);

  s->addParent(s->nodeParentNodes,node,toNode);
}

void Draw2D::setLayerNode(size_t layer,size_t node){
  assert(this!=nullptr);
  assert(this->_impl!=nullptr);
  auto*s=this->_impl;
  if(!this->isLayer(layer)){
    ge::core::printError(GE_CORE_FCENAME,"there is no such layer",layer,node);
    return;
  }
  if(!this->isNode(node)){
    ge::core::printError(GE_CORE_FCENAME,"there is no such node",layer,node);
    return;
  }
  assert(s->layers.at(layer)!=nullptr);
  s->layers.at(layer)->root = node;

  s->addParent(s->nodeParentLayers,node,layer);
}

void Draw2D::insertPrimitive(size_t node,size_t primitive){
  assert(this!=nullptr);
  assert(this->_impl!=nullptr);
  auto*s=this->_impl;
  if(!this->isNode(node)){
    ge::core::printError(GE_CORE_FCENAME,"there is no such node",node,primitive);
    return;
  }
  if(!this->isPrimitive(primitive)){
    ge::core::printError(GE_CORE_FCENAME,"there is no such primitive",node,primitive);
    return;
  }

  assert(s->nodes.at(node)!=nullptr);
  auto&primitives=s->nodes.at(node)->primitives;
  if(std::find(primitives.begin(),primitives.end(),primitive)!=primitives.end())return;
  primitives.push_back(primitive);

  s->addParent(s->primitiveParents,primitive,node);
  s->nodes.at(node)->changed = true;
}

void Draw2D::eraseLayer(size_t viewport,size_t layer){
  assert(this!=nullptr);
  assert(this->_impl!=nullptr);
  auto*s=this->_impl;
  if(!this->isViewport(viewport)){
    ge::core::printError(GE_CORE_FCENAME,"there is no such viewport",viewport,layer);
    return;
  }
  if(!this->isLayer(layer)){
    ge::core::printError(GE_CORE_FCENAME,"there is no such layer",viewport,layer);
    return;
  }
  assert(s->viewports.at(viewport)!=nullptr);
  auto&layers=s->viewports.at(viewport)->layers;
  layers.erase(std::remove(layers.begin(),layers.end(),layer),layers.end());

  s->removeParent(s->layerParents,layer,viewport);
}

void Draw2D::eraseViewport(size_t node,size_t viewport){
  assert(this!=nullptr);
  assert(this->_impl!=nullptr);
  auto*s=this->_impl;
  if(!this->isNode(node)){
    ge::core::printError(GE_CORE_FCENAME,"there is no such node",viewport,node);
    return;
  }
  if(!this->isViewport(viewport)){
    ge::core::printError(GE_CORE_FCENAME,"there is no such viewport",viewport,node);
    return;
  }

  assert(s->nodes.at(node)!=nullptr);
  auto&viewports=s->nodes.at(node)->viewports;
  viewports.erase(std::remove(viewports.begin(),viewports.end(),viewport),viewports.end());

  s->removeParent(s->viewportParents,viewport,node);
}

void Draw2D::eraseNode(size_t fromNode,size_t node){
  assert(this!=nullptr);
  assert(this->_impl!=nullptr);
  auto*s=this->_impl;
  if(!this->isNode(fromNode)){
    ge::core::printError(GE_CORE_FCENAME,"there is no such fromNode",fromNode,node);
    return;
  }
  if(!this->isNode(node)){
    ge::core::printError(GE_CORE_FCENAME,"there is no such node",fromNode,node);
    return;
  }

  assert(s->nodes.at(fromNode)!=nullptr);
  auto&nodes=s->nodes.at(fromNode)->childs;
  nodes.erase(std::remove(nodes.begin(),nodes.end(),node),nodes.end());

  s->removeParent(s->nodeParentNodes,node,fromNode);
}

void Draw2D::eraseNode(size_t layer){
  assert(this!=nullptr);
  assert(this->_impl!=nullptr);
  auto*s=this->_impl;
  if(!this->isLayer(layer)){
    ge::core::printError(GE_CORE_FCENAME,"there is no such layer",layer);
    return;
  }
  assert(s->layers.at(layer)!=nullptr);
  auto&node = s->layers.at(layer)->root;
  s->removeParent(s->nodeParentLayers,node,layer);
  node = -1;
}

void Draw2D::erasePrimitive(size_t node,size_t primitive){
  assert(this!=nullptr);
  assert(this->_impl!=nullptr);
  auto*s=this->_impl;
  if(!this->isNode(node)){
    ge::core::printError(GE_CORE_FCENAME,"there is no such node",node,primitive);
    return;
  }
  if(!this->isPrimitive(primitive)){
    ge::core::printError(GE_CORE_FCENAME,"there is no such primitive",node,primitive);
    return;
  }

  assert(s->nodes.at(node)!=nullptr);
  auto&primitives=s->nodes.at(node)->primitives;
  primitives.erase(std::remove(primitives.begin(),primitives.end(),primitive),primitives.end());

  s->removeParent(s->primitiveParents,primitive,node);
  s->nodes.at(node)->changed = true;
}

void Draw2D::deleteLayer(size_t layer){
  assert(this!=nullptr);
  assert(this->_impl!=nullptr);
  auto*s=this->_impl;
  if(!this->isLayer(layer))return;
  if(s->layerParents.count(layer)!=0)
    for(auto const&x:s->layerParents.at(layer))
      this->eraseLayer(x,layer);
  if(this->hasNode(layer))
    s->removeParent(s->nodeParentLayers,this->getNode(layer),layer);
  s->layerParents.erase(layer);
  s->layers.erase(layer);
}

void Draw2D::deleteViewport(size_t viewport){
  assert(this!=nullptr);
  assert(this->_impl!=nullptr);
  auto*s=this->_impl;
  if(!this->isViewport(viewport))return;
  if(s->viewportParents.count(viewport)!=0)
    for(auto const&x:s->layerParents.at(viewport))
      this->eraseViewport(x,viewport);
  for(auto const&x:s->viewports.at(viewport)->layers)
    s->removeParent(s->layerParents,x,viewport);
  s->viewportParents.erase(viewport);
  s->viewports.erase(viewport);
}

void Draw2D::deleteNode(size_t node){
  assert(this!=nullptr);
  assert(this->_impl!=nullptr);
  auto*s=this->_impl;
  if(!this->isNode(node))return;
  if(s->nodeParentNodes.count(node)!=0)
    for(auto const&x:s->nodeParentNodes.at(node))
      this->eraseNode(x,node);
  if(s->nodeParentLayers.count(node)!=0)
    for(auto const&x:s->nodeParentLayers.at(node))
      this->eraseNode(x);
  for(auto const&x:s->nodes.at(node)->childs)
    s->removeParent(s->nodeParentNodes,x,node);
  for(auto const&x:s->nodes.at(node)->viewports)
    s->removeParent(s->viewportParents,x,node);
  for(auto const&x:s->nodes.at(node)->primitives)
    s->removeParent(s->primitiveParents,x,node);

  s->nodeParentNodes.erase(node);
  s->nodeParentLayers.erase(node);
  s->nodes.erase(node);
}

void Draw2D::deletePrimitive(size_t primitive){
  assert(this!=nullptr);
  assert(this->_impl!=nullptr);
  auto*s=this->_impl;
  if(!this->isPrimitive(primitive))return;
  if(s->primitiveParents.count(primitive)!=0)
    for(auto const&x:s->primitiveParents.at(primitive))
      this->erasePrimitive(x,primitive);

  s->primitiveParents.erase(primitive);
  s->primitives.erase(primitive);
}

