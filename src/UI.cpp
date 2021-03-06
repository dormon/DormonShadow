#include<UI.h>

using namespace ui;

Element::Element(Type const&t,std::vector<NewData>const&prims,glm::vec2 const&minSize):type(t),_minSize(minSize){
  for(auto const&x:prims)
    this->data.addValue(x);
}

Element::~Element(){
}


glm::vec2 Element::getPosition(){
  assert(this!=nullptr);
  if(!this->_parent)return glm::vec2(0.f);
  //if(this->_changedGuts)
  this->_position = this->_parent->_getPositionOf(this);
  return this->_position;
}


void Element::_signalParents(){
  Element*p=this->_parent;
  while(p){
    p->_changedGuts = true;
    p=p->_parent;
  }
}







Split::Split(
    size_t direction,
    std::vector<Element*>const&elements,
    std::vector<NewData>const&prims,
    Spacing const&spacing,
    glm::vec2 const&minSize):Element((Type)(SPLITX+direction),prims,minSize),_direction(direction),_inners(elements),_spacing(spacing){
  assert(this!=nullptr);
  assert(elements.size()>0);
  if(this->_direction==1)std::reverse(this->_inners.begin(),this->_inners.end());
  for(auto const&x:elements)x->_parent = this;
  this->_signalParents();
  for(auto const&x:elements){
    this->_positions[x] = glm::vec2(0.f);
    this->_sizes[x] = glm::vec2(0.f);
  }
}

Split::~Split(){
  for(auto const&x:this->_inners)
    delete x;
}

