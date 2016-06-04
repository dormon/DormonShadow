#include<limits>
#include<string>
#include<geGL/geGL.h>
#include<geDE/geDE.h>
#include<geDE/Kernel.h>
#include<geUtil/CopyArgumentManager2VariableRegister.h>
#include<geUtil/ArgumentManager/ArgumentManager.h>
#include<geAd/SDLWindow/SDLWindow.h>
#include<geCore/Text.h>

#include<AntTweakBar.h>

#include"../include/VariableRegisterManipulator.h"

#include<assimp/cimport.h>
#include<assimp/scene.h>
#include<assimp/postprocess.h>

struct Data{
  ge::de::Kernel kernel;
  std::shared_ptr<ge::gl::opengl::FunctionProvider> gl                  = nullptr;
  std::shared_ptr<ge::util::SDLEventProc>           mainLoop            = nullptr;
  std::shared_ptr<ge::util::SDLWindow>              window              = nullptr;
  std::shared_ptr<ge::gl::VertexArray>              emptyVAO            = nullptr;
  std::shared_ptr<VariableRegisterManipulator>      variableManipulator = nullptr;
  static void init(Data*data);
  static void deinit(Data*data);
  class IdleCallback: public ge::util::CallbackInterface{
    public:
      Data*data;
      IdleCallback(Data*data){this->data = data;}
      virtual void operator()()override;
      virtual ~IdleCallback(){}
  };
  class WindowEventCallback: public ge::util::EventCallbackInterface{
    public:
      Data*data;
      WindowEventCallback(Data*data){this->data = data;}
      virtual bool operator()(ge::util::EventDataPointer const&)override;
      virtual ~WindowEventCallback(){}
  };
  class WindowEventHandler: public ge::util::EventHandlerInterface{
    public:
      Data*data;
      WindowEventHandler(Data*data){this->data = data;}
      virtual bool operator()(ge::util::EventDataPointer const&)override;
      virtual ~WindowEventHandler(){}
  };
  std::shared_ptr<ge::de::Resource>model = nullptr;
  std::shared_ptr<ge::de::Function>prg = nullptr;
};

namespace ge{
  namespace de{
    template<>
      std::string TypeRegister::getTypeKeyword<aiScene const*>(){
        return "AssimpScene";
      }
    template<>
      std::string TypeRegister::getTypeKeyword<std::shared_ptr<ge::gl::Shader>>(){
        return "SharedShader";
      }
    template<>
      std::string TypeRegister::getTypeKeyword<std::shared_ptr<ge::gl::Program>>(){
        return "SharedProgram";
      }
  }
}

aiScene const*assimpLoader(std::string name){
  aiScene const* model = aiImportFile(name.c_str(),aiProcess_Triangulate|aiProcess_GenNormals|aiProcess_SortByPType);
  return model;
}

std::string concatenate(std::string a,std::string b){
  return a+b;
}


