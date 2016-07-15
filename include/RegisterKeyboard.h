#pragma once

#include<iostream>

namespace ge{
  namespace de{
    class Kernel;
  }
}

namespace keyboard{
  std::string keyName(int key);
  void registerKeyboard(ge::de::Kernel*kernel);
}
