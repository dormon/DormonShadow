#include<GEDEEditor.h>
#include<tuple>
#include<algorithm>

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
    Element(Type const&t,std::vector<Primitive*>const&prims = {},glm::vec2 const&minSize = glm::vec2(0.f)):type(t),primitives(prims),_minSize(minSize){}
    virtual ~Element(){for(auto const&x:this->primitives)delete x;}
    virtual glm::vec2 getSize() = 0;
    glm::vec2 getPosition(){
      assert(this!=nullptr);
      if(!this->_parent)return glm::vec2(0.f);
      //if(this->_changedGuts)
        this->_position = this->_parent->_getPositionOf(this);
      return this->_position;
    }
    std::vector<Primitive*>primitives;
    virtual void addToNode(std::shared_ptr<Draw2D>const&draw2D,size_t node){
      glm::vec2 p = this->getPosition();
      glm::vec2 s = this->getSize();
      for(auto const&x:this->primitives){
        size_t primId = -1;
        switch(x->type){
          case Primitive::LINE:
            primId = draw2D->createPrimitive(std::make_shared<Line>(
                  ((Line*)x)->points[0]*s+p,
                  ((Line*)x)->points[1]*s+p,
                  ((Line*)x)->width,
                  x->color));
            break;
          case Primitive::POINT:
            primId = draw2D->createPrimitive(std::make_shared<Point>(
                  ((Point*)x)->point*s+p,
                  ((Point*)x)->size,
                  x->color));
            break;
          case Primitive::CIRCLE:
            primId = draw2D->createPrimitive(std::make_shared<Circle>(
                  ((Circle*)x)->point*s+p,
                  ((Circle*)x)->size,
                  ((Circle*)x)->width,
                  x->color));
            break;
          case Primitive::TRIANGLE:
            primId = draw2D->createPrimitive(std::make_shared<Triangle>(
                  ((Triangle*)x)->points[0]*s+p,
                  ((Triangle*)x)->points[1]*s+p,
                  ((Triangle*)x)->points[2]*s+p,
                  x->color));
            break;
          case Primitive::TEXT:
            primId = draw2D->createPrimitive(std::make_shared<Text>(
                  ((Text*)x)->data,
                  ((Text*)x)->size,
                  ((Text*)x)->position*s+p,
                  ((Text*)x)->direction,
                  x->color));
            break;
          case Primitive::SPLINE:
            primId = draw2D->createPrimitive(std::make_shared<Spline>(
                  ((Spline*)x)->points[0]*s+p,
                  ((Spline*)x)->points[1]*s+p,
                  ((Spline*)x)->points[2]*s+p,
                  ((Spline*)x)->points[3]*s+p,
                  ((Spline*)x)->width,
                  x->color));
            break;
        }
        draw2D->insertPrimitive(node,primId);
      }
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
    virtual void _setSize(glm::vec2 const&newSize) = 0;
  };

  enum Spacing{
    LEFT,
    MIDDLE,
    RIGHT,
    EQUAL,
  };

  template<size_t X,typename std::enable_if<(X==0||X==1),unsigned>::type>
    class Split: public Element{
      public:
        Split(
            std::vector<Element*>const&elements,
            std::vector<Primitive*>const&prims = {},
            Spacing const&spacing = LEFT,
            glm::vec2 const&minSize = glm::vec2(0)):Element((Type)(SPLITX+X),prims,minSize),_inners(elements),_spacing(spacing){
          assert(this!=nullptr);
          assert(elements.size()>0);
          if(X==1)std::reverse(this->_inners.begin(),this->_inners.end());
          for(auto const&x:elements)x->_parent = this;
          this->_signalParents();
          for(auto const&x:elements){
            this->_positions[x] = glm::vec2(0.f);
            this->_sizes[x] = glm::vec2(0.f);
          }
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
            switch(this->_spacing){
              case LEFT:
                if(newSize[X]<this->_minSize[X])
                  parts.back() += this->_minSize[X]-newSize[X];
                break;
              case MIDDLE:
                if(newSize[X]<this->_minSize[X]){
                  float first = parts.at(0);
                  float last  = parts.back();
                  float toAdd = this->_minSize[X]-newSize[X];
                  if(first+toAdd<last){
                    parts.at(0)+=toAdd;
                  }else if(last+toAdd<first){
                    parts.back()+=toAdd;
                  }else{
                    parts.at(0) = (first+last+toAdd)/2;
                    parts.back() = (first+last+toAdd)/2;
                  }
                }
                break;
              case RIGHT:
                if(newSize[X]<this->_minSize[X])
                  parts.at(0) += this->_minSize[X]-newSize[X];
                break;
              case EQUAL:
                newSize[X] = largestPart*this->_inners.size();
                if(newSize[X]<this->_minSize[X])
                  for(auto&x:parts)
                    x = this->_minSize[X]/this->_inners.size();
                else
                  for(auto&x:parts)
                    x = largestPart;
                break;
            }
            //newSize[X] = glm::max(newSize[X],this->_minSize[X]);
            newSize = glm::max(newSize,this->_minSize);
            this->_size = newSize;

            float offset = 0;
            for(size_t i=0;i<this->_inners.size();++i){
              assert(this->_positions.count(this->_inners.at(i))!=0);
              glm::vec2 newPos = glm::vec2(0.f);
              newPos[X] = offset;
              newSize[X] = parts.at(i);
              this->_positions.at(this->_inners.at(i))=newPos;
              this->_sizes.at(this->_inners.at(i))=newSize;
              offset+=parts.at(i);
            }
            this->_changedGuts = false;
            for(auto const&x:this->_inners)
              x->_setSize(this->_sizes.at(x));
          }
          return this->_size;
        }
        virtual void addToNode(std::shared_ptr<Draw2D>const&draw2D,size_t node)override{
          this->Element::addToNode(draw2D,node);
          for(auto const&x:this->_inners)
            x->addToNode(draw2D,node);
        }
      protected:
        std::vector<Element*>_inners;
        Spacing _spacing;
        std::map<Element const*,glm::vec2>_positions;
        std::map<Element const*,glm::vec2>_sizes;
        virtual glm::vec2 _getPositionOf(Element*e)override{
          assert(this!=nullptr);
          assert(this->_positions.count(e)!=0);
          return this->getPosition()+this->_positions.at(e);
        }
        virtual void _setSize(glm::vec2 const&ns)override{
          glm::vec2 newSize = ns;
          if(newSize==this->_size)return;
          assert(this!=nullptr);
          std::vector<float>parts;
          for(auto const&x:this->_inners)
            parts.push_back(this->_sizes.at(x)[X]);
          assert(newSize[X]>=this->_minSize[X]);
          switch(this->_spacing){
            case LEFT:
              if(newSize[X]>this->_size[X])
                parts.back() += newSize[X]-this->_size[X];
              break;
            case MIDDLE:
              if(newSize[X]>this->_size[X]){
                float first = parts.at(0);
                float last  = parts.back();
                float toAdd = newSize[X]-this->_size[X];
                if(first+toAdd<last){
                  parts.at(0)+=toAdd;
                }else if(last+toAdd<first){
                  parts.back()+=toAdd;
                }else{
                  parts.at(0) = (first+last+toAdd)/2;
                  parts.back() = (first+last+toAdd)/2;
                }
              }
              break;
            case RIGHT:
              if(newSize[X]>this->_size[X])
                parts.at(0) += newSize[X]-this->_size[X];
              break;
            case EQUAL:
              for(auto&x:parts)
                x = newSize[X]/this->_inners.size();
              break;
          }
          this->_size = newSize;

          float offset = 0;
          for(size_t i=0;i<this->_inners.size();++i){
            assert(this->_positions.count(this->_inners.at(i))!=0);
            glm::vec2 newPos = glm::vec2(0.f);
            newPos[X] = offset;
            newSize[X] = parts.at(i);
            this->_positions.at(this->_inners.at(i))=newPos;
            this->_sizes.at(this->_inners.at(i))=newSize;
            offset+=parts.at(i);
          }

          for(auto const&x:this->_inners)
            x->_setSize(this->_sizes.at(x));
        }
    };

  class Grid: public Element{
    public:
      Grid(
          std::vector<std::vector<Element*>>const&elements,
          std::vector<Primitive*>const&prims,
          Spacing const&spacingX = LEFT,
          Spacing const&spacingY = LEFT,
          glm::vec2 const&minSize = glm::vec2(0)):Element(GRID,prims,minSize),_inners(elements){
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
          for(auto const&y:x){
            this->_positions[y] = glm::vec2(0.f);
            this->_sizes[y] = glm::vec2(0.f);
          }
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

          for(size_t i=0;i<2;++i)
            switch(this->_spacings[i]){
              case LEFT:
                if(newSize[i]<this->_minSize[i])
                  rcSize[1-i].back() += this->_minSize[i]-newSize[i];
                break;
              case MIDDLE:
                if(newSize[i]<this->_minSize[i]){
                  float first = rcSize[1-i].at(0);
                  float last  = rcSize[1-i].back();
                  float toAdd = this->_minSize[i]-newSize[i];
                  if(first+toAdd<last){
                    rcSize[1-i].at(0)+=toAdd;
                  }else if(last+toAdd<first){
                    rcSize[1-i].back()+=toAdd;
                  }else{
                    rcSize[1-i].at(0) = (first+last+toAdd)/2;
                    rcSize[1-i].back() = (first+last+toAdd)/2;
                  }
                }
                break;
              case RIGHT:
                if(newSize[i]<this->_minSize[i])
                  rcSize[1-i].at(0) += this->_minSize[i]-newSize[i];
                break;
              case EQUAL:
                newSize[i] = largestPart[i]*this->_gridSize[i];
                if(newSize[i]<this->_minSize[i])
                  for(auto&x:rcSize[1-i])
                    x = this->_minSize[i]/this->_gridSize[i];
                else
                  for(auto&x:rcSize[1-i])
                    x = largestPart[i];
                break;
            }
          this->_size = glm::max(newSize,this->_minSize);
          glm::vec2 o = glm::vec2(0.f);
          for(size_t j=0;j<this->_gridSize.y;++j){
            o.x = 0.f;
            for(size_t i=0;i<this->_gridSize.x;++i){
              this->_positions.at(this->_inners.at(j).at(i))=o;
              this->_sizes.at(this->_inners.at(j).at(i)) = glm::vec2(rcSize[1].at(i),rcSize[0].at(j));
              o.x+=rcSize[1][i];
            }
            o.y+=rcSize[0][j];
          }

          for(auto const&x:this->_inners)
            for(auto const&y:x)
              y->_setSize(this->_sizes.at(y));

          this->_changedGuts = false;
        }
        return this->_size;
      }
      virtual void addToNode(std::shared_ptr<Draw2D>const&draw2D,size_t node)override{
        this->Element::addToNode(draw2D,node);
        for(auto const&x:this->_inners)
          for(auto const&y:x)
            y->addToNode(draw2D,node);
      }
    protected:
      glm::uvec2 _gridSize;
      std::vector<std::vector<Element*>>_inners;
      Spacing _spacings[2];
      std::map<Element const*,glm::vec2>_positions;
      std::map<Element const*,glm::vec2>_sizes;
      virtual glm::vec2 _getPositionOf(Element*e)override{
        assert(this!=nullptr);
        assert(this->_positions.count(e)!=0);
        return this->getPosition()+this->_positions.at(e);
      }
      virtual void _setSize(glm::vec2 const&ns)override{
        glm::vec2 newSize = ns;
        if(newSize==this->_size)return;
        std::vector<std::vector<glm::vec2>>parts;
        std::vector<float>rcSize[2];
        rcSize[0].resize(this->_gridSize[1],0.f);
        rcSize[1].resize(this->_gridSize[0],0.f);
        for(size_t j=0;j<this->_gridSize.y;++j){
          auto row = std::vector<glm::vec2>();
          for(size_t i=0;i<this->_gridSize.x;++i)
            row.push_back(this->_sizes.at(this->_inners.at(j).at(i)));
          parts.push_back(row);
        }
        for(size_t j=0;j<this->_gridSize.y;++j){
          for(size_t i=0;i<this->_gridSize.x;++i){
            auto is = parts.at(j).at(i);
            rcSize[0][j] = glm::max(rcSize[0][j],is[1]);
            rcSize[1][i] = glm::max(rcSize[1][i],is[0]);
          }
        }

        for(size_t i=0;i<2;++i)
          switch(this->_spacings[i]){
            case LEFT:
              if(newSize[i]>this->_size[i])
                rcSize[1-i].back() += newSize[i]-this->_size[i];
              break;
            case MIDDLE:
              if(newSize[i]>this->_size[i]){
                float first = rcSize[1-i].at(0);
                float last  = rcSize[1-i].back();
                float toAdd = newSize[i]-this->_size[i];
                if(first+toAdd<last){
                  rcSize[1-i].at(0)+=toAdd;
                }else if(last+toAdd<first){
                  rcSize[1-i].back()+=toAdd;
                }else{
                  rcSize[1-i].at(0) = (first+last+toAdd)/2;
                  rcSize[1-i].back() = (first+last+toAdd)/2;
                }
              }
              break;
            case RIGHT:
              if(newSize[i]>this->_size[i])
                rcSize[1-i].at(0) += newSize[i]-this->_size[i];
              break;
            case EQUAL:
              if(newSize[i]>this->_size[i])
                for(auto&x:rcSize[1-i])
                  x = newSize[i]/this->_gridSize[i];
              break;
          }
        this->_size = newSize;

        glm::vec2 o = glm::vec2(0.f);
        for(size_t j=0;j<this->_gridSize.y;++j){
          o.x = 0.f;
          for(size_t i=0;i<this->_gridSize.x;++i){
            this->_positions.at(this->_inners.at(j).at(i))=o;
            this->_sizes.at(this->_inners.at(j).at(i)) = glm::vec2(rcSize[1].at(i),rcSize[0].at(j));
            o.x+=rcSize[1][i];
          }
          o.y+=rcSize[0][j];
        }

        for(auto const&x:this->_inners)
          for(auto const&y:x)
            y->_setSize(this->_sizes.at(y));

      }
  };

  class Rectangle: public Element{
    public:
      Rectangle(std::vector<Primitive*>const&prims = {},glm::vec2 const&minSize = glm::vec2(0.f)):Element(RECTANGLE,prims,minSize){
        assert(this!=nullptr);
        this->_size = minSize;
      }
      Rectangle(float x=0,float y=0,std::vector<Primitive*>const&prims = {}):Rectangle(prims,glm::vec2(x,y)){}
      virtual ~Rectangle(){}
      virtual glm::vec2 getSize()override{
        assert(this!=nullptr);
        return this->_size;
      }
      virtual void addToNode(std::shared_ptr<Draw2D>const&draw2D,size_t node)override{
        this->Element::addToNode(draw2D,node);
      }
    protected:
      virtual glm::vec2 _getPositionOf(Element*)override{
        return glm::vec2(0.f);
      }
      virtual void _setSize(glm::vec2 const&newSize)override{
        this->_size = newSize;
      }
  };

  std::vector<Element*>repear1D(size_t N,std::function<Element*(size_t)>const&fce){
    std::vector<Element*>result;
    for(size_t i=0;i<N;++i)
      result.push_back(fce(i));
    return result;
  }
  std::vector<std::vector<Element*>>repear2D(size_t X,size_t Y,std::function<Element*(size_t,size_t)>const&fce){
    std::vector<std::vector<Element*>>result;
    for(size_t y=0;y<Y;++y){
      auto row = std::vector<Element*>();
      for(size_t x=0;x<X;++x)row.push_back(fce(x,y));
      result.push_back(row);
    }
    return result;
  }

}


