#pragma once

#include<geDE/Keyword.h>
#include<geGL/geGL.h>
#include<geDE/Statement.h>

namespace ge{
  namespace de{
    class Kernel;
    template<>inline std::string keyword<ge::gl::Program          >(){return"Program"          ;}
    template<>inline std::string keyword<ge::gl::Shader           >(){return"Shader"           ;}
    template<>inline std::string keyword<ge::gl::VertexArray      >(){return"VertexArray"      ;}
    template<>inline std::string keyword<ge::gl::Texture          >(){return"Texture"          ;}
    template<>inline std::string keyword<ge::gl::Framebuffer      >(){return"Framebuffer"      ;}
    template<>inline std::string keyword<ge::gl::Renderbuffer     >(){return"Renderbuffer"     ;}
    template<>inline std::string keyword<ge::gl::Buffer           >(){return"Buffer"           ;}
    template<>inline std::string keyword<ge::gl::Sampler          >(){return"Sampler"          ;}
    template<>inline std::string keyword<ge::gl::ProgramPipeline  >(){return"ProgramPipeline"  ;}
    template<>inline std::string keyword<ge::gl::AsynchronousQuery>(){return"AsynchronousQuery";}
    template<>inline std::string keyword<std::shared_ptr<ge::gl::Program          >>(){return"SharedProgram"          ;}
    template<>inline std::string keyword<std::shared_ptr<ge::gl::Shader           >>(){return"SharedShader"           ;}
    template<>inline std::string keyword<std::shared_ptr<ge::gl::VertexArray      >>(){return"SharedVertexArray"      ;}
    template<>inline std::string keyword<std::shared_ptr<ge::gl::Texture          >>(){return"SharedTexture"          ;}
    template<>inline std::string keyword<std::shared_ptr<ge::gl::Framebuffer      >>(){return"SharedFramebuffer"      ;}
    template<>inline std::string keyword<std::shared_ptr<ge::gl::Renderbuffer     >>(){return"SharedRenderbuffer"     ;}
    template<>inline std::string keyword<std::shared_ptr<ge::gl::Buffer           >>(){return"SharedBuffer"           ;}
    template<>inline std::string keyword<std::shared_ptr<ge::gl::Sampler          >>(){return"SharedSampler"          ;}
    template<>inline std::string keyword<std::shared_ptr<ge::gl::ProgramPipeline  >>(){return"SharedProgramPipeline"  ;}
    template<>inline std::string keyword<std::shared_ptr<ge::gl::AsynchronousQuery>>(){return"SharedAsynchronousQuery";}
    template<>inline std::string keyword<ge::de::Statement>(){return"Statement";}
    template<>inline std::string keyword<ge::de::Statement*>(){return"Statement*";}
    template<>inline std::string keyword<std::shared_ptr<ge::de::Statement>>(){return"SharedStatement";}
  }
}

void registerPlugin(ge::de::Kernel*kernel);
