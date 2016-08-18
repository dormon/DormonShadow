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
    virtual glm::vec2 getSize() = 0;
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
          assert(elements.size()>0);
          for(auto const&x:elements)x->_parent = this;
          this->_signalParents();
          for(auto const&x:elements)
            this->_positions[x]=glm::vec2(0.f);
        }
        virtual ~Split(){for(auto const&x:this->_inners)delete x;}
        virtual glm::vec2 getSize()override{
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
            float offset=0;
            bool useLargestStep = false;
            switch(this->_spacing){
              case LEFT:
                break;
              case LEFT_EQUAL:
                useLargestStep = true;
                newSize[X] = largestPart*this->_inners.size();
                break;
              case RIGHT:
                if(newSize[X]<this->_minSize[X])
                  offset = this->_minSize[X]-newSize[X];
                break;
              case RIGHT_EQUAL:
                newSize[X] = largestPart*this->_inners.size();
                if(newSize[X]<this->_minSize[X])
                  offset = this->_minSize[X]-newSize[X];
                useLargestStep = true;
                break;
              case MIDDLE:
                if(newSize[X]<this->_minSize[X])
                  offset = (this->_minSize[X]-newSize[X])/2.f;
                break;
              case MIDDLE_EQUAL:
                newSize[X] = largestPart*this->_inners.size();
                if(newSize[X]<this->_minSize[X])
                  offset = (this->_minSize[X]-newSize[X])/2.f;
                useLargestStep = true;
                break;
            }
            newSize[X] = glm::max(newSize[X],this->_minSize[X]);
            this->_size = newSize;

            for(size_t i=0;i<this->_inners.size();++i){
              assert(this->_positions.count(this->_inners.at(i))!=0);
              this->_positions.at(this->_inners.at(i))=offset;
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
        virtual glm::vec2 _getPositionOf(Element*e)override{
          assert(this!=nullptr);
          assert(this->_position.count(e)!=0);
          return this->getPosition()+this->_positions.at(e);
        }
    };

  class Grid: public Element{
    public:
      Grid(std::vector<std::vector<Element*>>const&elements,Spacing const&spacingX,Spacing const&spacingY,glm::vec2 const&minSize = glm::vec2(0)):Element(GRID,minSize),_inners(elements){
        assert(this!=nullptr);
        this->_gridSize.x = elements.size();
        assert(elements.size()>0);
        this->_gridSize.y = elements.at(0).size();
        for(auto const&x:elements){
          assert(this->_gridSize.x==x.size());
          (void)x;
        }
        this->_spacings[0] = spacingX;
        this->_spacings[1] = spacingY;
        for(auto const&x:elements)
          for(auto const&y:x)
            y->_parent = this;
        this->_signalParents();
        for(auto const&x:elements)
          for(auto const&y:x)
          this->_positions[y]=glm::vec2(0.f);
      }
      virtual ~Grid(){for(auto const&x:this->_inners)for(auto const&y:x)delete y;}
      virtual glm::vec2 getSize()override{
        assert(this!=nullptr);
        if(this->_changedGuts){
          glm::vec2 newSize = glm::vec2(0.f);
          std::vector<std::vector<glm::vec2>>parts;
          std::vector<float>rcSize[2];
          rcSize[0].resize(this->_gridSize[1],0.f);
          rcSize[1].resize(this->_gridSize[0],0.f);
          glm::vec2 largestPart = glm::vec2(0);
          for(size_t j=0;j<this->_gridSize.y;++j){
            auto row = std::vector<glm::vec2>();
            for(size_t i=0;i<this->_gridSize.x;++i)
              row.push_back(this->_inners.at(j).at(i)->getSize());
            parts.push_back(row);
          }
          for(size_t j=0;j<this->_gridSize.y;++j){
            for(size_t i=0;i<this->_gridSize.x;++i){
              auto is = parts.at(j).at(i);
              rcSize[0][j] = glm::max(rcSize[0][j],is[1]);
              rcSize[1][i] = glm::max(rcSize[1][i],is[0]);
              largestPart.x = glm::max(largestPart.x,is.x);
              largestPart.y = glm::max(largestPart.y,is.y);
            }
          }
          for(size_t k=0;k<2;++k)
            for(size_t i=0;i<this->_gridSize[k];++i)
              newSize[k]+=rcSize[1-k][i];

          glm::vec2 offset = glm::vec2(0.f);
          glm::bvec2 useLargestStep = glm::bvec2(false);
          for(size_t i=0;i<2;++i)
          switch(this->_spacings[i]){
            case LEFT:
              break;
            case LEFT_EQUAL:
              newSize[i] = largestPart[i]*this->_gridSize[i];
              useLargestStep[i] = true;
              break;
            case RIGHT:
              if(newSize[i]<this->_minSize[i])
                offset[i] = this->_minSize[i]-newSize[i];
              break;
            case RIGHT_EQUAL:
              newSize[i] = largestPart[i]*this->_gridSize[i];
              if(newSize[i]<this->_minSize[i])
                offset[i] = this->_minSize[i]-newSize[i];
              useLargestStep[i] = true;
              break;
            case MIDDLE:
              if(newSize[i]<this->_minSize[i])
                offset[i] = (this->_minSize[i]-newSize[i])/2.f;
              break;
            case MIDDLE_EQUAL:
              newSize[i] = largestPart[i]*this->_gridSize[i];
              if(newSize[i]<this->_minSize[i])
                offset[i] = (this->_minSize[i]-newSize[i])/2.f;
              useLargestStep[i] = true;
              break;
          }
          this->_size = glm::max(newSize,this->_minSize);
          glm::vec2 o = offset;
          for(size_t j=0;j<this->_gridSize.y;++j){
            o.x = offset.x;
            for(size_t i=0;i<this->_gridSize.x;++i){
              this->_positions.at(this->_inners.at(j).at(i))=o;
              if(useLargestStep.x)o.x+=largestPart.x;
              else o.x+=rcSize[1][i];
            }
            if(useLargestStep.y)o.y+=largestPart.y;
            else o.y+=rcSize[0][j];
          }
          this->_changedGuts = false;
        }
        return this->_size;
      }
    protected:
      glm::uvec2 _gridSize;
      std::vector<std::vector<Element*>>_inners;
      Spacing _spacings[2];
      std::map<Element const*,glm::vec2>_positions;
      virtual glm::vec2 _getPositionOf(Element*e)override{
        assert(this!=nullptr);
        assert(this->_positions.count(e)!=0);
        return this->getPosition()+this->_positions.at(e);
      }
  };

  class Rectangle: public Element{
    public:
      Rectangle(glm::vec2 const&minSize = glm::vec2(0.f)):Element(RECTANGLE,minSize){}
      virtual ~Rectangle(){}
      virtual glm::vec2 getSize()override{
        assert(this!=nullptr);
        return this->_minSize;
      }
  };
  
  template<size_t N,typename T,typename...ARGS>
    std::vector<T*>repeat(ARGS...args){
      std::vector<T*>result;
      for(size_t i=0;i<N;++i)
        result.push_back(new T(args...));
      return result;
    }
  template<size_t X,size_t Y,typename T,typename...ARGS>
    std::vector<std::vector<T*>>repeat(ARGS...args){
      std::vector<std::vector<T*>>result;
      for(size_t j=0;j<Y;++j){
        auto row = std::vector<T*>();
        for(size_t i=0;i<X;++i)
          row.push_back(new T(args...));
        result.push_back(row);
      }
      return result;
    }

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

