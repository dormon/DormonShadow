#include<Functions.h>

#include<geDE/Kernel.h>
#include<geDE/RegisterBasicFunction.h>
#include<geCore/Text.h>
#include<geGL/geGL.h>

using namespace ge::de;

std::string shaderSourceLoader(
    std::string const&version,
    std::string const&defines,
    std::string const&dir,
    std::string const&fileNames){
  std::cout<<"("<<version<<"#"<<defines<<"#"<<dir<<"#"<<fileNames<<")"<<std::endl;
  std::stringstream ss;
  ss<<version;
  ss<<defines;
  size_t pos=0;
  do{
    size_t newPos = fileNames.find("\n",pos);
    if(newPos == std::string::npos){
      auto fileName = fileNames.substr(pos);
      ss<<ge::core::loadTextFile(dir+fileName);
      break;
    }
    auto fileName = fileNames.substr(pos,newPos-pos);
    ss<<ge::core::loadTextFile(dir+fileName);
    pos = newPos+1;
  }while(true);
  return ss.str();
}

template<GLenum TYPE>
std::shared_ptr<ge::gl::Shader>createShader(std::string source){
  std::cout<<"createShader: "<<TYPE<<std::endl;
  return std::make_shared<ge::gl::Shader>(TYPE,source);
}

template<typename...SHADERS>
std::shared_ptr<ge::gl::Program>createProgram(
    SHADERS...shaders){
  std::cout<<"createProgram"<<std::endl;
  return std::make_shared<ge::gl::Program>(shaders...);
}

template<typename T>inline T*sharedPointerToPointer(std::shared_ptr<T>const&p){return &*p;}
template<typename T,size_t N>inline T const*arrayToPointer(const T (&p)[N]){return &p[0];}
int32_t neco(){
  return 10;
}

template<typename...BUFFERS>
void framebufferDrawBuffers(ge::gl::Framebuffer*framebuffer,BUFFERS...buffs){
  GLenum const buffers[] = {buffs...};
  framebuffer->drawBuffers(sizeof(buffers)/sizeof(buffers[0]),buffers);
}

std::shared_ptr<ge::gl::Texture>createTexture(
    GLenum  target        ,
    GLenum  internalFormat,
    GLsizei levels        ,
    GLsizei width         ,
    GLsizei height        ,
    GLsizei depth         ){
  auto result = std::make_shared<ge::gl::Texture>();
  result->create(target,internalFormat,levels,width,height,depth);
  return result;
}




