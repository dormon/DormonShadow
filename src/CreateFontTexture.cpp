#include<CreateFontTexture.h>
#include<geGL/geGL.h>
#include<Font.h>

std::shared_ptr<ge::gl::Texture>createFontTexture(){
  const uint32_t w=ge::res::font::width;
  const uint32_t h=ge::res::font::height;
  uint8_t bytes[w*h];
  auto result = std::make_shared<ge::gl::Texture>(GL_TEXTURE_2D,GL_R8,0,w,h);
  for(uint32_t i=0;i<w*h/8;++i)
    for(size_t k=0;k<8;++k)
      bytes[i*8+k]=255*((ge::res::font::data[i]>>k)&0x1);
  result->setData2D(bytes,GL_RED,GL_UNSIGNED_BYTE,0,0,0,w,h,w);
  result->generateMipmap();
  result->texParameteri(GL_TEXTURE_MIN_FILTER,GL_LINEAR_MIPMAP_LINEAR);
  result->texParameteri(GL_TEXTURE_MAG_FILTER,GL_LINEAR);
  result->texParameteri(GL_TEXTURE_WRAP_S,GL_CLAMP_TO_EDGE);
  result->texParameteri(GL_TEXTURE_WRAP_T,GL_CLAMP_TO_EDGE);
  return result;
}


