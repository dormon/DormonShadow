#pragma once

#include<iostream>

namespace ge{
  namespace de{
    class Kernel;
  }
}

namespace mouse{
  std::string buttonName(int key);
  std::string fullButtonName(int key);
  void registerMouse(ge::de::Kernel*kernel);
}