void registerPlugin(Kernel*kernel){
  (void)kernel;
  kernel->addAtomicClass<ge::gl::Program          >();
  kernel->addAtomicClass<ge::gl::Shader           >();
  kernel->addAtomicClass<ge::gl::VertexArray      >();
  kernel->addAtomicClass<ge::gl::Texture          >();
  kernel->addAtomicClass<ge::gl::Framebuffer      >();
  kernel->addAtomicClass<ge::gl::Renderbuffer     >();
  kernel->addAtomicClass<ge::gl::Buffer           >();
  kernel->addAtomicClass<ge::gl::Sampler          >();
  kernel->addAtomicClass<ge::gl::ProgramPipeline  >();
  kernel->addAtomicClass<ge::gl::AsynchronousQuery>();
  kernel->typeRegister->addType<ge::gl::Program          *>();
  kernel->typeRegister->addType<ge::gl::Shader           *>();
  kernel->typeRegister->addType<ge::gl::VertexArray      *>();
  kernel->typeRegister->addType<ge::gl::Texture          *>();
  kernel->typeRegister->addType<ge::gl::Framebuffer      *>();
  kernel->typeRegister->addType<ge::gl::Renderbuffer     *>();
  kernel->typeRegister->addType<ge::gl::Buffer           *>();
  kernel->typeRegister->addType<ge::gl::Sampler          *>();
  kernel->typeRegister->addType<ge::gl::ProgramPipeline  *>();
  kernel->typeRegister->addType<ge::gl::AsynchronousQuery*>();
  kernel->addAtomicClass<std::shared_ptr<ge::gl::Program          >>();
  kernel->addAtomicClass<std::shared_ptr<ge::gl::Shader           >>();
  kernel->addAtomicClass<std::shared_ptr<ge::gl::VertexArray      >>();
  kernel->addAtomicClass<std::shared_ptr<ge::gl::Texture          >>();
  kernel->addAtomicClass<std::shared_ptr<ge::gl::Framebuffer      >>();
  kernel->addAtomicClass<std::shared_ptr<ge::gl::Renderbuffer     >>();
  kernel->addAtomicClass<std::shared_ptr<ge::gl::Buffer           >>();
  kernel->addAtomicClass<std::shared_ptr<ge::gl::Sampler          >>();
  kernel->addAtomicClass<std::shared_ptr<ge::gl::ProgramPipeline  >>();
  kernel->addAtomicClass<std::shared_ptr<ge::gl::AsynchronousQuery>>();

  kernel->addFunction("cast<SharedProgram,Program*>"        ,{"sharedProgram"    ,"program*"    },sharedPointerToPointer<ge::gl::Program    >);
  kernel->addFunction("cast<SharedVertexArray,VertexArray*>",{"sharedVertexArray","vertexArray*"},sharedPointerToPointer<ge::gl::VertexArray>);
  kernel->addFunction("cast<SharedFramebuffer,Framebuffer*>",{"sharedFramebuffer","framebuffer*"},sharedPointerToPointer<ge::gl::Framebuffer>);
  kernel->addFunction("cast<SharedTexture,Texture*>"        ,{"sharedTexture"    ,"texture*"    },sharedPointerToPointer<ge::gl::Texture    >);

  kernel->addFunction("shaderSourceLoader"    ,{"version","defines","dir","fileNames","source"},shaderSourceLoader);
  kernel->addFunction("createVertexShader"    ,{"source","sharedShader"},createShader<GL_VERTEX_SHADER>);
  kernel->addFunction("createFragmentShader"  ,{"source","sharedShader"},createShader<GL_FRAGMENT_SHADER>);
  kernel->addFunction("createGeometryShader"  ,{"source","sharedShader"},createShader<GL_GEOMETRY_SHADER>);
  kernel->addFunction("createControlShader"   ,{"source","sharedShader"},createShader<GL_TESS_CONTROL_SHADER>);
  kernel->addFunction("createEvaluationShader",{"source","sharedShader"},createShader<GL_TESS_EVALUATION_SHADER>);
  kernel->addFunction("createComputeShader"   ,{"source","sharedShader"},createShader<GL_COMPUTE_SHADER>);

  using SS = std::shared_ptr<ge::gl::Shader>const&;
  kernel->addFunction("createProgram1",{"shader0"                                        ,"sharedProgram"},createProgram<SS>);
  kernel->addFunction("createProgram2",{"shader0","shader1"                              ,"sharedProgram"},createProgram<SS,SS>);
  kernel->addFunction("createProgram3",{"shader0","shader1","shader2"                    ,"sharedProgram"},createProgram<SS,SS,SS>);
  kernel->addFunction("createProgram4",{"shader0","shader1","shader2","shader3"          ,"sharedProgram"},createProgram<SS,SS,SS,SS>);
  kernel->addFunction("createProgram5",{"shader0","shader1","shader2","shader3","shader4","sharedProgram"},createProgram<SS,SS,SS,SS,SS>);


  kernel->addFunction("cast<f32[3],f32*>"     ,{"f32[3]","f32*"},arrayToPointer<float,3>);
  kernel->addFunction("cast<f32[16],f32*>"    ,{"f32[16]","f32*"},arrayToPointer<float,16>);

  kernel->addFunction("Program::use"          ,{"program"                                   },&ge::gl::Program::use);
  kernel->addFunction("Program::set3fv"       ,{"program","name","value","count"            },&ge::gl::Program::set3fv);
  kernel->addFunction("Program::setMatrix4fv" ,{"program","name","value","count","transpose"},&ge::gl::Program::setMatrix4fv);

  kernel->addFunction("VertexArray::bind"     ,{"vertexArray"},&ge::gl::VertexArray::bind  );
  kernel->addFunction("VertexArray::unbind"   ,{"vertexArray"},&ge::gl::VertexArray::unbind);
  kernel->addFunction("VertexArray::addAttrib",{"vertexArray","sharedBuffer","index","nofComponents","type","stride","offset","normalized","divisor","ptrType"},&ge::gl::VertexArray::addAttrib);


  kernel->addFunction("createTexture",{"target","internalFormal","levels","width","height","depth"},&createTexture);

  kernel->addFunction("Framebuffer::attachTexture",{"Framebuffer","attachment","texture","level","layer"},&ge::gl::Framebuffer::attachTexture);
  kernel->addFunction("Framebuffer::attachRenderbuffer",{"Framebuffer","attachment","renderbuffer"},&ge::gl::Framebuffer::attachRenderbuffer);
  kernel->addFunction("Framebuffer::bind"  ,{"Framebuffer","target"},&ge::gl::Framebuffer::bind  );
  kernel->addFunction("Framebuffer::unbind",{"Framebuffer","target"},&ge::gl::Framebuffer::unbind);
  kernel->addFunction("Framebuffer::drawBuffers1",{"Framebuffer","buffer0"                                        },&framebufferDrawBuffers<GLenum>);
  kernel->addFunction("Framebuffer::drawBuffers2",{"Framebuffer","buffer0","buffer1"                              },&framebufferDrawBuffers<GLenum,GLenum>);
  kernel->addFunction("Framebuffer::drawBuffers3",{"Framebuffer","buffer0","buffer1","buffer2"                    },&framebufferDrawBuffers<GLenum,GLenum,GLenum>);
  kernel->addFunction("Framebuffer::drawBuffers4",{"Framebuffer","buffer0","buffer1","buffer2","buffer3"          },&framebufferDrawBuffers<GLenum,GLenum,GLenum,GLenum>);
  kernel->addFunction("Framebuffer::drawBuffers5",{"Framebuffer","buffer0","buffer1","buffer2","buffer3","buffer4"},&framebufferDrawBuffers<GLenum,GLenum,GLenum,GLenum,GLenum>);

  kernel->addFunction("Texture::bind"  ,{"Texture","unit"},&ge::gl::Texture::bind  );
  kernel->addFunction("Texture::unbind",{"Texture","unit"},&ge::gl::Texture::unbind);
}
