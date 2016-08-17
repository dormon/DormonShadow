#include<GEDEEditor.h>
#include<tuple>

using namespace gde;

namespace ui{
  template<size_t X=0,typename std::enable_if<(X==0||X==1),unsigned>::type = 0>
    class Split;
  class Grid;
  class Element{
    template<size_t X,typename std::enable_if<(X==0||X==1),unsigned>::type>
    friend class Split;
    friend class Grid;
    public:
      enum Type{
        SPLITX    = 0,
        SPLITY    = 1,
        GRID      = 2,
        RECTANGLE = 3,
      }type;
      Element(Type const&t,glm::vec2 const&minSize = glm::vec2(0.f)):type(t),_minSize(minSize){}
      virtual ~Element(){}
      virtual glm::vec2 getSize()const = 0;
      glm::vec2 getPosition(){
        assert(this!=nullptr);
        if(!this->_parent)return glm::vec2(0.f);
        if(this->_changedGuts)
          this->_position = this->_parent->_getPositionOf(this);
        return this->_position;
      }
    protected:
      glm::vec2 _minSize;
      Element*_parent = nullptr;
      bool _changedGuts = true;
      glm::vec2 _size = glm::vec2(0.f);
      glm::vec2 _position = glm::vec2(0.f);
      void _signalParents(){
        Element*p=this->_parent;
        while(p){
          p->_changedGuts = true;
          p=p->_parent;
        }
      }
      virtual glm::vec2 _getPositionOf(Element*e) = 0;
  };

  enum Spacing{
    LEFT_EQUAL,
    LEFT,
    MIDDLE_EQUAL,
    MIDDLE,
    RIGHT_EQUAL,
    RIGHT,
  };

  template<size_t X,typename std::enable_if<(X==0||X==1),unsigned>::type>
    class Split: public Element{
      public:
        Split(std::vector<Element*>const&elements,Spacing const&spacing,glm::vec2 const&minSize = glm::vec2(0)):Element((Type)(SPLITX+X),minSize),_inners(elements),_spacing(spacing){
          assert(this!=nullptr);
          for(auto const&x:elements)x->_parent = this;
          this->_signalParents();
          for(auto const&x:elements)
            this->_positions[x]=glm::vec2(0.f);
        }
        virtual ~Split(){for(auto const&x:this->_inners)delete x;}
        virtual glm::vec2 getSize()const override{
          assert(this!=nullptr);
          if(this->_changedGuts){
            glm::vec2 newSize = glm::vec2(0.f);
            newSize[1-X] = this->_minSize[1-X];
            std::vector<float>parts;
            float largestPart = 0;
            for(auto const&x:this->_inners){
              auto is = x->getSize();
              newSize[1-X] = glm::max(is[1-X],newSize[1-X]);
              newSize[X]+=is[X];
              largestPart = glm::max(largestPart,is[X]);
              parts.push_back(is[X]);
            }
            newSize[X] = glm::max(newSize[X],this->_minSize[X]);
            this->_size = newSize;
            float offset=0;
            bool useLargestStep = false;
            switch(this->_spacing){
              case LEFT:
                break;
              case LEFT_EQUAL:
                useLargestStep = true;
                break;
              case RIGHT:
                if(newSize[X]<this->_minSize[X])
                  offset = this->_minSize[X]-newSize[X];
                break;
              case RIGHT_EQUAL:
                if(largestPart*this->_inners.size()<this->_minSize[X])
                  offset = this->_minSize[X]-largestPart*this->_inners.size();
                useLargestStep = true;
                break;
              case MIDDLE:
                if(newSize[X]<this->_minSize[X])
                  offset = (this->_minSize[X]-newSize[X])/2.f;
                break;
              case MIDDLE_EQUAL:
                if(largestPart*this->_inners.size()<this->_minSize[X])
                  offset = (this->_minSize[X]-largestPart*this->_inners.size())/2.f;
                useLargestStep = true;
                break;
            }
            for(size_t i=0;i<this->_inners.size();++i){
              glm::vec2 newPos = glm::vec2(0.f);
              newPos[X]=offset;
              assert(this->_positions.count(this->_inners.at(i))!=0);
              this->_positions.at(this->_inners.at(i))=newPos;
              if(useLargestStep)offset+=largestPart;
              else offset+=parts.at(i);
            }
            this->_changedGuts = false;
          }
          return this->_size;
        }
      protected:
        std::vector<Element*>_inners;
        Spacing _spacing;
        std::map<Element const*,glm::vec2>_positions;
        virtual glm::vec2 _getPositionOf(Element*e){
          assert(this!=nullptr);
          assert(this->_position.count(e)!=0);
          return this->getPosition()+this->_positions.at(e);
        }
    };

