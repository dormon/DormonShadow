#pragma once

#include<Node.h>

class Viewport;
class Layer{
  public:
    Layer(std::shared_ptr<Node>const&root = nullptr);
    virtual ~Layer();
    std::shared_ptr<Node>root = nullptr;
};

inline Layer::Layer(std::shared_ptr<Node>const&r):root(r){}

inline Layer::~Layer(){}
