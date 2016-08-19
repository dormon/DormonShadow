#include<GEDEEditor.h>
#include<algorithm>

#include<UI.h>
using namespace gde;

struct AddToNodeData{
  std::shared_ptr<Draw2D>draw2D;
  size_t node;
};

const size_t DATA_PRIMITIVE = 0;

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
  this->inputRadius = 4;
  this->outputRadius = 4;
  this->lineWidth = 3;
  this->node = this->draw2D->createNode();
  using namespace ui;

#if 1
  auto root = new Split(1,{
      new Rectangle(0,this->lineWidth,{newData<Line>(0,0,.5,1,.5,this->lineWidth,this->lineColor)}),//top line
      new Split(0,{
        new Rectangle(this->lineWidth,0,{newData<Line>(0,.5,0,.5,1,this->lineWidth,this->lineColor)}),//left line
        new Split(1,{
          new Split(1,{
            new Rectangle(0,this->captionMargin),
            new Split(0,{
              new Rectangle(this->captionMargin,0),
              new Rectangle(this->captionFontSize*this->functionName.length(),this->captionFontSize*2,{newData<Text>(0,this->functionName,this->captionFontSize,this->captionColor)}),
              new Rectangle(this->captionMargin,0),
              }),
            new Rectangle(0,this->captionMargin),
            },{newData<Triangle>(0,0,0,1,0,1,1,this->captionBackgrounColor),newData<Triangle>(0,0,0,1,1,0,1,this->captionBackgrounColor)}),
          new Rectangle(0,this->lineWidth,{newData<Line>(0,0,.5,1,.5,this->lineWidth,this->lineColor)}),//caption line
          new Split(0,{
            new Rectangle(this->margin,0),
            new Split(1,{
              new Rectangle(0,this->margin),
              new Split(0,{
                new Split(1,//inputs
                  repear1D(this->inputNames.size(),[this](size_t i)->Element*{
                    return new Split(0,{
                      new Rectangle(this->inputRadius*2,0,{newData<Circle>(0,.5,.5,this->inputRadius,this->lineWidth)}),
                      new Rectangle(this->textIndent,0),
                      new Rectangle(this->fontSize*this->inputNames[i].length(),this->fontSize*2,{newData<Text>(0,this->inputNames[i],this->fontSize,this->captionColor)})
                      });
                    })
                  ),
                new Rectangle(this->inputOutputDistance,0),
                new Split(0,{
                  new Rectangle(this->fontSize*this->outputName.length(),this->fontSize*2,{newData<Text>(0,this->outputName,this->fontSize,this->captionColor,glm::vec2(0,.5))}),
                  new Rectangle(this->textIndent,0),
                  new Rectangle(this->outputRadius*2,0,{newData<Circle>(0,.5,.5,this->outputRadius,this->lineWidth,this->lineColor)}),
                  }),
                //output
                }),
              new Rectangle(0,this->margin),
              }),
            new Rectangle(this->margin,0),
            },{newData<Triangle>(0,0,0,1,0,1,1,this->backgroundColor),newData<Triangle>(0,0,0,1,1,0,1,this->backgroundColor)}),
          }),
        new Rectangle(this->lineWidth,0,{newData<Line>(0,.5,0,.5,1,this->lineWidth,this->lineColor)}),//right line
        }),
      new Rectangle(0,this->lineWidth,{newData<Line>(0,0,.5,1,.5,this->lineWidth,this->lineColor)}),});//bottom line
  root->getSize();

  AddToNodeData data={this->draw2D,node};
  root->visitor(addToNode,&data);

  delete root;
#endif

#if 0
  //Vertical SPLIT TEST
  auto root = new Split<1>({
      new Rectangle(0,this->lineWidth,{new Line(0,.5,1,.5,this->lineWidth)}),//top line
      new Rectangle(0,30),
      new Rectangle(4*this->captionFontSize,this->captionFontSize*2,{new Text("AHOJ",8)}),
      new Rectangle(0,30),
      new Rectangle(0,this->lineWidth,{new Line(0,.5,1,.5,this->lineWidth)}),//bottom line
      });
  root->addToNode(this->draw2D,node);
  delete root;
#endif

#if 0
  //HORIZONTAL SPLIT TEST
  auto root = new Split<0>({
      new Rectangle(this->lineWidth,0,{new Line(.5,0,.5,1,this->lineWidth)}),//left line
      new Rectangle(30,0),
      new Rectangle(4*this->captionFontSize,this->captionFontSize*2,{new Text("AHOJ",8)}),
      new Rectangle(30,0),
      new Rectangle(this->lineWidth,0,{new Line(.5,0,.5,1,this->lineWidth)}),//right line
      });
  root->addToNode(this->draw2D,node);
  delete root;
#endif

#if 0
  //HORIZONTAL THAN VERTICAL SPLIT TEST
  auto root = new Split<0>({
      new Rectangle(this->lineWidth,0,{new Line(.5,0,.5,1,this->lineWidth)}),//left line
      new Rectangle(30,0),
      new Split<1>({
        new Rectangle(0,this->lineWidth,{new Line(0,.5,1,.5,this->lineWidth)}),//top line
        new Rectangle(0,30),
        new Rectangle(4*this->captionFontSize,this->captionFontSize*2,{new Text("AHOJ",8)}),
        new Rectangle(0,30),
        new Rectangle(0,this->lineWidth,{new Line(0,.5,1,.5,this->lineWidth)}),//bottom line
        }),
      new Rectangle(30,0),
      new Rectangle(this->lineWidth,0,{new Line(.5,0,.5,1,this->lineWidth)}),//left line
      });
  root->addToNode(this->draw2D,node);
  delete root;
#endif


}