glm::vec2 Split::getSize(){
  assert(this!=nullptr);
  if(this->_changedGuts){
    glm::vec2 newSize = glm::vec2(0.f);
    newSize[1-this->_direction] = this->_minSize[1-this->_direction];
    std::vector<float>parts;
    float largestPart = 0;
    for(auto const&x:this->_inners){
      auto is = x->getSize();
      newSize[1-this->_direction] = glm::max(is[1-this->_direction],newSize[1-this->_direction]);
      newSize[this->_direction]+=is[this->_direction];
      largestPart = glm::max(largestPart,is[this->_direction]);
      parts.push_back(is[this->_direction]);
    }
    switch(this->_spacing){
      case LEFT:
        if(newSize[this->_direction]<this->_minSize[this->_direction])
          parts.back() += this->_minSize[this->_direction]-newSize[this->_direction];
        break;
      case MIDDLE:
        if(newSize[this->_direction]<this->_minSize[this->_direction]){
          float first = parts.at(0);
          float last  = parts.back();
          float toAdd = this->_minSize[this->_direction]-newSize[this->_direction];
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
        if(newSize[this->_direction]<this->_minSize[this->_direction])
          parts.at(0) += this->_minSize[this->_direction]-newSize[this->_direction];
        break;
      case EQUAL:
        newSize[this->_direction] = largestPart*this->_inners.size();
        if(newSize[this->_direction]<this->_minSize[this->_direction])
          for(auto&x:parts)
            x = this->_minSize[this->_direction]/this->_inners.size();
        else
          for(auto&x:parts)
            x = largestPart;
        break;
    }
    newSize = glm::max(newSize,this->_minSize);
    this->_size = newSize;

    float offset = 0;
    for(size_t i=0;i<this->_inners.size();++i){
      assert(this->_positions.count(this->_inners.at(i))!=0);
      glm::vec2 newPos = glm::vec2(0.f);
      newPos[this->_direction] = offset;
      newSize[this->_direction] = parts.at(i);
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

glm::uvec2 Split::getDimensions()const{
  if(this->_direction==0)
    return glm::uvec2(this->_inners.size(),1);
  else
    return glm::uvec2(1,this->_inners.size());
}

void Split::visitor(void(*fce)(Element*,void*),void*data){
  fce(this,data);
  for(auto const&x:this->_inners)
    x->visitor(fce,data);
}

glm::vec2 Split::_getPositionOf(Element*e){
  assert(this!=nullptr);
  assert(this->_positions.count(e)!=0);
  return this->getPosition()+this->_positions.at(e);
}

void Split::_setSize(glm::vec2 const&ns){
  glm::vec2 newSize = ns;
  if(newSize==this->_size)return;
  assert(this!=nullptr);
  std::vector<float>parts;
  for(auto const&x:this->_inners)
    parts.push_back(this->_sizes.at(x)[this->_direction]);
  assert(newSize[this->_direction]>=this->_minSize[this->_direction]);
  switch(this->_spacing){
    case LEFT:
      if(newSize[this->_direction]>this->_size[this->_direction])
        parts.back() += newSize[this->_direction]-this->_size[this->_direction];
      break;
    case MIDDLE:
      if(newSize[this->_direction]>this->_size[this->_direction]){
        float first = parts.at(0);
        float last  = parts.back();
        float toAdd = newSize[this->_direction]-this->_size[this->_direction];
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
      if(newSize[this->_direction]>this->_size[this->_direction])
        parts.at(0) += newSize[this->_direction]-this->_size[this->_direction];
      break;
    case EQUAL:
      for(auto&x:parts)
        x = newSize[this->_direction]/this->_inners.size();
      break;
  }
  this->_size = newSize;

  float offset = 0;
  for(size_t i=0;i<this->_inners.size();++i){
    assert(this->_positions.count(this->_inners.at(i))!=0);
    glm::vec2 newPos = glm::vec2(0.f);
    newPos[this->_direction] = offset;
    newSize[this->_direction] = parts.at(i);
    this->_positions.at(this->_inners.at(i))=newPos;
    this->_sizes.at(this->_inners.at(i))=newSize;
    offset+=parts.at(i);
  }

  for(auto const&x:this->_inners)
    x->_setSize(this->_sizes.at(x));
}






Grid::Grid(
    std::vector<std::vector<Element*>>const&elements,
    std::vector<NewData>const&prims,
    Spacing const&spacingX,
    Spacing const&spacingY,
    glm::vec2 const&minSize):Element(GRID,prims,minSize),_inners(elements){
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

Grid::~Grid(){
  for(auto const&x:this->_inners)
    for(auto const&y:x)
      delete y;
}

glm::vec2 Grid::getSize(){
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

glm::uvec2 Grid::getDimensions()const{
  return glm::uvec2(this->_gridSize);
}

void Grid::visitor(void(*fce)(Element*,void*),void*data){
  fce(this,data);
  for(auto const&x:this->_inners)
    for(auto const&y:x)
      y->visitor(fce,data);
}

glm::vec2 Grid::_getPositionOf(Element*e){
  assert(this!=nullptr);
  assert(this->_positions.count(e)!=0);
  return this->getPosition()+this->_positions.at(e);
}

void Grid::_setSize(glm::vec2 const&ns){
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

Rectangle::Rectangle(std::vector<NewData>const&prims,glm::vec2 const&minSize):Element(RECTANGLE,prims,minSize){
  assert(this!=nullptr);
  this->_size = minSize;
}

Rectangle::Rectangle(float x,float y,std::vector<NewData>const&prims):Rectangle(prims,glm::vec2(x,y)){}

Rectangle::~Rectangle(){}

glm::vec2 Rectangle::getSize(){
  assert(this!=nullptr);
  return this->_size;
}

glm::uvec2 Rectangle::getDimensions()const{
  return glm::uvec2(0,0);
}

void Rectangle::visitor(void(*fce)(Element*,void*),void*data){
  fce(this,data);
}

glm::vec2 Rectangle::_getPositionOf(Element*){
  return glm::vec2(0.f);
}

void Rectangle::_setSize(glm::vec2 const&newSize){
  this->_size = newSize;
}

std::vector<Element*>ui::repear1D(size_t N,std::function<Element*(size_t)>const&fce){
  std::vector<Element*>result;
  for(size_t i=0;i<N;++i)
    result.push_back(fce(i));
  return result;
}

std::vector<std::vector<Element*>>ui::repear2D(size_t X,size_t Y,std::function<Element*(size_t,size_t)>const&fce){
  std::vector<std::vector<Element*>>result;
  for(size_t y=0;y<Y;++y){
    auto row = std::vector<Element*>();
    for(size_t x=0;x<X;++x)row.push_back(fce(x,y));
    result.push_back(row);
  }
  return result;
}

