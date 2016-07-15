#include<limits>
#include<string>
#include<geGL/geGL.h>
#include<geDE/geDE.h>
#include<geDE/Kernel.h>
#include<geDE/FunctionNodeFactory.h>
#include<geDE/CompositeFunctionFactory.h>
#include<geUtil/CopyArgumentManager2VariableRegister.h>
#include<geUtil/ArgumentManager/ArgumentManager.h>
#include<geAd/SDLWindow/SDLWindow.h>
#include<geCore/Text.h>

#include<AntTweakBar.h>

#include"../include/VariableRegisterManipulator.h"

#include<assimp/cimport.h>
#include<assimp/scene.h>
#include<assimp/postprocess.h>

#include<glm/glm.hpp>
#include<glm/gtc/matrix_transform.hpp>
#include<glm/gtc/type_ptr.hpp>
#include<glm/gtc/matrix_access.hpp>


#include<RegisterKeyboard.h>

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
  class KeyDownCallback: public ge::util::EventCallbackInterface{
    public:
      Data*data;
      KeyDownCallback(Data*data){this->data = data;}
      virtual bool operator()(ge::util::EventDataPointer const&)override;
      virtual ~KeyDownCallback(){}
  };
  class KeyUpCallback: public ge::util::EventCallbackInterface{
    public:
      Data*data;
      KeyUpCallback(Data*data){this->data = data;}
      virtual bool operator()(ge::util::EventDataPointer const&)override;
      virtual ~KeyUpCallback(){}
  };

  std::shared_ptr<ge::de::Resource>model = nullptr;
  //std::shared_ptr<ge::de::Statement>prg = nullptr;
  //std::shared_ptr<ge::de::Function>prg = nullptr;
  std::shared_ptr<ge::de::Statement>prg2 = nullptr;
  std::shared_ptr<ge::de::Function>blafce = nullptr;
};


namespace ge{
  namespace de{
    GE_DE_ADD_KEYWORD(aiScene const*,"AssimpScene")
    GE_DE_ADD_KEYWORD(std::shared_ptr<ge::gl::Shader>,"SharedShader")
    GE_DE_ADD_KEYWORD(std::shared_ptr<ge::gl::Program>,"SharedProgram")
    GE_DE_ADD_KEYWORD(glm::vec3,"vec3")
    GE_DE_ADD_KEYWORD(glm::vec4,"vec4")
    GE_DE_ADD_KEYWORD(glm::mat3,"mat3")
    GE_DE_ADD_KEYWORD(glm::mat4,"mat4")
  }
}

aiScene const*assimpLoader(std::string name){
  aiScene const* model = aiImportFile(name.c_str(),aiProcess_Triangulate|aiProcess_GenNormals|aiProcess_SortByPType);
  return model;
}

std::string concatenate(std::string a,std::string b){
  return a+b;
}

glm::vec3 addOneToX(glm::vec3 const& a){
  return glm::vec3(1.f,0.f,0.f)+a;
}

glm::vec3 addOneToXIf(glm::vec3 a,bool trigger){
  if(!trigger)return a;
  return glm::vec3(1.f,0.f,0.f)+a;
}

bool addOneToXIfSignaling(glm::vec3,bool trigger){
  return trigger;
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
  std::cout<<"createVertexShader"<<std::endl;
  return std::make_shared<ge::gl::Shader>(GL_VERTEX_SHADER,source);
}

std::shared_ptr<ge::gl::Shader>createFragmentShader(std::string source){
  std::cout<<"createFragmentShader"<<std::endl;
  return std::make_shared<ge::gl::Shader>(GL_FRAGMENT_SHADER,source);
}

std::shared_ptr<ge::gl::Program>createProgram2(
    std::shared_ptr<ge::gl::Shader>s0,
    std::shared_ptr<ge::gl::Shader>s1){
  std::cout<<"createProgram"<<std::endl;
  return std::make_shared<ge::gl::Program>(s0,s1);
}

