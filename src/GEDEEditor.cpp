#include<GEDEEditor.h>
#include<algorithm>

#include<UI.h>
using namespace gde;

struct AddToNodeData{
  std::shared_ptr<Draw2D>draw2D;
  size_t node;
};

const size_t DATA_PRIMITIVE = 0;

namespace ui{
  template<>inline size_t getTypeId<Line>(){return DATA_PRIMITIVE;}
  template<>inline size_t getTypeId<Point>(){return DATA_PRIMITIVE;}
  template<>inline size_t getTypeId<Circle>(){return DATA_PRIMITIVE;}
  template<>inline size_t getTypeId<Triangle>(){return DATA_PRIMITIVE;}
  template<>inline size_t getTypeId<Text>(){return DATA_PRIMITIVE;}
  template<>inline size_t getTypeId<Spline>(){return DATA_PRIMITIVE;}
}

void addToNode(ui::Element*elm,void*d){
  auto data = (AddToNodeData*)d;
  glm::vec2 p = elm->getPosition();
  glm::vec2 s = elm->getSize();
  for(auto const&dd:elm->data){
    if(dd->getId()!=DATA_PRIMITIVE)continue;
    auto x = (Primitive*)dd->getData();
    size_t primId = -1;
    switch(x->type){
      case Primitive::LINE:
        primId = data->draw2D->createPrimitive(std::make_shared<Line>(
              ((Line*)x)->points[0]*s+p,
              ((Line*)x)->points[1]*s+p,
              ((Line*)x)->width,
              x->color));
        break;
      case Primitive::POINT:
        primId = data->draw2D->createPrimitive(std::make_shared<Point>(
              ((Point*)x)->point*s+p,
              ((Point*)x)->size,
              x->color));
        break;
      case Primitive::CIRCLE:
        primId = data->draw2D->createPrimitive(std::make_shared<Circle>(
              ((Circle*)x)->point*s+p,
              ((Circle*)x)->size,
              ((Circle*)x)->width,
              x->color));
        break;
      case Primitive::TRIANGLE:
        primId = data->draw2D->createPrimitive(std::make_shared<Triangle>(
              ((Triangle*)x)->points[0]*s+p,
              ((Triangle*)x)->points[1]*s+p,
              ((Triangle*)x)->points[2]*s+p,
              x->color));
        break;
      case Primitive::TEXT:
        primId = data->draw2D->createPrimitive(std::make_shared<Text>(
              ((Text*)x)->data,
              ((Text*)x)->size,
              ((Text*)x)->position*s+p,
              ((Text*)x)->direction,
              x->color));
        break;
      case Primitive::SPLINE:
        primId = data->draw2D->createPrimitive(std::make_shared<Spline>(
              ((Spline*)x)->points[0]*s+p,
              ((Spline*)x)->points[1]*s+p,
              ((Spline*)x)->points[2]*s+p,
              ((Spline*)x)->points[3]*s+p,
              ((Spline*)x)->width,
              x->color));
        break;
    }
    data->draw2D->insertPrimitive(data->node,primId);
  }
}


