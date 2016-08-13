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

void registerPlugin(Kernel*kernel){
  (void)kernel;
  kernel->addAtomicClass<ge::gl::Program          >();
  kernel->addAtomicClass<ge::gl::Shader           >();
  kernel->addAtomicClass<ge::gl::VertexArray      >();
  //kernel->addAtomicClass<ge::gl::Texture          >();
  kernel->addAtomicClass<ge::gl::Framebuffer      >();
  //kernel->addAtomicClass<ge::gl::Renderbuffer     >();
  kernel->addAtomicClass<ge::gl::Buffer           >();
  kernel->addAtomicClass<ge::gl::Sampler          >();
  kernel->addAtomicClass<ge::gl::ProgramPipeline  >();
  //kernel->addAtomicClass<ge::gl::AsynchronousQuery>();
  kernel->typeRegister->addType<ge::gl::Program          *>();
  kernel->typeRegister->addType<ge::gl::Shader           *>();
  kernel->typeRegister->addType<ge::gl::VertexArray      *>();
  //kernel->typeRegister->addType<ge::gl::Texture          *>();
  kernel->typeRegister->addType<ge::gl::Framebuffer      *>();
  //kernel->typeRegister->addType<ge::gl::Renderbuffer     *>();
  kernel->typeRegister->addType<ge::gl::Buffer           *>();
  kernel->typeRegister->addType<ge::gl::Sampler          *>();
  kernel->typeRegister->addType<ge::gl::ProgramPipeline  *>();
  //kernel->typeRegister->addType<ge::gl::AsynchronousQuery*>();
  kernel->addAtomicClass<std::shared_ptr<ge::gl::Program          >>();
  kernel->addAtomicClass<std::shared_ptr<ge::gl::Shader           >>();
  kernel->addAtomicClass<std::shared_ptr<ge::gl::VertexArray      >>();
  //kernel->addAtomicClass<std::shared_ptr<ge::gl::Texture          >>();
  kernel->addAtomicClass<std::shared_ptr<ge::gl::Framebuffer      >>();
  //kernel->addAtomicClass<std::shared_ptr<ge::gl::Renderbuffer     >>();
  kernel->addAtomicClass<std::shared_ptr<ge::gl::Buffer           >>();
  kernel->addAtomicClass<std::shared_ptr<ge::gl::Sampler          >>();
  kernel->addAtomicClass<std::shared_ptr<ge::gl::ProgramPipeline  >>();
  //kernel->addAtomicClass<std::shared_ptr<ge::gl::AsynchronousQuery>>();

  kernel->addFunction("sharedProgram2Program*"        ,{"sharedProgram"    ,"program*"    },sharedPointerToPointer<ge::gl::Program    >);
  kernel->addFunction("sharedVertexArray2VertexArray*",{"sharedVertexArray","vertexArray*"},sharedPointerToPointer<ge::gl::VertexArray>);

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


  kernel->addFunction("f32[3]2f32*"           ,{"f32[3]","f32*"},arrayToPointer<float,3>);
  kernel->addFunction("f32[16]2f32*"          ,{"f32[16]","f32*"},arrayToPointer<float,16>);
  kernel->addFunction("Program::use"         ,{"program"                                   },&ge::gl::Program::use);
  kernel->addFunction("Program::set3fv"      ,{"program","name","value","count"            },&ge::gl::Program::set3fv);
  kernel->addFunction("Program::setMatrix4fv",{"program","name","value","count","transpose"},&ge::gl::Program::setMatrix4fv);
  kernel->addFunction("VertexArray::bind"  ,{"vertexArray"},&ge::gl::VertexArray::bind  );
  kernel->addFunction("VertexArray::unbind",{"vertexArray"},&ge::gl::VertexArray::unbind);
  
}
