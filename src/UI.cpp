#include<UI.h>

using namespace ui;

glm::vec2 Element::getPosition(){
  assert(this!=nullptr);
  if(!this->_parent)return glm::vec2(0.f);
  //if(this->_changedGuts)
  this->_position = this->_parent->_getPositionOf(this);
  return this->_position;
}

