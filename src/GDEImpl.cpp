#include<GDEImpl.h>

//#include<Draw2D.h>
#include<Draw2DShaders.h>
#include<CreateFontTexture.h>

using namespace ge::gl;

Edit::Edit(ge::gl::Context const&g,glm::uvec2 const&size):gl(g){
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


  this->stencilProgram = std::make_shared<Program>();
  this->stencilProgram->link(
      {std::make_shared<Shader>(GL_VERTEX_SHADER,stencilVS)});
  this->stencilVAO = std::make_shared<VertexArray>();

  this->rootViewport = new Viewport2d(size);
  this->currentViewport = this->rootViewport;

  this->rootViewport->push_back(new Layer(new Node2d()));//edit layer
  this->rootViewport->push_back(new Layer(new Node2d()));//menu layer
  this->rootViewport->at(0)->root->addValue<Viewport2d>(size);
  this->editViewport = this->rootViewport->at(0)->root->getValue<Viewport2d>(0);
  this->editViewport->push_back(new Layer(new Node2d()));//connection layer
  this->editViewport->push_back(new Layer(new Node2d()));//function layer
  this->connectionsNode = (Node2d*)this->editViewport->at(0)->root;
  this->functionsNode = (Node2d*)this->editViewport->at(1)->root;
  this->menuNode = (Node2d*)this->rootViewport->at(1)->root;
}

Edit::~Edit(){
  delete this->rootViewport;
}

void Edit::draw(){

  gl.glClearColor(0,0,0,1);
  gl.glClear(GL_DEPTH_BUFFER_BIT|GL_COLOR_BUFFER_BIT|GL_STENCIL_BUFFER_BIT);
  gl.glEnable(GL_STENCIL_TEST);
  gl.glEnable(GL_BLEND);
  gl.glDepthFunc(GL_LEQUAL);
  gl.glDisable(GL_DEPTH_TEST);
  gl.glBlendFunc(GL_SRC_ALPHA,GL_ONE_MINUS_SRC_ALPHA);

  auto v = this->currentViewport;
  if(v==nullptr)return;

  auto viewProjection = glm::mat3(1.f);
  viewProjection[0].x = 2./(float)v->cameraSize.x;
  viewProjection[1].y = 2./(float)v->cameraSize.y;
  viewProjection[2] = glm::vec3(-1,-1,1);

  this->drawViewport(v,glm::mat3(1.f),viewProjection);

  gl.glDisable(GL_BLEND);
  gl.glDisable(GL_STENCIL_TEST);
}

void Edit::drawViewport(Viewport2d*viewport,glm::mat3 const&model,glm::mat3 const&projection){
  glm::vec3 wp[4];
  wp[0]=model*glm::vec3(0.f,0.f,1.f);
  wp[1]=model*glm::vec3(glm::vec2(viewport->cameraSize.x,0.f   ),1.f);
  wp[2]=model*glm::vec3(glm::vec2(0.f   ,viewport->cameraSize.y),1.f);
  wp[3]=model*glm::vec3(glm::vec2(viewport->cameraSize.x,viewport->cameraSize.y),1.f);
  glm::vec2 op=glm::vec2(
      glm::min(glm::min(wp[0].x,wp[1].x),glm::min(wp[2].x,wp[3].x)),
      glm::min(glm::min(wp[0].y,wp[1].y),glm::min(wp[2].y,wp[3].y)));
  glm::vec2 os=glm::vec2(
      glm::max(glm::max(wp[0].x,wp[1].x),glm::max(wp[2].x,wp[3].x)),
      glm::max(glm::max(wp[0].y,wp[1].y),glm::max(wp[2].y,wp[3].y)))-op;
  gl.glViewport(op.x,op.y,os.x,os.y);
  auto viewTranslate = Edit::translate(-viewport->cameraPosition);
  auto viewRotation = Edit::rotate(viewport->cameraAngle);
  auto viewScale = Edit::scale(viewport->cameraScale);
  glm::mat3 viewMatrix = viewRotation*viewTranslate*viewScale;

  for(auto const&x:*viewport){
    //gl.glDisable(GL_DEPTH_TEST);
    gl.glStencilFunc(GL_ALWAYS,0,0);
    gl.glStencilOp(GL_KEEP,GL_KEEP,GL_ZERO);
    gl.glColorMask(GL_FALSE,GL_FALSE,GL_FALSE,GL_FALSE);
    this->stencilVAO->bind();
    this->stencilProgram->use();
    this->stencilProgram->set2f("a",-1,-1);
    this->stencilProgram->set2f("b",+1,-1);
    this->stencilProgram->set2f("c",-1,+1);
    this->stencilProgram->set2f("d",+1,+1);
    gl.glDrawArrays(GL_TRIANGLE_STRIP,0,4);
    gl.glStencilOp(GL_KEEP,GL_KEEP,GL_INCR);
    this->stencilProgram->set2f("a",-1+2*wp[0].x/os.x,-1+2*wp[0].y/os.y);
    this->stencilProgram->set2f("b",-1+2*wp[1].x/os.x,-1+2*wp[1].y/os.y);
    this->stencilProgram->set2f("c",-1+2*wp[2].x/os.x,-1+2*wp[2].y/os.y);
    this->stencilProgram->set2f("d",-1+2*wp[3].x/os.x,-1+2*wp[3].y/os.y);
    gl.glDrawArrays(GL_TRIANGLE_STRIP,0,4);
    this->stencilVAO->unbind();
    gl.glStencilOp(GL_KEEP,GL_KEEP,GL_KEEP);
    gl.glStencilFunc(GL_EQUAL,1,0xff);
    //gl.glEnable(GL_DEPTH_TEST);
    gl.glColorMask(GL_TRUE,GL_TRUE,GL_TRUE,GL_TRUE);
    this->drawLayer(x,viewMatrix*model,projection);
  }
}

