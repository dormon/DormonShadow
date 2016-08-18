#include<GEDEEditor.h>
#include<algorithm>

using namespace gde;
#include<UI.h>

void Function::create(){
  this->inputRadius = 4;
  this->outputRadius = 4;
  this->lineWidth = 3;
  this->node = this->draw2D->createNode();
  using namespace ui;

#if 1
  auto root = new Split(1,{
      new Rectangle(0,this->lineWidth,{new Line(0,.5,1,.5,this->lineWidth,this->lineColor)}),//top line
      new Split(0,{
        new Rectangle(this->lineWidth,0,{new Line(.5,0,.5,1,this->lineWidth,this->lineColor)}),//left line
        new Split(1,{
          new Split(1,{
            new Rectangle(0,this->captionMargin),
            new Split(0,{
              new Rectangle(this->captionMargin,0),
              new Rectangle(this->captionFontSize*this->functionName.length(),this->captionFontSize*2,{new Text(this->functionName,this->captionFontSize,this->captionColor)}),
              new Rectangle(this->captionMargin,0),
              }),
            new Rectangle(0,this->captionMargin),
            },{new Triangle(0,0,1,0,1,1,this->captionBackgrounColor),new Triangle(0,0,1,1,0,1,this->captionBackgrounColor)}),
          new Rectangle(0,this->lineWidth,{new Line(0,.5,1,.5,this->lineWidth,this->lineColor)}),//caption line
          new Split(0,{
            new Rectangle(this->margin,0),
            new Split(1,{
              new Rectangle(0,this->margin),
              new Split(0,{
                new Split(1,//inputs
                  repear1D(this->inputNames.size(),[this](size_t i)->Element*{
                    return new Split(0,{
                      new Rectangle(this->inputRadius*2,0,{new Circle(.5,.5,this->inputRadius,this->lineWidth)}),
                      new Rectangle(this->textIndent,0),
                      new Rectangle(this->fontSize*this->inputNames[i].length(),this->fontSize*2,{new Text(this->inputNames[i],this->fontSize,this->captionColor)})
                      });
                    })
                  ),
                new Rectangle(this->inputOutputDistance,0),
                new Split(0,{
                  new Rectangle(this->fontSize*this->outputName.length(),this->fontSize*2,{new Text(this->outputName,this->fontSize,this->captionColor,glm::vec2(0,.5))}),
                  new Rectangle(this->textIndent,0),
                  new Rectangle(this->outputRadius*2,0,{new Circle(.5,.5,this->outputRadius,this->lineWidth,this->lineColor)}),
                  }),
                //output
                }),
              new Rectangle(0,this->margin),
              }),
            new Rectangle(this->margin,0),
            },{new Triangle(0,0,1,0,1,1,this->backgroundColor),new Triangle(0,0,1,1,0,1,this->backgroundColor)}),
          }),
        new Rectangle(this->lineWidth,0,{new Line(.5,0,.5,1,this->lineWidth,this->lineColor)}),//right line
        }),
      new Rectangle(0,this->lineWidth,{new Line(0,.5,1,.5,this->lineWidth,this->lineColor)}),});//bottom line
  root->getSize();
  root->addToNode(this->draw2D,node);
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

