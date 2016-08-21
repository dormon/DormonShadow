#include<GEDEEditor.h>
#include<Draw2D.h>
#include<algorithm>

#include<GDEImpl.h>

#include<UI.h>
using namespace gde;

struct AddToNodeData{
  std::shared_ptr<Draw2D>draw2D;
  size_t node;
};

class Event{
  public:
    enum Type{
      HOVER,
    }type;
    std::function<void(ui::Element*,void*)>callback = nullptr;
    void*userData = nullptr;
    Event(
        Type const&t,
        std::function<void(ui::Element*,void*)>const&c,
        void*data = nullptr):type(t),callback(c),userData(data){}
    void operator()(ui::Element*elem){
      assert(this->callback!=nullptr);
      this->callback(elem,this->userData);
    }
    ~Event(){}
};


const size_t DATA_PRIMITIVE = 0;
const size_t DATA_EVENT = 1;

/*
namespace ui{
  template<>inline size_t getTypeId<Line>(){return DATA_PRIMITIVE;}
  template<>inline size_t getTypeId<Point>(){return DATA_PRIMITIVE;}
  template<>inline size_t getTypeId<Circle>(){return DATA_PRIMITIVE;}
  template<>inline size_t getTypeId<Triangle>(){return DATA_PRIMITIVE;}
  template<>inline size_t getTypeId<Text>(){return DATA_PRIMITIVE;}
  template<>inline size_t getTypeId<Spline>(){return DATA_PRIMITIVE;}
  template<>inline size_t getTypeId<Event>(){return DATA_EVENT;}
}*/

void addToNode(ui::Element*elm,void*d){
  (void)elm;
  (void)d;
  return;
  /*
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
     */
}

void addToNode2(ui::Element*elm,void*d){
  auto node = (Node*)d;
  glm::vec2 p = elm->getPosition();
  glm::vec2 s = elm->getSize();
  if(elm->data.hasValues<Line>()){
    auto v = elm->data.getValues<Line>();
    for(auto const&vv:v){
      auto x = (Line*)vv;
      node->addValue<Line>(
          x->points[0]*s+p,
          x->points[1]*s+p,
          x->width,
          x->color);
    }
  }
  if(elm->data.hasValues<Point>()){
    auto v = elm->data.getValues<Point>();
    for(auto const&vv:v){
      auto x = (Point*)vv;
      node->addValue<Point>(
          x->point*s+p,
          x->size,
          x->color);
    }
  }
  if(elm->data.hasValues<Circle>()){
    auto v = elm->data.getValues<Circle>();
    for(auto const&vv:v){
      auto x = (Circle*)vv;
      node->addValue<Circle>(
          x->point*s+p,
          x->size,
          x->width,
          x->color);
    }
  }
  if(elm->data.hasValues<Triangle>()){
    auto v = elm->data.getValues<Triangle>();
    for(auto const&vv:v){
      auto x = (Triangle*)vv;
      node->addValue<Triangle>(
          x->points[0]*s+p,
          x->points[1]*s+p,
          x->points[2]*s+p,
          x->color);
    }
  }
  if(elm->data.hasValues<Spline>()){
    auto v = elm->data.getValues<Spline>();
    for(auto const&vv:v){
      auto x = (Spline*)vv;
      node->addValue<Spline>(
          x->points[0]*s+p,
          x->points[1]*s+p,
          x->points[2]*s+p,
          x->points[3]*s+p,
          x->width,
          x->color);
    }
  }
  if(elm->data.hasValues<Text>()){
    auto v = elm->data.getValues<Text>();
    for(auto const&vv:v){
      auto x = (Text*)vv;
      node->addValue<Text>(
          x->data,
          x->size,
          x->position*s+p,
          x->direction,
          x->color);
    }
  }
}

void addMouseMotionEventToNode(ui::Element*elm,void*d){
  auto node = (Node2d*)d;
  glm::vec2 p = elm->getPosition();
  glm::vec2 s = elm->getSize();
  if(elm->data.hasValues<MouseMotionEvent>()){
    auto v = elm->data.getValues<MouseMotionEvent>();
    for(auto const&x:v){
      auto vv = (MouseMotionEvent*)x;
      node->addValue<MouseMotionEvent>(p,s,vv->callback,vv->userData);
    }
  }
}


class CallHoverData{
  public:
    int32_t x;
    int32_t y;
};

void callHover(ui::Element*elm,void*d){
  (void)elm;
  (void)d;
  return;
  /*
     auto data = (CallHoverData*)d;
     glm::vec2 p = elm->getPosition();
     glm::vec2 s = elm->getSize();
     if(data->x<p.x||data->y<p.y||data->x>s.x+p.x||data->y>s.y+p.y)return;
     for(auto const&dd:elm->data){
     if(dd->getId()!=DATA_EVENT)continue;
     auto event = (Event*)dd->getData();
     (*event)(elm);
     }
     */
}

class Function{
  public:
    std::shared_ptr<Draw2D>draw2D;
    std::string functionName;
    std::vector<std::string>inputNames;
    std::string outputName;
    size_t node;
    Function(
        std::shared_ptr<Draw2D> const&draw2D,
        std::string             const&fce,
        std::vector<std::string>const&inputs,
        std::string             const&output){
      this->draw2D = draw2D;
      this->functionName = fce;
      this->inputNames = inputs;
      this->outputName = output;
    }
    ui::Element*root = nullptr;
    void create();
    ~Function(){
      delete root;
      draw2D->deleteNode(this->node);
    }
};



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

  this->root = new Split(1,{
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
      new Rectangle(0,lineWidth,{newData<Line>(0,.5,1,.5,lineWidth,lineColor)}),//bottom line
  },{newData<MouseMotionEvent>([](){std::cerr<<"A";})});//{newData<Event>(Event::HOVER,[](Element*,void*){std::cerr<<"A";})});
  root->getSize();

  AddToNodeData data={this->draw2D,node};
  root->visitor(addToNode,&data);

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
      this->testFce = new Function(this->draw2d,"addSome",{"valueA","valueB","valueC","val"},"output");
      this->testFce->create();
      this->draw2d->insertNode(nn,this->testFce->node);
      this->edit = new Edit(g,size);
      this->testFce->root->visitor(addToNode2,this->edit->rootViewport->at(0)->root);
      this->testFce->root->visitor(addMouseMotionEventToNode,this->edit->rootViewport->at(0)->root);
    }
    Edit*edit;
    Function*testFce;
    ~EditorImpl(){delete this->testFce;delete this->edit;}
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
    this->_impl->draw2d->setCameraPosition(this->_impl->draw2d->getCameraPosition()+glm::vec2(-xrel,-yrel));
    return;
  }
  auto viewMatrix = this->_impl->draw2d->getCameraViewMatrix();
  auto modelMatrix = this->_impl->draw2d->getNodeTransform(this->_impl->testFce->node);
  auto newMouseCoord = glm::vec2(glm::inverse(viewMatrix*modelMatrix)*glm::vec3(x,y,1));
  CallHoverData data;
  data.x = newMouseCoord.x;
  data.y = newMouseCoord.y;
  this->_impl->testFce->root->visitor(callHover,&data);
  this->_impl->edit->mouseMotion(xrel,yrel,x,y);
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
  //  this->_impl->draw2d->draw();
  this->_impl->edit->draw();
}

