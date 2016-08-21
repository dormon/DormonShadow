#pragma once

#include<vector>
#include<functional>
#include<cassert>
#include<AllContainer.h>

class Node: public std::vector<Node*>, public AllContainer{
  public:
    Node(Node*parent = nullptr);
    virtual ~Node();
    virtual void visitor(
        std::function<bool(Node*,void*)>const&atEnter = nullptr,
        std::function<void(Node*,void*)>const&atExit = nullptr,
        void*data = nullptr);
    Node*parent = nullptr;
};

inline Node::Node(Node*p):parent(p){}

inline Node::~Node(){
  assert(this!=nullptr);
  for(auto const&x:*this)
    delete x;
}

inline void Node::visitor(
    std::function<bool(Node*,void*)>const&atEnter,
    std::function<void(Node*,void*)>const&atExit,
    void*data){
  assert(this!=nullptr);
  bool doRest = true;
  if(atEnter)doRest = atEnter(this,data);
  if(doRest){
    for(auto const&x:*this){
      assert(x!=nullptr);
      x->visitor(atEnter,atExit,data);
    }
  }
  if(atExit)atExit(this,data);
}


