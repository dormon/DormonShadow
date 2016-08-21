#pragma once

#include<memory>

namespace ge{
  namespace gl{
    class Texture;
  }
}


std::shared_ptr<ge::gl::Texture>createFontTexture();