int main(int argc,char*argv[]){
  Data data;
  auto argm = std::make_shared<ge::util::ArgumentManager>(argc-1,argv+1);
  ge::util::copyArgumentManager2VariableRegister(data.kernel.variableRegister,*argm,data.kernel.functionRegister);
  std::cout<<data.kernel.variableRegister->toStr(0,data.kernel.typeRegister)<<std::endl;

  data.mainLoop = std::make_shared<ge::util::SDLEventProc>(true);
  data.mainLoop->setIdleCallback(std::make_shared<Data::IdleCallback>(&data));
  data.mainLoop->setEventHandler(std::make_shared<Data::WindowEventHandler>(&data));

  data.window   = std::make_shared<ge::util::SDLWindow>();
  data.window->createContext("rendering",450u,ge::util::SDLWindow::CORE,ge::util::SDLWindow::DEBUG);
  data.window->setEventCallback(SDL_WINDOWEVENT,std::make_shared<Data::WindowEventCallback>(&data));
  data.window->setEventCallback(SDL_KEYDOWN,std::make_shared<Data::KeyDownCallback>(&data));
  data.window->setEventCallback(SDL_KEYUP  ,std::make_shared<Data::KeyUpCallback>(&data));
  data.mainLoop->addWindow("primaryWindow",data.window);
  data.init(&data);
  (*data.mainLoop)();
  data.deinit(&data);

  return EXIT_SUCCESS;
}

