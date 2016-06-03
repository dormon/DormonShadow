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
  std::shared_ptr<ge::de::TypeRegister>             typeRegister        = nullptr;
  std::shared_ptr<ge::de::FunctionRegister>         functionRegister    = nullptr;
  std::shared_ptr<ge::de::NameRegister>             nameRegister        = nullptr;
  std::shared_ptr<ge::de::VariableRegister>         variableRegister    = nullptr;
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
  std::shared_ptr<ge::de::Resource>program = nullptr;
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

std::shared_ptr<ge::gl::Shader>createShader(GLenum type,std::string source){
  return std::make_shared<ge::gl::Shader>(type,source);
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
  data.typeRegister     = std::make_shared<ge::de::TypeRegister>();
  data.nameRegister     = std::make_shared<ge::de::NameRegister>();
  data.functionRegister = std::make_shared<ge::de::FunctionRegister>(data.typeRegister,data.nameRegister);
  ge::de::registerStdFunctions(data.functionRegister);
  data.variableRegister = std::make_shared<ge::de::VariableRegister>("*");
  auto argm = std::make_shared<ge::util::ArgumentManager>(argc-1,argv+1);
  ge::util::copyArgumentManager2VariableRegister(data.variableRegister,*argm,data.functionRegister);
  std::cout<<data.variableRegister->toStr(0,data.typeRegister)<<std::endl;

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
  (*(std::shared_ptr<ge::gl::Program>*)*this->data->program)->use();
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

  data->variableManipulator = std::make_shared<VariableRegisterManipulator>(data->variableRegister);
  //so part
  data->typeRegister->addAtomicType(
      "AssimpScene",
      sizeof(aiScene const*),
      nullptr,
      [](void*ptr){aiReleaseImport(*((aiScene const**)ptr));});
  data->typeRegister->addAtomicType(
      "SharedShader",
      sizeof(std::shared_ptr<ge::gl::Shader>),
      [](void*ptr){new(ptr)std::shared_ptr<ge::gl::Shader>;},
      [](void*ptr){((std::shared_ptr<ge::gl::Shader>*)ptr)->~shared_ptr();});
  data->typeRegister->addAtomicType(
      "SharedProgram",
      sizeof(std::shared_ptr<ge::gl::Program>),
      [](void*ptr){new(ptr)std::shared_ptr<ge::gl::Program>;},
      [](void*ptr){((std::shared_ptr<ge::gl::Program>*)ptr)->~shared_ptr();});
  ge::de::registerBasicFunction(data->functionRegister,"assimpLoader"      ,assimpLoader      );
  auto fid = ge::de::registerBasicFunction(data->functionRegister,"shaderSourceLoader",shaderSourceLoader);
  data->nameRegister->setFceInputName(fid,0,"version");
  data->nameRegister->setFceInputName(fid,1,"defines");
  data->nameRegister->setFceInputName(fid,2,"dir");
  data->nameRegister->setFceInputName(fid,3,"fileNames");
  data->nameRegister->setFceOutputName(fid,"source");
  fid = ge::de::registerBasicFunction(data->functionRegister,"createShader"      ,createShader      );
  data->nameRegister->setFceInputName(fid,0,"type");
  data->nameRegister->setFceInputName(fid,1,"source");
  data->nameRegister->setFceOutputName(fid,"sharedShader");
  fid = ge::de::registerBasicFunction(data->functionRegister,"createProgram2"    ,createProgram2    );
  data->nameRegister->setFceInputName(fid,0,"shader0");
  data->nameRegister->setFceInputName(fid,1,"shader1");
  data->nameRegister->setFceOutputName(fid,"sharedProgram");


  //script part
  data->model = data->typeRegister->sharedResource("AssimpScene");
  auto fceLoader = data->functionRegister->sharedFunction("assimpLoader");
  fceLoader->bindInput(data->functionRegister,0,data->variableRegister->getVariable("modelFile"));
  fceLoader->bindOutput(data->functionRegister,data->model);

  (*fceLoader)();
  std::cout<<(*((aiScene const**)*data->model))->mNumMeshes<<std::endl;

  auto version = data->functionRegister->sharedFunction("Nullary");
  version->bindOutput(data->functionRegister,data->typeRegister->sharedResource("string"));
  *(std::string*)*version->getOutputData() = "#version 450\n";

  auto vp = data->functionRegister->sharedFunction("Nullary");
  vp->bindOutput(data->functionRegister,data->typeRegister->sharedResource("string"));
  *(std::string*)*vp->getOutputData() = "vertex.vp";

  auto fp = data->functionRegister->sharedFunction("Nullary");
  fp->bindOutput(data->functionRegister,data->typeRegister->sharedResource("string"));
  *(std::string*)*fp->getOutputData() = "fragment.fp";

  auto defines = data->functionRegister->sharedFunction("Nullary");
  defines->bindOutput(data->functionRegister,data->typeRegister->sharedResource("string"));
  *(std::string*)*defines->getOutputData() = "";

  auto vpl = data->functionRegister->sharedFunction("shaderSourceLoader");
  vpl->bindInput(data->functionRegister,0,version);
  vpl->bindInput(data->functionRegister,1,defines);
  vpl->bindInput(data->functionRegister,2,data->variableRegister->getVariable("shaderDirectory"));
  vpl->bindInput(data->functionRegister,3,vp);
  vpl->bindOutput(data->functionRegister,data->typeRegister->sharedResource("string"));
  auto fpl = data->functionRegister->sharedFunction("shaderSourceLoader");
  fpl->bindInput(data->functionRegister,0,version);
  fpl->bindInput(data->functionRegister,1,defines);
  fpl->bindInput(data->functionRegister,2,data->variableRegister->getVariable("shaderDirectory"));
  fpl->bindInput(data->functionRegister,3,fp);
  fpl->bindOutput(data->functionRegister,data->typeRegister->sharedResource("string"));

  auto vpen = data->functionRegister->sharedFunction("Nullary");
  vpen->bindOutput(data->functionRegister,data->typeRegister->sharedResource("u32"));
  *(GLenum*)*vpen->getOutputData() = GL_VERTEX_SHADER;

  auto fpen = data->functionRegister->sharedFunction("Nullary");
  fpen->bindOutput(data->functionRegister,data->typeRegister->sharedResource("u32"));
  *(GLenum*)*fpen->getOutputData() = GL_FRAGMENT_SHADER;


  auto vpc = data->functionRegister->sharedFunction("createShader");
  vpc->bindInput(data->functionRegister,0,vpen);
  vpc->bindInput(data->functionRegister,1,vpl);
  vpc->bindOutput(data->functionRegister,data->typeRegister->sharedResource("SharedShader"));

  auto fpc = data->functionRegister->sharedFunction("createShader");
  fpc->bindInput(data->functionRegister,0,fpen);
  fpc->bindInput(data->functionRegister,1,fpl);
  fpc->bindOutput(data->functionRegister,data->typeRegister->sharedResource("SharedShader"));

  data->prg = data->functionRegister->sharedFunction("createProgram2");
  data->prg->bindInput(data->functionRegister,0,vpc);
  data->prg->bindInput(data->functionRegister,1,fpc);
  data->program = data->typeRegister->sharedResource("SharedProgram");
  data->prg->bindOutput(data->functionRegister,data->program);

  //data->model = nullptr;
  //fceLoader->bindOutput(data->functionRegister,nullptr);
}

void Data::deinit(Data*data){
  (void)data;
  data->variableManipulator = nullptr;
  data->model = nullptr;
  TwTerminate();
}