std::string shaderSourceLoader(
    std::string version,
    std::string defines,
    std::string dir,
    std::string fileNames){
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

std::shared_ptr<ge::gl::Shader>createVertexShader(std::string source){
  return std::make_shared<ge::gl::Shader>(GL_VERTEX_SHADER,source);
}

std::shared_ptr<ge::gl::Shader>createFragmentShader(std::string source){
  return std::make_shared<ge::gl::Shader>(GL_FRAGMENT_SHADER,source);
}

std::shared_ptr<ge::gl::Program>createProgram2(
    std::shared_ptr<ge::gl::Shader>s0,
    std::shared_ptr<ge::gl::Shader>s1){
  return std::make_shared<ge::gl::Program>(s0,s1);
}


int main(int argc,char*argv[]){
  //std::cout<<shaderSourceLoader("#version 450\n","","shaders/","vertex.vp\nfragment.fp");
  //return 0;
  Data data;
  /*
  data.typeRegister     = std::make_shared<ge::de::TypeRegister>();
  data.nameRegister     = std::make_shared<ge::de::NameRegister>();
  data.functionRegister = std::make_shared<ge::de::FunctionRegister>(data.typeRegister,data.nameRegister);
  ge::de::registerStdFunctions(data.functionRegister);
  data.variableRegister = std::make_shared<ge::de::VariableRegister>("*");
  */
  auto argm = std::make_shared<ge::util::ArgumentManager>(argc-1,argv+1);
  ge::util::copyArgumentManager2VariableRegister(data.kernel.variableRegister,*argm,data.kernel.functionRegister);
  std::cout<<data.kernel.variableRegister->toStr(0,data.kernel.typeRegister)<<std::endl;

  data.mainLoop = std::make_shared<ge::util::SDLEventProc>(true);
  data.mainLoop->setIdleCallback(std::make_shared<Data::IdleCallback>(&data));
  data.mainLoop->setEventHandler(std::make_shared<Data::WindowEventHandler>(&data));

  data.window   = std::make_shared<ge::util::SDLWindow>();
  data.window->createContext("rendering",450u,ge::util::SDLWindow::CORE,ge::util::SDLWindow::DEBUG);
  data.window->setEventCallback(SDL_WINDOWEVENT,std::make_shared<Data::WindowEventCallback>(&data));
  data.mainLoop->addWindow("primaryWindow",data.window);
  data.init(&data);
  (*data.mainLoop)();
  data.deinit(&data);

  return EXIT_SUCCESS;
}

void Data::IdleCallback::operator()(){
  this->data->gl->glClear(GL_COLOR_BUFFER_BIT|GL_DEPTH_BUFFER_BIT);

  this->data->emptyVAO->bind();
  (*this->data->prg)();
  (*(std::shared_ptr<ge::gl::Program>*)*this->data->prg->getOutputData())->use();
  this->data->gl->glDrawArrays(GL_TRIANGLE_STRIP,0,3);
  this->data->emptyVAO->unbind();

  TwDraw();
  this->data->window->swap();
}

bool Data::WindowEventCallback::operator()(ge::util::EventDataPointer const&event){
  auto sdlEventData = (ge::util::SDLEventData const*)(event);
  if(sdlEventData->event.window.event==SDL_WINDOWEVENT_CLOSE){
    this->data->mainLoop->removeWindow("primaryWindow");
    return true;
  }
  return false;
}


bool Data::WindowEventHandler::operator()(ge::util::EventDataPointer const&event){
  auto sdlEventData = (ge::util::SDLEventData const*)(event);
  bool handledByAnt = TwEventSDL(&sdlEventData->event,SDL_MAJOR_VERSION,SDL_MINOR_VERSION);
  if(handledByAnt)return true;
  return false;
}

void Data::init(Data*data){
  data->window->makeCurrent("rendering");

  ge::gl::init(std::make_shared<ge::gl::opengl::DefaultLoader>((ge::gl::opengl::GET_PROC_ADDRESS)SDL_GL_GetProcAddress));
  data->gl = ge::gl::opengl::getDefaultFunctionProvider();
  ge::gl::setHighDebugMessage();

  data->gl->glEnable(GL_DEPTH_TEST);
  data->gl->glDepthFunc(GL_LEQUAL);
  data->gl->glDisable(GL_CULL_FACE);
  data->gl->glClearColor(0,1,0,1);

  data->emptyVAO = std::make_shared<ge::gl::VertexArray>();

  TwInit(TW_OPENGL_CORE,nullptr);
  TwWindowSize(data->window->getWidth(),data->window->getHeight());

  auto &kernel = data->kernel;
  data->variableManipulator = std::make_shared<VariableRegisterManipulator>(kernel.variableRegister);
  //so part
  kernel.addAtomicType(
      "AssimpScene",
      sizeof(aiScene const*),
      nullptr,
      [](void*ptr){aiReleaseImport(*((aiScene const**)ptr));});
  kernel.addAtomicType(
      "SharedShader",
      sizeof(std::shared_ptr<ge::gl::Shader>),
      [](void*ptr){new(ptr)std::shared_ptr<ge::gl::Shader>;},
      [](void*ptr){((std::shared_ptr<ge::gl::Shader>*)ptr)->~shared_ptr();});
  kernel.addAtomicType(
      "SharedProgram",
      sizeof(std::shared_ptr<ge::gl::Program>),
      [](void*ptr){new(ptr)std::shared_ptr<ge::gl::Program>;},
      [](void*ptr){((std::shared_ptr<ge::gl::Program>*)ptr)->~shared_ptr();});

  kernel.addFunction({"assimpLoader"},assimpLoader);
  kernel.addFunction({"shaderSourceLoader","source","version","defines","dir","fileNames"},shaderSourceLoader);
  kernel.addFunction({"createVertexShader","sharedShader","source"},createVertexShader);
  kernel.addFunction({"createFragmentShader","sharedShader","source"},createFragmentShader);
  kernel.addFunction({"createProgram2","shaderProgram","shader0","shader1"},createProgram2);


  //script part
  auto fceLoader = kernel.createFunction("assimpLoader",{"modelFile"},"AssimpScene");

  (*fceLoader)();
  std::cout<<(*((aiScene const**)*fceLoader->getOutputData()))->mNumMeshes<<std::endl;

  kernel.addVariable("program.version",std::string("#version 450\n"));
  kernel.addVariable("program.vertexShader",std::string("vertex.vp"));
  kernel.addVariable("program.fragmentShader",std::string("fragment.fp"));
  kernel.addVariable("program.defines",std::string(""));

  auto vpl = kernel.createFunction("shaderSourceLoader",{"program.version","program.defines","shaderDirectory","program.vertexShader"},"string");
  auto fpl = kernel.createFunction("shaderSourceLoader",{"program.version","program.defines","shaderDirectory","program.fragmentShader"},"string");

  auto vpc = kernel.createFunction("createVertexShader",{vpl},"SharedShader");
  auto fpc = kernel.createFunction("createFragmentShader",{fpl},"SharedShader");

  data->prg = kernel.createFunction("createProgram2",{vpc,fpc},"SharedProgram");

}

void Data::deinit(Data*data){
  (void)data;
  data->variableManipulator = nullptr;
  data->model = nullptr;
  TwTerminate();
}