void Function::create(){
  glm::vec4 backgroundColor = glm::vec4(0,0,0,.8);
  glm::vec4 captionBackgrounColor = glm::vec4(0.0,0.1,0.0,1);
  glm::vec4 captionColor = glm::vec4(0,1,0,1);
  glm::vec4 lineColor = glm::vec4(0,1,0,1);
  glm::vec4 textColor = glm::vec4(0,1,0,1);
  size_t captionFontSize = 10;
  size_t fontSize = 8;
  size_t margin = 2;
  size_t captionMargin = 2;
  size_t textIndent = 2;
  //size_t inputSpacing = 2;
  size_t inputOutputDistance = 10;
  size_t inputRadius = 4;
  size_t outputRadius = 4;
  size_t lineWidth = 1;

  this->node = this->draw2D->createNode();
  using namespace ui;

  auto root = new Split(1,{
      new Rectangle(0,lineWidth,{newData<Line>(0,.5,1,.5,lineWidth,lineColor)}),//top line
      new Split(0,{
        new Rectangle(lineWidth,0,{newData<Line>(.5,0,.5,1,lineWidth,lineColor)}),//left line
        new Split(1,{
          new Split(1,{
            new Rectangle(0,captionMargin),
            new Split(0,{
              new Rectangle(captionMargin,0),
              new Rectangle(captionFontSize*this->functionName.length(),captionFontSize*2,{newData<Text>(this->functionName,captionFontSize,captionColor)}),
              new Rectangle(captionMargin,0),
              }),
            new Rectangle(0,captionMargin),
            },{newData<Triangle>(0,0,1,0,1,1,captionBackgrounColor),newData<Triangle>(0,0,1,1,0,1,captionBackgrounColor)}),
          new Rectangle(0,lineWidth,{newData<Line>(0,.5,1,.5,lineWidth,lineColor)}),//caption line
          new Split(0,{
            new Rectangle(margin,0),
            new Split(1,{
              new Rectangle(0,margin),
              new Split(0,{
                new Split(1,//inputs
                  repear1D(this->inputNames.size(),[&](size_t i)->Element*{
                    return new Split(0,{
                      new Rectangle(inputRadius*2,inputRadius*2,{newData<Circle>(.5,.5,inputRadius,lineWidth)}),
                      new Rectangle(textIndent,0),
                      new Rectangle(fontSize*this->inputNames[i].length(),fontSize*2,{newData<Text>(this->inputNames[i],fontSize,textColor)})
                      });
                    })
                  ),
                new Rectangle(inputOutputDistance,0),
                new Split(0,{
                  new Rectangle(fontSize*this->outputName.length(),fontSize*2,{newData<Text>(this->outputName,fontSize,textColor,glm::vec2(0,.5))}),
                  new Rectangle(textIndent,0),
                  new Rectangle(outputRadius*2,outputRadius*2,{newData<Circle>(.5,.5,outputRadius,lineWidth,lineColor)}),
                  }),
                //output
                }),
              new Rectangle(0,margin),
            }),
            new Rectangle(margin,0),
          },{newData<Triangle>(0,0,1,0,1,1,backgroundColor),newData<Triangle>(0,0,1,1,0,1,backgroundColor)}),
        }),
        new Rectangle(lineWidth,0,{newData<Line>(.5,0,.5,1,lineWidth,lineColor)}),//right line
      }),
      new Rectangle(0,lineWidth,{newData<Line>(0,.5,1,.5,lineWidth,lineColor)}),});//bottom line
  root->getSize();

  AddToNodeData data={this->draw2D,node};
  root->visitor(addToNode,&data);

  delete root;
}

class gde::EditorImpl{
  public:
    EditorImpl(ge::gl::Context const&g,glm::uvec2 const&size):gl(g){
      this->draw2d = std::make_shared<Draw2D>(gl);
      auto vv=this->draw2d->createViewport(size);
      auto ll=this->draw2d->createLayer();
      auto nn=this->draw2d->createNode();
      this->draw2d->insertLayer(vv,ll);
      this->draw2d->setLayerNode(ll,nn);
      this->draw2d->setRootViewport(vv);
      this->testFce = new gde::Function(this->draw2d,"addSome",{"valueA","valueB","valueC","val"},"output");
      this->testFce->create();
      this->draw2d->insertNode(nn,this->testFce->node);
    }
    Function*testFce;
    ~EditorImpl(){delete this->testFce;}
    ge::gl::Context const&gl;
    std::shared_ptr<Draw2D>draw2d;
    bool middleDown = false;
};

Editor::Editor(ge::gl::Context const&gl,glm::uvec2 const&size){
  this->_impl = new EditorImpl{gl,size};
}
Editor::~Editor(){
  delete this->_impl;
}

void Editor::mouseMotion(int32_t xrel,int32_t yrel,size_t x,size_t y){
  (void)xrel;
  (void)yrel;
  (void)x;
  (void)y;
  if(this->_impl->middleDown){
    this->_impl->draw2d->setCameraPosition(this->_impl->draw2d->getCameraPosition()+glm::vec2(-xrel,yrel));
  }
}

void Editor::mouseButtonDown(MouseButton b,size_t x,size_t y){
  (void)b;
  (void)x;
  (void)y;
  if(b==MIDDLE)this->_impl->middleDown = true;
}

void Editor::mouseButtonUp(MouseButton b,size_t x,size_t y){
  (void)b;
  (void)x;
  (void)y;
  if(b==MIDDLE)this->_impl->middleDown = false;
}

void Editor::mouseWheel(int32_t x,int32_t y){
  (void)x;
  (void)y;
  this->_impl->draw2d->setCameraScale(glm::clamp(this->_impl->draw2d->getCameraScale()+y*0.1f,0.001f,10.f));
}
void Editor::resize(size_t w,size_t h){
  this->_impl->draw2d->setViewportSize(glm::uvec2(w,h));
}

void Editor::draw(){
  this->_impl->draw2d->draw();
}

