#pragma once

#include<Layer.h>

class Viewport: public std::vector<std::shared_ptr<Layer>>{
  public:
    virtual~Viewport();
};

inline Viewport::~Viewport(){}