void Data::IdleCallback::operator()(){
  this->data->gl->glClear(GL_COLOR_BUFFER_BIT|GL_DEPTH_BUFFER_BIT);

  this->data->emptyVAO->bind();
  //(*this->data->prg)();
  (*this->data->prg2)();
  (*(std::shared_ptr<ge::gl::Program>*)*this->data->prg2->toBody()->at(0)->toFunction()->getOutputData())->use();
  this->data->gl->glDrawArrays(GL_TRIANGLE_STRIP,0,3);
  this->data->emptyVAO->unbind();

  (*this->data->blafce)();


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

bool Data::KeyDownCallback::operator()(ge::util::EventDataPointer const&event){
  auto sdlEventData = (ge::util::SDLEventData const*)(event);
  std::stringstream ss;
  ss<<"keyboard.";
  ss<<keyboard::keyName(sdlEventData->event.key.keysym.sym);
  auto &kernel = this->data->kernel;
  if(kernel.variableRegister->hasVariable(ss.str()))
    kernel.variableRegister->getVariable(ss.str())->update(true);
  return true;
}

bool Data::KeyUpCallback::operator()(ge::util::EventDataPointer const&event){
  auto sdlEventData = (ge::util::SDLEventData const*)(event);
  std::stringstream ss;
  ss<<"keyboard.";
  ss<<keyboard::keyName(sdlEventData->event.key.keysym.sym);
  auto &kernel = this->data->kernel;
  if(kernel.variableRegister->hasVariable(ss.str()))
    kernel.variableRegister->getVariable(ss.str())->update(false);
  return true;
}


std::string bla(std::string str){
  std::cout<<str<<std::endl;
  return str;
}

class GL: public ge::gl::opengl::FunctionProvider{
  public:
    GL():FunctionProvider(){}
};

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
  //so part
  kernel.addAtomicType(
      "AssimpScene",
      sizeof(aiScene const*),
      nullptr,
      [](void*ptr){aiReleaseImport(*((aiScene const**)ptr));});
  kernel.addAtomicClass<std::shared_ptr<ge::gl::Shader>>("SharedShader");
  kernel.addAtomicClass<std::shared_ptr<ge::gl::Program>>("SharedProgram");


  kernel.addFunction({"assimpLoader","fileName"},assimpLoader);
  kernel.addFunction({"shaderSourceLoader","source","version","defines","dir","fileNames"},shaderSourceLoader);
  kernel.addFunction({"createVertexShader","sharedShader","source"},createVertexShader);
  kernel.addFunction({"createFragmentShader","sharedShader","source"},createFragmentShader);
  kernel.addFunction({"createProgram2","shaderProgram","shader0","shader1"},createProgram2);
  kernel.addFunction({"bla","output","input"},bla);

  kernel.addFunction({"addOneToX","output","input"},addOneToX);

  kernel.addFunction({"addOneToXIf","output","input"},addOneToXIf,addOneToXIfSignaling);
  //script part
  auto fceLoader = kernel.createFunction("assimpLoader",{"modelFile"});

  (*fceLoader)();
  std::cout<<(*((aiScene const**)*fceLoader->getOutputData()))->mNumMeshes<<std::endl;

  kernel.addVariable("program.version"       ,std::string("#version 450\n"));
  kernel.addVariable("program.vertexShader"  ,std::string("vertex.vp")     );
  kernel.addVariable("program.fragmentShader",std::string("fragment.fp")   );
  kernel.addVariable("program.defines"       ,std::string("")              );

  /*
     auto vpl = kernel.createFunction("shaderSourceLoader",{"program.version","program.defines","shaderDirectory","program.vertexShader"},"string");
     auto fpl = kernel.createFunction("shaderSourceLoader",{"program.version","program.defines","shaderDirectory","program.fragmentShader"},"string");

     auto vpc = kernel.createFunction("createVertexShader"  ,{vpl},"SharedShader");
     auto fpc = kernel.createFunction("createFragmentShader",{fpl},"SharedShader");

     data->prg = kernel.createFunction("createProgram2",{vpc,fpc},"SharedProgram");
     */

  auto a = kernel.createFunctionNodeFactory("vertexShaderSourceLoader","shaderSourceLoader");
  auto b = kernel.createFunctionNodeFactory("fragmentShaderSourceLoader","shaderSourceLoader");
  auto aa = kernel.createFunctionNodeFactory("createVertexShaderFromSource","createVertexShader",{a});
  auto bb = kernel.createFunctionNodeFactory("createFragmentShaderFromSource","createFragmentShader",{b});
  auto c = kernel.createFunctionNodeFactory("createProgramFromVertexFragment","createProgram2",{aa,bb});
  auto fac = kernel.createCompositeFunctionFactory("createVSFSProgram",c,{{a,b},{a,b},{a},{a},{b},{b}},{{0,0},{2,2},{1},{3},{1},{3}});
  kernel.addCompositeFunction({"sharedProgram","version","vsDefines","vertexSourceFiles","fsDefines","fragmentSourceFiles"},fac);

  //data->prg = kernel.createFunction("createVSFSProgram",{"program.version","shaderDirectory","program.defines","program.vertexShader","program.defines","program.fragmentShader"});


  kernel.addArrayType("vec3",3,"f32");
  kernel.addArrayType("mat4",16,"f32");

  //kernel.addVariable("camera.position",kernel.createResource("vec3"));

  kernel.addVariable("camera.position",glm::vec3(10.f));


  kernel.addVariable("camera.fovy",glm::radians<float>(90.f));
  kernel.addVariable("camera.near",1.f);
  kernel.addVariable("camera.far",1000.f);
  kernel.addVariable("camera.aspect",1.f);
  kernel.addVariable("camera.projection",glm::mat4(1.f));

  kernel.addFunction({"computeProjection","projectionMatrix","fovy","aspect","near","far"},glm::perspective<float>);



  keyboard::registerKeyboard(&kernel);

  data->prg2 = std::make_shared<ge::de::Body>();
  data->prg2->toBody()->addStatement(kernel.createFunction("createVSFSProgram",{"program.version","shaderDirectory","program.defines","program.vertexShader","program.defines","program.fragmentShader"}));
  data->prg2->toBody()->addStatement(kernel.createFunction("addOneToXIf",{"camera.position","keyboard.A"},"camera.position"));
  data->prg2->toBody()->addStatement(kernel.createFunction("computeProjection",{"camera.fovy","camera.aspect","camera.near","camera.far"},"camera.projection"));

  data->variableManipulator = std::make_shared<VariableRegisterManipulator>(kernel.variableRegister);

  data->blafce = kernel.createFunction("bla",{"testString"});

}

void Data::deinit(Data*data){
  (void)data;
  data->variableManipulator = nullptr;
  data->model = nullptr;
  TwTerminate();
}