void Function::create(){
  this->node = this->draw2D->createNode();
  using namespace ui;

#if 1
  auto root = new Split<1>({
      new Rectangle(0,this->lineWidth,{new Line(0,.5,1,.5,this->lineWidth,this->lineColor)}),//top line
      new Split<0>({
        new Rectangle(this->lineWidth,0,{new Line(.5,0,.5,1,this->lineWidth,this->lineColor)}),//left line
        new Split<1>({
          new Split<1>({
            new Rectangle(0,this->captionMargin),
            new Split<0>({
              new Rectangle(this->captionMargin,0),
              new Rectangle(this->captionFontSize*this->functionName.length(),this->captionFontSize*2,{new Text(this->functionName,this->captionFontSize,this->captionColor)}),
              new Rectangle(this->captionMargin,0),
              }),
            new Rectangle(0,this->captionMargin),
            },{new Triangle(0,0,1,0,1,1,this->captionBackgrounColor),new Triangle(0,0,1,1,0,1,this->captionBackgrounColor)}),
          new Rectangle(0,this->lineWidth,{new Line(0,.5,1,.5,this->lineWidth,this->lineColor)}),//caption line
          new Split<0>({
            new Rectangle(this->margin,0),
            new Split<1>({
              new Rectangle(0,this->margin),
              new Split<0>({
                new Split<1>(//inputs
                  repear1D(this->inputNames.size(),[this](size_t i)->Element*{
                    return new Rectangle(this->fontSize*this->inputNames[i].length(),this->fontSize*2,{new Text(this->inputNames[i],this->fontSize,this->captionColor)});
                    })
                  ),
                new Rectangle(this->inputOutputDistance,0),
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

