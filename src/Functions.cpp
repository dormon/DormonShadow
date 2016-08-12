#include<Functions.h>

#include<geDE/Kernel.h>
#include<geDE/RegisterBasicFunction.h>
#include<geCore/Text.h>
#include<geGL/geGL.h>

using namespace ge::de;

namespace ge{
  namespace de{
    GE_DE_ADD_KEYWORD(ge::gl::Program,"Program")
    GE_DE_ADD_KEYWORD(std::shared_ptr<ge::gl::Shader>,"SharedShader")
    GE_DE_ADD_KEYWORD(std::shared_ptr<ge::gl::Program>,"SharedProgram")
  }
}

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
  kernel->addAtomicClass<std::shared_ptr<ge::gl::Shader>>("SharedShader");
  kernel->addAtomicClass<std::shared_ptr<ge::gl::Program>>("SharedProgram");
  kernel->addAtomicClass<ge::gl::Program>("Program");
  kernel->typeRegister->addType<ge::gl::Program*>();


  kernel->addFunction({"shaderSourceLoader","source","version","defines","dir","fileNames"},shaderSourceLoader);
  kernel->addFunction({"createVertexShader"    ,"sharedShader","source"},createShader<GL_VERTEX_SHADER>);
  kernel->addFunction({"createFragmentShader"  ,"sharedShader","source"},createShader<GL_FRAGMENT_SHADER>);
  kernel->addFunction({"createGeometryShader"  ,"sharedShader","source"},createShader<GL_GEOMETRY_SHADER>);
  kernel->addFunction({"createControlShader"   ,"sharedShader","source"},createShader<GL_TESS_CONTROL_SHADER>);
  kernel->addFunction({"createEvaluationShader","sharedShader","source"},createShader<GL_TESS_EVALUATION_SHADER>);
  kernel->addFunction({"createComputeShader"   ,"sharedShader","source"},createShader<GL_COMPUTE_SHADER>);
  


  using SS = std::shared_ptr<ge::gl::Shader>const&;
  kernel->addFunction({"createProgram1","shaderProgram","shader0"},createProgram<SS>);
  kernel->addFunction({"createProgram2","shaderProgram","shader0","shader1"},createProgram<SS,SS>);
  kernel->addFunction({"createProgram3","shaderProgram","shader0","shader1","shader2"},createProgram<SS,SS,SS>);
  kernel->addFunction({"createProgram4","shaderProgram","shader0","shader1","shader2","shader3"},createProgram<SS,SS,SS,SS>);
  kernel->addFunction({"createProgram5","shaderProgram","shader0","shader1","shader2","shader3","shader4"},createProgram<SS,SS,SS,SS,SS>);


  kernel->addFunction({"sharedProgram2Program*","program*","sharedProgram"},sharedPointerToPointer<ge::gl::Program>);
  kernel->addFunction({"f32[3]2f32*","f32[3]","f32*"},arrayToPointer<float,3>);
  kernel->addFunction({"f32[16]2f32*","f32[16]","f32*"},arrayToPointer<float,16>);
  kernel->addFunction({"Program::use"},&ge::gl::Program::use);
  kernel->addFunction({"Program::set3fv"},&ge::gl::Program::set3fv);
  kernel->addFunction({"Program::setMatrix4fv"},&ge::gl::Program::setMatrix4fv);
  
}
