#include<GEDEEditor.h>
#include<algorithm>
#include<GDEImpl.h>
#include<UI.h>

using namespace gde;

#define ___ std::cerr<<__FILE__<<" :"<<__LINE__<<std::endl

void addToNode2(ui::Element*elm,void*d){
  auto node = (Node2d*)d;
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
      node->addValue<MouseMotionEvent>(p,s,vv->callback,vv->userData,vv->type);
    }
  }
}

void addMouseButtonEventToNode(ui::Element*elm,void*d){
  auto node = (Node2d*)d;
  glm::vec2 p = elm->getPosition();
  glm::vec2 s = elm->getSize();
  if(elm->data.hasValues<MouseButtonEvent>()){
    auto v = elm->data.getValues<MouseButtonEvent>();
    for(auto const&x:v){
      auto vv = (MouseButtonEvent*)x;
      node->addValue<MouseButtonEvent>(p,s,vv->callback,vv->userData,vv->down,vv->button);
    }
  }
}

class Function{
  public:
    std::string functionName;
    std::vector<std::string>inputNames;
    std::string outputName;
    Function(
        std::string             const&fce,
        std::vector<std::string>const&inputs,
        std::string             const&output){
      this->functionName = fce;
      this->inputNames = inputs;
      this->outputName = output;
      this->create();
    }
    ui::Element*root = nullptr;
    std::shared_ptr<Node2d>node = nullptr;
    void create();
    ~Function(){
      delete root;
    }
    size_t id = 0;
    void setLineColor(glm::vec4 const&color = glm::vec4(1.f));
    glm::vec2 grabedPosition;
    bool doMove = false;
    void move(glm::vec2 const&mouseDiff);
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
            },{
            newData<Triangle>(0,0,1,0,1,1,captionBackgrounColor),
            newData<Triangle>(0,0,1,1,0,1,captionBackgrounColor),
            //newData<MouseMotionEvent>([](void*ptr,glm::vec2 const&diff){((Function*)ptr)->move(diff);},this),
            newData<MouseMotionEvent>([](void*ptr,glm::vec2 const&diff){((Function*)ptr)->move(diff);},this,MouseMotionEvent::MOUSE_EXIT_BIT|MouseMotionEvent::MOUSE_MOVE_BIT),
            newData<MouseButtonEvent>([](void*ptr){((Function*)ptr)->doMove = true;std::cout<<"klikam"<<std::endl;},this,true,MouseButton::LEFT),
            newData<MouseButtonEvent>([](void*ptr){((Function*)ptr)->doMove = false;std::cout<<"klikam"<<std::endl;},this,false,MouseButton::LEFT),
            }),
          new Rectangle(0,lineWidth,{newData<Line>(0,.5,1,.5,lineWidth,lineColor)}),//caption line
          new Split(0,{
            new Rectangle(margin,0),
            new Split(1,{
              new Rectangle(0,margin),
              new Split(0,{
                new Split(1,//inputs
                  repear1D(this->inputNames.size(),[&](size_t i)->Element*{
                    return new Split(0,{
                      new Rectangle(inputRadius*2,inputRadius*2,{
                        newData<Circle>(.5,.5,inputRadius,lineWidth,lineColor),
                        newData<MouseMotionEvent>([](std::shared_ptr<void>const&ptr){std::cerr<<"input: "<<*(int32_t*)ptr.get()<<std::endl;},std::make_shared<int32_t>(i))
                        }),
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
  },{
    newData<MouseMotionEvent>([](void*ptr){((Function*)ptr)->setLineColor();},this,MouseMotionEvent::MOUSE_ENTER_BIT),
    newData<MouseMotionEvent>([](void*ptr){((Function*)ptr)->setLineColor(glm::vec4(0.f,1.f,0.f,1.f));},this,MouseMotionEvent::MOUSE_EXIT_BIT),
  });
  root->getSize();

  this->node = std::make_shared<Node2d>();
  root->visitor(addToNode2,&*this->node);
  root->visitor(addMouseMotionEventToNode,&*this->node);
  root->visitor(addMouseButtonEventToNode,&*this->node);
}

void Function::setLineColor(glm::vec4 const&color){
  assert(this!=nullptr);
  assert(this->node!=nullptr);
  if(!this->node->hasValues<Line>())return;
  auto v = this->node->getValues<Line>();
  for(auto const&x:v){
    auto vv=(Line*)x;
    vv->color = color;
  }
  if(!this->node->hasNamedValue<bool>("dataChanged"))return;
  *this->node->getNamedValue<bool>("dataChanged") = true;
}

void Function::move(glm::vec2 const&diff){
  assert(this!=nullptr);
  assert(this->node!=nullptr);
  if(!this->doMove)return;
  this->node->mat *= Edit::translate(diff); 
}

class gde::EditorImpl{
  public:
    EditorImpl(ge::gl::Context const&g,glm::uvec2 const&size):gl(g){
      this->functions.push_back(new Function("addSome",{"valueA","valueB","valueC","val"},"output"));
      this->functions.push_back(new Function("computeProjection",{"fovy","aspect","near","far"},"projection"));
      this->edit = new Edit(g,size);
      this->edit->functionsNode->pushNode(this->functions[0]->node);
      this->edit->functionsNode->pushNode(this->functions[1]->node);
      this->functions[1]->node->mat = Edit::translate(glm::vec2(100,100));
      //this->testFce->root->visitor(addToNode2,&*this->edit->functionsNode);
      //this->testFce->root->visitor(addMouseMotionEventToNode,&*this->edit->functionsNode);
    }
    Edit*edit;
    std::vector<Function*>functions;
    ~EditorImpl(){
      for(auto const&x:this->functions)
        delete x;
    }
    ge::gl::Context const&gl;
    bool mouseButtons[3] = {false,false,false};
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
  if(this->_impl->mouseButtons[MIDDLE]){
    this->_impl->edit->editViewport->cameraPosition+=glm::vec2(-xrel,-yrel);
    return;
  }
  this->_impl->edit->mouseMotion(xrel,yrel,x,y);
}

void Editor::mouseButtonDown(MouseButton b,size_t x,size_t y){
  (void)x;
  (void)y;
  this->_impl->mouseButtons[b] = true;
  this->_impl->edit->mouseButton(true,b,x,y);
}

void Editor::mouseButtonUp(MouseButton b,size_t x,size_t y){
  (void)x;
  (void)y;
  this->_impl->mouseButtons[b] = false;
  this->_impl->edit->mouseButton(false,b,x,y);
}

void Editor::mouseWheel(int32_t x,int32_t y){
  (void)x;
  (void)y;
  this->_impl->edit->editViewport->cameraScale = glm::clamp(this->_impl->edit->editViewport->cameraScale+y*0.1f,0.001f,10.f);
}
void Editor::resize(size_t w,size_t h){
  this->_impl->edit->rootViewport->cameraSize = glm::vec2(w,h);
  this->_impl->edit->editViewport->cameraSize = glm::vec2(w,h);
}

void Editor::draw(){
  this->_impl->edit->draw();
}

