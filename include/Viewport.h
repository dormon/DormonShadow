#pragma once

#include<Layer.h>

class Viewport: public std::vector<Layer*>{
  public:
    virtual~Viewport();
};

inline Viewport::~Viewport(){
  assert(this!=nullptr);
  for(auto const&x:*this)
    delete x;
}