  class Grid: public Element{
    public:
    protected:
  };

  class Rectangle: public Element{
    public:
      Rectangle(glm::vec2 const&minSize = glm::vec2(0.f)):Element(RECTANGLE,minSize){}
      virtual ~Rectangle(){}
      virtual glm::vec2 getSize()const override{
        assert(this!=nullptr);
        return this->_minSize;
      }
  };
}


void Function::create(){
  size_t inputLength = 0;
  for(auto const&x:this->inputNames)
    inputLength = glm::max(inputLength,x.length());
  size_t outputLength = this->outputName.length();
  size_t captionLength = this->functionName.length();

  size_t inputLineHeight = glm::max(this->inputRadius*2,this->fontSize*2);
  bool hasOutput = this->outputName!="";

  this->node = this->draw2D->createNode();

  {//create rectangleLines
    size_t wa =
      this->lineWidth+
      this->margin+
      inputLength*this->fontSize+
      this->inputOutputDistance+
      outputLength*this->fontSize+
      this->textIndent+
      this->outputRadius*2+
      this->margin+
      this->lineWidth;
    size_t wb = 
      this->lineWidth+
      this->captionMargin+
      captionLength*this->captionFontSize+
      this->captionMargin+
      this->lineWidth;
    size_t width = glm::max(wa,wb);
    size_t height;
    height = 
      this->lineWidth+
      this->margin+
      this->inputNames.size()*inputLineHeight+
      (this->inputNames.size()==0?(size_t)hasOutput:(this->inputNames.size()-1))*this->inputSpacing+
      this->margin+
      this->lineWidth+
      this->captionMargin+
      this->captionFontSize*2+
      this->captionMargin+
      this->lineWidth;

    this->rectangleLines[0] = this->draw2D->createPrimitive(std::make_shared<Line>(glm::vec2(0,0),glm::vec2(width,0),this->lineWidth,this->lineColor));
    this->rectangleLines[1] = this->draw2D->createPrimitive(std::make_shared<Line>(glm::vec2(0,0),glm::vec2(0,height),this->lineWidth,this->lineColor));
    this->rectangleLines[2] = this->draw2D->createPrimitive(std::make_shared<Line>(glm::vec2(width,0),glm::vec2(width,height),this->lineWidth,this->lineColor));
    this->rectangleLines[3] = this->draw2D->createPrimitive(std::make_shared<Line>(glm::vec2(0,height),glm::vec2(width,height),this->lineWidth,this->lineColor));


    this->draw2D->insertPrimitive(this->node,this->rectangleLines[0]);
    this->draw2D->insertPrimitive(this->node,this->rectangleLines[1]);
    this->draw2D->insertPrimitive(this->node,this->rectangleLines[2]);
    this->draw2D->insertPrimitive(this->node,this->rectangleLines[3]);
  }

  {//main triangles
    size_t wa =
      this->margin+
      inputLength*this->fontSize+
      this->inputOutputDistance+
      outputLength*this->fontSize+
      this->textIndent+
      this->outputRadius*2+
      this->margin;
    size_t wb = 
      this->captionMargin+
      captionLength*this->captionFontSize+
      this->captionMargin;
    size_t width = glm::max(wa,wb);
    size_t height;
    height = 
      this->margin+
      this->inputNames.size()*inputLineHeight+
      (this->inputNames.size()==0?(size_t)hasOutput:(this->inputNames.size()-1))*this->inputSpacing+
      this->margin;

    size_t px = this->lineWidth;
    size_t py = this->lineWidth;

    this->rectangleTriangles[0] = this->draw2D->createPrimitive(std::make_shared<Triangle>(glm::vec2(px,py),glm::vec2(px+width,py),glm::vec2(px+width,py+height),this->backgroundColor));
    this->rectangleTriangles[1] = this->draw2D->createPrimitive(std::make_shared<Triangle>(glm::vec2(px,py),glm::vec2(px+width,py+height),glm::vec2(px,py+height),this->backgroundColor));

    this->draw2D->insertPrimitive(this->node,this->rectangleTriangles[0]);
    this->draw2D->insertPrimitive(this->node,this->rectangleTriangles[1]);
  }

  {//caption line
    size_t wa =
      this->margin+
      inputLength*this->fontSize+
      this->inputOutputDistance+
      outputLength*this->fontSize+
      this->textIndent+
      this->outputRadius*2+
      this->margin;
    size_t wb = 
      this->captionMargin+
      captionLength*this->captionFontSize+
      this->captionMargin;
    size_t width = glm::max(wa,wb);
    size_t px = this->lineWidth;
    size_t py =
      this->lineWidth+
      this->margin+
      this->inputNames.size()*inputLineHeight+
      (this->inputNames.size()==0?(size_t)hasOutput:(this->inputNames.size()-1))*this->inputSpacing+
      this->margin;

    this->captionLine = this->draw2D->createPrimitive(std::make_shared<Line>(glm::vec2(px,py),glm::vec2(px+width,py),this->lineWidth,this->lineColor));

    this->draw2D->insertPrimitive(this->node,this->captionLine);
  }

  {//caption triangles
    size_t wa =
      this->margin+
      inputLength*this->fontSize+
      this->inputOutputDistance+
      outputLength*this->fontSize+
      this->textIndent+
      this->outputRadius*2+
      this->margin;
    size_t wb = 
      this->captionMargin+
      captionLength*this->captionFontSize+
      this->captionMargin;
    size_t width = glm::max(wa,wb);
    size_t height;
    height = 
      this->captionMargin+
      this->captionFontSize*2+
      this->captionMargin;

    size_t px = this->lineWidth;
    size_t py = 
      this->lineWidth+
      this->margin+
      this->inputNames.size()*inputLineHeight+
      (this->inputNames.size()==0?(size_t)hasOutput:(this->inputNames.size()-1))*this->inputSpacing+
      this->margin+
      this->lineWidth;

    this->captionTriangles[0] = this->draw2D->createPrimitive(std::make_shared<Triangle>(glm::vec2(px,py),glm::vec2(px+width,py),glm::vec2(px+width,py+height),this->captionBackgrounColor));
    this->captionTriangles[1] = this->draw2D->createPrimitive(std::make_shared<Triangle>(glm::vec2(px,py),glm::vec2(px+width,py+height),glm::vec2(px,py+height),this->captionBackgrounColor));

    this->draw2D->insertPrimitive(this->node,this->captionTriangles[0]);
    this->draw2D->insertPrimitive(this->node,this->captionTriangles[1]);
  }

  {//caption
    size_t px = this->lineWidth+this->captionMargin;
    size_t py = 
      this->lineWidth+
      this->margin+
      this->inputNames.size()*inputLineHeight+
      (this->inputNames.size()==0?(size_t)hasOutput:(this->inputNames.size()-1))*this->inputSpacing+
      this->margin+
      this->lineWidth+
      this->captionMargin;

    this->captionText = this->draw2D->createPrimitive(std::make_shared<Text>(this->functionName,this->captionFontSize,glm::vec2(px,py),glm::vec2(1.f,0.f),this->captionColor));

    this->draw2D->insertPrimitive(this->node,this->captionText);
  }

  {//inputs circles
    size_t px = this->lineWidth+this->margin+this->inputRadius;
    size_t py = 
      this->lineWidth+
      this->margin+
      this->fontSize;
    size_t step = this->fontSize*2+this->inputSpacing;

    for(size_t i=0;i<this->inputNames.size();++i){
      this->inputCircle.push_back(this->draw2D->createPrimitive(std::make_shared<Circle>(glm::vec2(px,py),this->inputRadius,this->lineWidth,this->lineColor)));
      py+=step;
    }
    for(auto const&x:this->inputCircle)
      this->draw2D->insertPrimitive(this->node,x);
  }
  {//inputs texts
    size_t px = this->lineWidth+this->margin+this->inputRadius*2+this->textIndent;
    size_t py = 
      this->lineWidth+
      this->margin;
    size_t step = this->fontSize*2+this->inputSpacing;

    for(auto const&x:this->inputNames){
      this->inputText.push_back(this->draw2D->createPrimitive(std::make_shared<Text>(x,this->fontSize,glm::vec2(px,py),glm::vec2(1.f,0.f),this->textColor)));
      py+=step;
    }
    for(auto const&x:this->inputText)
      this->draw2D->insertPrimitive(this->node,x);
  }

  class Push{
    glm::uvec2 oldOrigin,oldSize;
    glm::uvec2&refOrigin,&refSize;
    public:
    Push(glm::uvec2&origin,glm::uvec2&size):refOrigin(origin),refSize(size){oldOrigin = origin;oldSize = size;}
    ~Push(){refOrigin = oldOrigin;refSize = oldSize;}
  };

  size_t wa =
    this->lineWidth+
    this->margin+
    inputLength*this->fontSize+
    this->inputOutputDistance+
    outputLength*this->fontSize+
    this->textIndent+
    this->outputRadius*2+
    this->margin+
    this->lineWidth;
  size_t wb = 
    this->lineWidth+
    this->captionMargin+
    captionLength*this->captionFontSize+
    this->captionMargin+
    this->lineWidth;
  size_t width = glm::max(wa,wb);
  size_t height;
  height = 
    this->lineWidth+
    this->margin+
    this->inputNames.size()*inputLineHeight+
    (this->inputNames.size()==0?(size_t)hasOutput:(this->inputNames.size()-1))*this->inputSpacing+
    this->margin+
    this->lineWidth+
    this->captionMargin+
    this->captionFontSize*2+
    this->captionMargin+
    this->lineWidth;
  glm::uvec2 origin = glm::uvec2(0,0);
  glm::uvec2 size = glm::uvec2(width,height);
  {
    //
    {
      Push Stack{origin,size};
      origin+=glm::uvec2(this->lineWidth);
      size-=glm::uvec2(this->lineWidth*2);
    }
  }
  /*
     {//output circle
     size_t px =
     this->lineWidth+
     this->margin+
     inputLength*this->fontSize+
     this->inputOutputDistance+
     outputLength*this->fontSize+
     this->textIndent+
     this->outputRadius;
     size_t wb = 
     this->lineWidth+
     this->captionMargin+
     captionLength*this->captionFontSize+
     this->captionMargin+
     this->lineWidth;
     size_t width = glm::max(wa,wb);
     size_t height;
     height = 
     this->lineWidth+
     this->margin+
     this->inputNames.size()*inputLineHeight+
     (this->inputNames.size()==0?(size_t)hasOutput:(this->inputNames.size()-1))*this->inputSpacing+
     this->margin+
     this->lineWidth+
     this->captionMargin+
     this->captionFontSize*2+
     this->captionMargin+
     this->lineWidth;

     size_t px = this->lineWidth+this->margin+this->inputRadius;
     size_t py = 
     this->lineWidth+
     this->margin+
     this->fontSize;
     size_t step = this->fontSize*2+this->inputSpacing;

     for(size_t i=0;i<this->inputNames.size();++i){
     this->inputCircle.push_back(this->draw2D->createPrimitive(std::make_shared<Circle>(glm::vec2(px,py),this->inputRadius,this->lineWidth,this->lineColor)));
     py+=step;
     }
     for(auto const&x:this->inputCircle)
     this->draw2D->insertPrimitive(this->node,x);
     }
     */

}

