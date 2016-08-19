#pragma once

#include<Draw2D.h>
#include<algorithm>

namespace ui{
  class Split;
  class Grid;
  class Element{
    public:
      virtual ~Element();
      virtual glm::vec2 getSize() = 0;
      glm::vec2 getPosition();
      virtual glm::uvec2 getDimensions()const = 0;
      virtual void visitor(void(*fce)(Element*,void*),void*data = nullptr) = 0;
      std::vector<Primitive*>primitives;
    protected:
      enum Type{SPLITX,SPLITY,GRID,RECTANGLE,}type;
      Element(Type const&t,std::vector<Primitive*>const&prims = {},glm::vec2 const&minSize = glm::vec2(0.f));
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
          std::vector<Primitive*>const&prims = {},
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
          std::vector<Primitive*>const&prims,
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
      Rectangle(std::vector<Primitive*>const&prims = {},glm::vec2 const&minSize = glm::vec2(0.f));
      Rectangle(float x=0,float y=0,std::vector<Primitive*>const&prims = {});
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