void Edit::drawLayer(Layer*layer,glm::mat3 const&model,glm::mat3 const&projection){
  assert(layer!=nullptr);
  if(!layer->root)return;
  this->drawNode((Node2d*)layer->root,model,projection);
}

class RenderData{
  public:
    std::shared_ptr<Buffer>lineBuffer;
    std::shared_ptr<VertexArray>lineVAO;
    size_t nofLines = 0;
    std::shared_ptr<Buffer>pointBuffer;
    std::shared_ptr<VertexArray>pointVAO;
    size_t nofPoints = 0;
    std::shared_ptr<Buffer>circleBuffer;
    std::shared_ptr<VertexArray>circleVAO;
    size_t nofCircles = 0;
    std::shared_ptr<Buffer>triangleBuffer;
    std::shared_ptr<VertexArray>triangleVAO;
    size_t nofTriangles = 0;
    std::shared_ptr<Buffer>splineBuffer;
    std::shared_ptr<VertexArray>splineVAO;
    size_t nofSplines = 0;
    std::shared_ptr<Buffer>textBuffer;
    std::shared_ptr<VertexArray>textVAO;
    size_t nofCharacters = 0;
    bool changed = true;
};

void Edit::drawNode(Node2d*node,glm::mat3 const&model,glm::mat3 const&projection){
  assert(node!=nullptr);
  glm::mat3 newModelMatrix = node->mat*model;
  glm::mat3 matrix = projection*newModelMatrix;
  if(!node->hasValues<RenderData>()){
    node->addValue<RenderData>();
  }
  auto rd = node->getValue<RenderData>(0);

  if(rd->changed){
    std::vector<float>lineData;
    std::vector<float>pointData;
    std::vector<float>circleData;
    std::vector<float>triangleData;
    std::vector<float>splineData;
    std::vector<float>textData;
    if(node->hasValues<Line>()){
      auto linePtrs = node->getValues<Line>();
      for(auto const&x:linePtrs){
        auto l = (Line*)x;
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
    if(node->hasValues<Point>()){
      auto pointPtrs = node->getValues<Point>();
      for(auto const&x:pointPtrs){
        auto l = (Point*)x;
        pointData.push_back(l->color[0]);
        pointData.push_back(l->color[1]);
        pointData.push_back(l->color[2]);
        pointData.push_back(l->color[3]);
        pointData.push_back(l->point.x);
        pointData.push_back(l->point.y);
        pointData.push_back(l->size);
      }
    }
    if(node->hasValues<Circle>()){
      auto pointPtrs = node->getValues<Circle>();
      for(auto const&x:pointPtrs){
        auto l = (Circle*)x;
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
    if(node->hasValues<Triangle>()){
      auto pointPtrs = node->getValues<Triangle>();
      for(auto const&x:pointPtrs){
        auto l = (Triangle*)x;
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
    if(node->hasValues<Spline>()){
      auto pointPtrs = node->getValues<Spline>();
      for(auto const&x:pointPtrs){
        auto l = (Spline*)x;
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
    if(node->hasValues<Text>()){
      auto pointPtrs = node->getValues<Text>();
      for(auto const&x:pointPtrs){
        auto l = (Text*)x;
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
    rd->lineBuffer = std::make_shared<Buffer>(lineData.size()*sizeof(float),lineData.data());
    rd->nofLines = lineData.size()/9;
    rd->lineVAO = std::make_shared<VertexArray>();
    rd->lineVAO->addAttrib(rd->lineBuffer,
        0,4,GL_FLOAT,sizeof(float)*9,sizeof(float)*0);
    rd->lineVAO->addAttrib(rd->lineBuffer,
        1,2,GL_FLOAT,sizeof(float)*9,sizeof(float)*4);
    rd->lineVAO->addAttrib(rd->lineBuffer,
        2,2,GL_FLOAT,sizeof(float)*9,sizeof(float)*6);
    rd->lineVAO->addAttrib(rd->lineBuffer,
        3,1,GL_FLOAT,sizeof(float)*9,sizeof(float)*8);

    rd->pointBuffer = std::make_shared<Buffer>(pointData.size()*sizeof(float),pointData.data());
    rd->nofPoints = pointData.size()/7;
    rd->pointVAO = std::make_shared<VertexArray>();
    rd->pointVAO->addAttrib(rd->pointBuffer,
        0,4,GL_FLOAT,sizeof(float)*7,sizeof(float)*0);
    rd->pointVAO->addAttrib(rd->pointBuffer,
        1,2,GL_FLOAT,sizeof(float)*7,sizeof(float)*4);
    rd->pointVAO->addAttrib(rd->pointBuffer,
        2,1,GL_FLOAT,sizeof(float)*7,sizeof(float)*6);

    rd->circleBuffer = std::make_shared<Buffer>(circleData.size()*sizeof(float),circleData.data());
    rd->nofCircles = circleData.size()/8;
    rd->circleVAO = std::make_shared<VertexArray>();
    rd->circleVAO->addAttrib(rd->circleBuffer,
        0,4,GL_FLOAT,sizeof(float)*8,sizeof(float)*0);
    rd->circleVAO->addAttrib(rd->circleBuffer,
        1,2,GL_FLOAT,sizeof(float)*8,sizeof(float)*4);
    rd->circleVAO->addAttrib(rd->circleBuffer,
        2,1,GL_FLOAT,sizeof(float)*8,sizeof(float)*6);
    rd->circleVAO->addAttrib(rd->circleBuffer,
        3,1,GL_FLOAT,sizeof(float)*8,sizeof(float)*7);

    rd->triangleBuffer = std::make_shared<Buffer>(triangleData.size()*sizeof(float),triangleData.data());
    rd->nofTriangles = triangleData.size()/6/3;
    rd->triangleVAO = std::make_shared<VertexArray>();
    rd->triangleVAO->addAttrib(rd->triangleBuffer,
        0,4,GL_FLOAT,sizeof(float)*6,sizeof(float)*0);
    rd->triangleVAO->addAttrib(rd->triangleBuffer,
        1,2,GL_FLOAT,sizeof(float)*6,sizeof(float)*4);

    rd->splineBuffer = std::make_shared<Buffer>(splineData.size()*sizeof(float),splineData.data());
    rd->nofSplines = splineData.size()/13;
    rd->splineVAO = std::make_shared<VertexArray>();
    rd->splineVAO->addAttrib(rd->splineBuffer,
        0,4,GL_FLOAT,sizeof(float)*13,sizeof(float)*0 );
    rd->splineVAO->addAttrib(rd->splineBuffer,
        1,2,GL_FLOAT,sizeof(float)*13,sizeof(float)*4 );
    rd->splineVAO->addAttrib(rd->splineBuffer,
        2,2,GL_FLOAT,sizeof(float)*13,sizeof(float)*6 );
    rd->splineVAO->addAttrib(rd->splineBuffer,
        3,2,GL_FLOAT,sizeof(float)*13,sizeof(float)*8 );
    rd->splineVAO->addAttrib(rd->splineBuffer,
        4,2,GL_FLOAT,sizeof(float)*13,sizeof(float)*10);
    rd->splineVAO->addAttrib(rd->splineBuffer,
        5,1,GL_FLOAT,sizeof(float)*13,sizeof(float)*12);

    rd->textBuffer = std::make_shared<Buffer>(textData.size()*sizeof(float),textData.data());
    rd->nofCharacters = textData.size()/11;
    rd->textVAO = std::make_shared<VertexArray>();
    rd->textVAO->addAttrib(rd->textBuffer,
        0,4,GL_FLOAT,sizeof(float)*11,sizeof(float)*0);
    rd->textVAO->addAttrib(rd->textBuffer,
        1,2,GL_FLOAT,sizeof(float)*11,sizeof(float)*4);
    rd->textVAO->addAttrib(rd->textBuffer,
        2,2,GL_FLOAT,sizeof(float)*11,sizeof(float)*6);
    rd->textVAO->addAttrib(rd->textBuffer,
        3,1,GL_FLOAT,sizeof(float)*11,sizeof(float)*8);
    rd->textVAO->addAttrib(rd->textBuffer,
        4,1,GL_FLOAT,sizeof(float)*11,sizeof(float)*9);
    rd->textVAO->addAttrib(rd->textBuffer,
        5,1,GL_FLOAT,sizeof(float)*11,sizeof(float)*10);

    rd->changed = false;
  }
  this->lineProgram->use();
  this->lineProgram->setMatrix3fv("matrix",glm::value_ptr(matrix));
  rd->lineVAO->bind();
  this->gl.glDrawArrays(GL_POINTS,0,rd->nofLines);
  rd->lineVAO->unbind();

  this->pointProgram->use();
  this->pointProgram->setMatrix3fv("matrix",glm::value_ptr(matrix));
  rd->pointVAO->bind();
  this->gl.glDrawArrays(GL_POINTS,0,rd->nofPoints);
  rd->pointVAO->unbind();

  
  this->circleProgram->use();
  this->circleProgram->setMatrix3fv("matrix",glm::value_ptr(matrix));
  rd->circleVAO->bind();
  this->gl.glDrawArrays(GL_POINTS,0,rd->nofCircles);
  rd->circleVAO->unbind();
  
  this->triangleProgram->use();
  this->triangleProgram->setMatrix3fv("matrix",glm::value_ptr(matrix));
  rd->triangleVAO->bind();
  this->gl.glDrawArrays(GL_TRIANGLES,0,rd->nofTriangles*3);
  rd->triangleVAO->unbind();

  this->splineProgram->use();
  this->splineProgram->setMatrix3fv("matrix",glm::value_ptr(matrix));
  rd->splineVAO->bind();
  this->gl.glPatchParameteri(GL_PATCH_VERTICES,1);
  this->gl.glDrawArrays(GL_PATCHES,0,rd->nofSplines);
  rd->splineVAO->unbind();

  this->textProgram->use();
  this->textProgram->setMatrix3fv("matrix",glm::value_ptr(matrix));
  rd->textVAO->bind();
  this->fontTexture->bind(0);
  this->gl.glDrawArrays(GL_POINTS,0,rd->nofCharacters);
  rd->textVAO->unbind();

  if(node->hasValues<Viewport2d>()){
    auto viewportPtrs = node->getValues<Viewport2d>();
    for(auto const&x:viewportPtrs)
      this->drawViewport((Viewport2d*)x,newModelMatrix,projection);
  }

  for(auto x:*node){
    this->drawNode((Node2d*)x,newModelMatrix,projection);
  }
 
}

void Edit::mouseMotion(int32_t xrel,int32_t yrel,size_t x,size_t y){
  if(!this->currentViewport)return;
  this->mouseMotionViewport(
      this->rootViewport,
      glm::vec2(xrel,yrel),
      glm::vec2(x,y));
}

bool Edit::mouseMotionViewport(Viewport2d*viewport,glm::vec2 const&diff,glm::vec2 const&pos){
  if(!viewport)return false;
  auto viewTranslate = Edit::translate(-viewport->cameraPosition);
  auto viewRotation = Edit::rotate(viewport->cameraAngle);
  auto viewScale = Edit::scale(viewport->cameraScale);
  auto matrix = glm::inverse(viewRotation*viewTranslate*viewScale);
  glm::vec2 newDiff = glm::vec2(matrix*glm::vec3(diff,0.f));
  glm::vec2 newPos  = glm::vec2(matrix*glm::vec3(pos,1.f));
  if(newPos.x<0.f||newPos.y<0.f||newPos.x>viewport->cameraSize.x||newPos.y>viewport->cameraSize.y)return false;
  for(int32_t i=viewport->size()-1;i>=0;--i)
    if(this->mouseMotionLayer(viewport->at(i),newDiff,newPos))return true;
  return false;
}

bool Edit::mouseMotionLayer(Layer*layer,glm::vec2 const&diff,glm::vec2 const&pos){
  if(!layer)return false;
  if(!layer->root)return false;
  return this->mouseMotionNode((Node2d*)layer->root,diff,pos);
}

bool Edit::mouseMotionNode(Node2d*node,glm::vec2 const&diff,glm::vec2 const&pos){
  auto matrix = glm::inverse(node->mat);
  auto newPos = glm::vec2(matrix*glm::vec3(pos,1.f));
  auto newDiff = glm::vec2(matrix*glm::vec3(diff,0.f));
  if(node->hasValues<MouseMotionEvent>()){
    auto v = node->getValues<MouseMotionEvent>();
    for(auto const&x:v){
      auto vv = (MouseMotionEvent*)x;
      if(newPos.x<vv->pos.x||newPos.y<vv->pos.y||newPos.x>vv->pos.x+vv->size.x||newPos.y>vv->pos.y+vv->size.y)continue;
      (*vv)(node,newDiff,newPos);
      return true;
    }
  }
  if(node->hasValues<Viewport2d>()){
    auto v = node->getValues<Viewport2d>();
    for(auto const&x:v){
      auto vv = (Viewport2d*)x;
      if(this->mouseMotionViewport(vv,newDiff,newPos))return true;
    }
  }
  for(auto const&x:*node)
    if(this->mouseMotionNode((Node2d*)x,newDiff,newPos))return true;
  return false;
}

