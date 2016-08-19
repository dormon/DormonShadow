#pragma once

#include<map>
#include<functional>
#include<algorithm>
#include<glm/glm.hpp>
#include<glm/gtc/matrix_transform.hpp>
#include<glm/gtc/type_ptr.hpp>
#include<glm/gtc/matrix_access.hpp>

namespace ui{
  template<typename T>
    size_t getTypeId();
  class Data{
    public:
      Data(size_t id,void*data,void(*deleter)(void*) = nullptr):_id(id),_data(data),_deleter(deleter){}
      ~Data(){assert(this!=nullptr);assert(_deleter!=nullptr);this->_deleter(this->_data);}
      void*getData()const{assert(this!=nullptr);return this->_data;}
      size_t getId()const{assert(this!=nullptr);return this->_id;}
    protected:
      size_t _id;
      void*_data;
      void(*_deleter)(void*);
  };
  template<typename CLASS,typename...ARGS>
  Data*newData(ARGS...args){
    uint8_t*d = new uint8_t[sizeof(CLASS)];
    new(d)CLASS(args...);
    return new Data(getTypeId<CLASS>(),d,[](void*d){((CLASS*)d)->~CLASS();delete[](uint8_t*)d;});
  }

  class Split;
  class Grid;
  class Element{
    public:
      virtual ~Element();
      virtual glm::vec2 getSize() = 0;
      glm::vec2 getPosition();
      virtual glm::uvec2 getDimensions()const = 0;
      virtual void visitor(void(*fce)(Element*,void*),void*data = nullptr) = 0;
      std::vector<Data*>data;
    protected:
      enum Type{SPLITX,SPLITY,GRID,RECTANGLE,}type;
      Element(Type const&t,std::vector<Data*>const&prims = {},glm::vec2 const&minSize = glm::vec2(0.f));
      glm::vec2 _minSize;
      Element*_parent = nullptr;
      bool _changedGuts = true;
      glm::vec2 _size = glm::vec2(0.f);
      glm::vec2 _position = glm::vec2(0.f);
      void _signalParents();
      virtual glm::vec2 _getPositionOf(Element*e) = 0;
      virtual void _setSize(glm::vec2 const&newSize) = 0;
      friend class Split;
      friend class Grid;
  };

  enum Spacing{LEFT,MIDDLE,RIGHT,EQUAL,};

  class Split: public Element{
    public:
      Split(
          size_t direction,
          std::vector<Element*>const&elements,
          std::vector<Data*>const&prims = {},
          Spacing const&spacing = LEFT,
          glm::vec2 const&minSize = glm::vec2(0));
      virtual ~Split();
      virtual glm::vec2 getSize()override;
      virtual glm::uvec2 getDimensions()const override;
      virtual void visitor(void(*fce)(Element*,void*),void*data = nullptr)override;
    protected:
      size_t _direction = 0;
      std::vector<Element*>_inners;
      Spacing _spacing;
      std::map<Element const*,glm::vec2>_positions;
      std::map<Element const*,glm::vec2>_sizes;
      virtual glm::vec2 _getPositionOf(Element*e)override;
      virtual void _setSize(glm::vec2 const&ns)override;
   };

  class Grid: public Element{
    public:
      Grid(
          std::vector<std::vector<Element*>>const&elements,
          std::vector<Data*>const&prims,
          Spacing const&spacingX = LEFT,
          Spacing const&spacingY = LEFT,
          glm::vec2 const&minSize = glm::vec2(0));
      virtual ~Grid();
      virtual glm::vec2 getSize()override;
      virtual glm::uvec2 getDimensions()const override;
      virtual void visitor(void(*fce)(Element*,void*),void*data = nullptr)override;
    protected:
      glm::uvec2 _gridSize;
      std::vector<std::vector<Element*>>_inners;
      Spacing _spacings[2];
      std::map<Element const*,glm::vec2>_positions;
      std::map<Element const*,glm::vec2>_sizes;
      virtual glm::vec2 _getPositionOf(Element*e)override;
      virtual void _setSize(glm::vec2 const&ns)override;
  };

  class Rectangle: public Element{
    public:
      Rectangle(std::vector<Data*>const&prims = {},glm::vec2 const&minSize = glm::vec2(0.f));
      Rectangle(float x=0,float y=0,std::vector<Data*>const&prims = {});
      virtual ~Rectangle();
      virtual glm::vec2 getSize()override;
      virtual glm::uvec2 getDimensions()const override;
      virtual void visitor(void(*fce)(Element*,void*),void*data = nullptr)override;
    protected:
      virtual glm::vec2 _getPositionOf(Element*)override;
      virtual void _setSize(glm::vec2 const&newSize)override;
  };

  std::vector<Element*>repear1D(size_t N,std::function<Element*(size_t)>const&fce);
  std::vector<std::vector<Element*>>repear2D(size_t X,size_t Y,std::function<Element*(size_t,size_t)>const&fce);
}

