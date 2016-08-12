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
#include<RegisterMouse.h>
#include<Functions.h>

struct Data{
  ge::de::Kernel kernel;
  std::shared_ptr<ge::gl::Context>            gl                  = nullptr;
  std::shared_ptr<ge::ad::SDLMainLoop>        mainLoop            = nullptr;
  std::shared_ptr<ge::ad::SDLWindow>          window              = nullptr;
  std::shared_ptr<ge::gl::VertexArray>        emptyVAO            = nullptr;
  std::shared_ptr<VariableRegisterManipulator>variableManipulator = nullptr;
  static void init(Data*data);
  static void deinit(Data*data);
  class IdleCallback: public ge::ad::SDLCallbackInterface{
    public:
      Data*data;
      IdleCallback(Data*data){this->data = data;}
      virtual void operator()()override;
      virtual ~IdleCallback(){}
  };
  class WindowEventCallback: public ge::ad::SDLEventCallbackInterface{
    public:
      Data*data;
      WindowEventCallback(Data*data){this->data = data;}
      virtual bool operator()(SDL_Event const&)override;
      virtual ~WindowEventCallback(){}
  };
  class WindowEventHandler: public ge::ad::SDLEventHandlerInterface{
    public:
      Data*data;
      WindowEventHandler(Data*data){this->data = data;}
      virtual bool operator()(SDL_Event const&)override;
      virtual ~WindowEventHandler(){}
  };
  class KeyDownCallback: public ge::ad::SDLEventCallbackInterface{
    public:
      Data*data;
      KeyDownCallback(Data*data){this->data = data;}
      virtual bool operator()(SDL_Event const&)override;
      virtual ~KeyDownCallback(){}
  };
  class KeyUpCallback: public ge::ad::SDLEventCallbackInterface{
    public:
      Data*data;
      KeyUpCallback(Data*data){this->data = data;}
      virtual bool operator()(SDL_Event const&)override;
      virtual ~KeyUpCallback(){}
  };
  class MouseButtonDownCallback: public ge::ad::SDLEventCallbackInterface{
    public:
      Data*data;
      MouseButtonDownCallback(Data*data){this->data = data;}
      virtual bool operator()(SDL_Event const&)override;
      virtual ~MouseButtonDownCallback(){}
  };
  class MouseButtonUpCallback: public ge::ad::SDLEventCallbackInterface{
    public:
      Data*data;
      MouseButtonUpCallback(Data*data){this->data = data;}
      virtual bool operator()(SDL_Event const&)override;
      virtual ~MouseButtonUpCallback(){}
  };
  class MouseMotionCallback: public ge::ad::SDLEventCallbackInterface{
    public:
      Data*data;
      MouseMotionCallback(Data*data){this->data = data;}
      virtual bool operator()(SDL_Event const&)override;
      virtual ~MouseMotionCallback(){}
  };


  std::shared_ptr<ge::de::Statement>prg2 = nullptr;
  std::shared_ptr<ge::de::Function>blafce = nullptr;
  std::shared_ptr<ge::de::Function>modelFce = nullptr;
};

class AssimpModel{
  public:
    aiScene const*model = nullptr;
    AssimpModel(aiScene const*m){
      std::cout<<"AssimpModel::this: "<<this<<std::endl;
      std::cout<<"AssimpModel::AssimpModel(): "<<m<<std::endl;
      this->model = m;
    }
    ~AssimpModel(){
      std::cout<<"AssimpModel::this: "<<this<<std::endl;
      std::cout<<"AssimpModel::~AssimpModel(): "<<this->model<<std::endl;
      if(this->model)aiReleaseImport(this->model);
    }
};

namespace ge{
  namespace de{
    GE_DE_ADD_KEYWORD(std::shared_ptr<AssimpModel>,"SharedAssimpModel")
    GE_DE_ADD_KEYWORD(glm::vec3,ge::de::keyword<float[3]>())
    GE_DE_ADD_KEYWORD(glm::vec4,ge::de::keyword<float[4]>())
    GE_DE_ADD_KEYWORD(glm::mat3,ge::de::keyword<float[3][3]>())
    GE_DE_ADD_KEYWORD(glm::mat4,ge::de::keyword<float[4][4]>())
    GE_DE_ADD_KEYWORD(ge::gl::Context,"GL")
  }
}

std::shared_ptr<AssimpModel>assimpLoader(std::string name){
  auto model = aiImportFile(name.c_str(),aiProcess_Triangulate|aiProcess_GenNormals|aiProcess_SortByPType);
  assert(model!=nullptr);
  return std::make_shared<AssimpModel>(model);
}

glm::vec3 addOneToXIf(glm::vec3 const&a,bool trigger){
  if(!trigger)return a;
  return glm::vec3(1.f,0.f,0.f)+a;
}

bool addOneToXIfSignaling(glm::vec3 const&,bool trigger){
  return trigger;
}

glm::mat4 computeViewRotation(float rx,float ry,float rz){
  return
    glm::rotate(glm::mat4(1.f),rz,glm::vec3(0.f,0.f,1.f))*
    glm::rotate(glm::mat4(1.f),rx,glm::vec3(1.f,0.f,0.f))*
    glm::rotate(glm::mat4(1.f),ry,glm::vec3(0.f,1.f,0.f));
}

glm::mat4 computeView(glm::mat4 const&viewRotation,glm::vec3 const&pos){
  return viewRotation*glm::translate(glm::mat4(1.f),-pos);
}

glm::vec3 cameraForward(glm::mat4 const&viewRotation,glm::vec3 const&pos,float speed,bool trigger){
  if(!trigger)return pos;
  return pos-speed*glm::vec3(glm::row(viewRotation,2));
}

glm::vec3 cameraBack(glm::mat4 const&viewRotation,glm::vec3 const&pos,float speed,bool trigger){
  if(!trigger)return pos;
  return pos+speed*glm::vec3(glm::row(viewRotation,2));
}

glm::vec3 cameraLeft(glm::mat4 const&viewRotation,glm::vec3 const&pos,float speed,bool trigger){
  if(!trigger)return pos;
  return pos-speed*glm::vec3(glm::row(viewRotation,0));
}

glm::vec3 cameraRight(glm::mat4 const&viewRotation,glm::vec3 const&pos,float speed,bool trigger){
  if(!trigger)return pos;
  return pos+speed*glm::vec3(glm::row(viewRotation,0));
}

glm::vec3 cameraUp(glm::mat4 const&viewRotation,glm::vec3 const&pos,float speed,bool trigger){
  if(!trigger)return pos;
  return pos+speed*glm::vec3(glm::row(viewRotation,1));
}

glm::vec3 cameraDown(glm::mat4 const&viewRotation,glm::vec3 const&pos,float speed,bool trigger){
  if(!trigger)return pos;
  return pos-speed*glm::vec3(glm::row(viewRotation,1));
}

float cameraAddXRotation(float angle,float sensitivity,int32_t rel,bool trigger){
  if(!trigger)return angle;
  angle+=rel*sensitivity;
  return glm::clamp(angle,-glm::half_pi<float>(),glm::half_pi<float>());
}

float cameraAddYRotation(float angle,float sensitivity,int32_t rel,bool trigger){
  if(!trigger)return angle;
  return angle+rel*sensitivity;
}

int32_t clearMouseRel(){
  return 0;
}


int main(int argc,char*argv[]){
  Data data;
  auto argm = std::make_shared<ge::util::ArgumentManager>(argc-1,argv+1);
  ge::util::copyArgumentManager2VariableRegister(data.kernel.variableRegister,*argm,data.kernel.functionRegister);
  std::cout<<data.kernel.variableRegister->toStr(0,data.kernel.typeRegister)<<std::endl;

  data.mainLoop = std::make_shared<ge::ad::SDLMainLoop>(true);
  data.mainLoop->setIdleCallback(std::make_shared<Data::IdleCallback>(&data));
  data.mainLoop->setEventHandler(std::make_shared<Data::WindowEventHandler>(&data));

  data.window   = std::make_shared<ge::ad::SDLWindow>();
  data.window->createContext("rendering",450u,ge::ad::SDLWindow::CORE,ge::ad::SDLWindow::DEBUG);
  data.window->setEventCallback(SDL_WINDOWEVENT    ,std::make_shared<Data::WindowEventCallback>(&data));
  data.window->setEventCallback(SDL_KEYDOWN        ,std::make_shared<Data::KeyDownCallback>(&data));
  data.window->setEventCallback(SDL_KEYUP          ,std::make_shared<Data::KeyUpCallback>(&data));
  data.window->setEventCallback(SDL_MOUSEBUTTONDOWN,std::make_shared<Data::MouseButtonDownCallback>(&data));
  data.window->setEventCallback(SDL_MOUSEBUTTONUP  ,std::make_shared<Data::MouseButtonUpCallback>(&data));
  data.window->setEventCallback(SDL_MOUSEMOTION    ,std::make_shared<Data::MouseMotionCallback>(&data));
  data.mainLoop->addWindow("primaryWindow",data.window);
  data.init(&data);
  (*data.mainLoop)();
  data.deinit(&data);

  return EXIT_SUCCESS;
}

void Data::IdleCallback::operator()(){
  this->data->gl->glClear(GL_COLOR_BUFFER_BIT|GL_DEPTH_BUFFER_BIT);

  this->data->emptyVAO->bind();
  (*this->data->prg2)();
  this->data->emptyVAO->unbind();

  (*this->data->blafce)();
  (*this->data->modelFce)();
  TwDraw();
  this->data->window->swap();
}

bool Data::WindowEventCallback::operator()(SDL_Event const&event){
  if(event.window.event==SDL_WINDOWEVENT_CLOSE){
    this->data->mainLoop->removeWindow("primaryWindow");
    return true;
  }
  return false;
}


bool Data::WindowEventHandler::operator()(SDL_Event const&event){
  bool handledByAnt = TwEventSDL(&event,SDL_MAJOR_VERSION,SDL_MINOR_VERSION);
  if(handledByAnt)return true;
  return false;
}

bool Data::KeyDownCallback::operator()(SDL_Event const&event){
  auto name = keyboard::fullKeyName(event.key.keysym.sym);
  auto &kernel = this->data->kernel;
  if(kernel.variableRegister->hasVariable(name))
    kernel.variableRegister->getVariable(name)->update(true);
  return true;
}

bool Data::KeyUpCallback::operator()(SDL_Event const&event){
  auto name = keyboard::fullKeyName(event.key.keysym.sym);
  auto &kernel = this->data->kernel;
  if(kernel.variableRegister->hasVariable(name))
    kernel.variableRegister->getVariable(name)->update(false);
  return true;
}

bool Data::MouseButtonDownCallback::operator()(SDL_Event const&event){
  auto name = mouse::fullButtonName(event.button.button);
  auto &kernel = this->data->kernel;
  if(kernel.variableRegister->hasVariable(name))
    kernel.variableRegister->getVariable(name)->update(true);
  return true;
}

bool Data::MouseButtonUpCallback::operator()(SDL_Event const&event){
  auto name = mouse::fullButtonName(event.button.button);
  auto &kernel = this->data->kernel;
  if(kernel.variableRegister->hasVariable(name))
    kernel.variableRegister->getVariable(name)->update(false);
  return true;
}

bool Data::MouseMotionCallback::operator()(SDL_Event const&event){
  auto &kernel = this->data->kernel;
  if(kernel.variableRegister->hasVariable("mouse.x"))
    kernel.variableRegister->getVariable("mouse.x")->update(event.motion.x);
  if(kernel.variableRegister->hasVariable("mouse.y"))
    kernel.variableRegister->getVariable("mouse.y")->update(event.motion.y);
  if(kernel.variableRegister->hasVariable("mouse.xrel"))
    kernel.variableRegister->getVariable("mouse.xrel")->update(event.motion.xrel);
  if(kernel.variableRegister->hasVariable("mouse.yrel"))
    kernel.variableRegister->getVariable("mouse.yrel")->update(event.motion.yrel);
  return true;
}




std::string bla(std::string str){
  std::cout<<str<<std::endl;
  return str;
}

void Data::init(Data*data){
  data->window->makeCurrent("rendering");

  ge::gl::init(SDL_GL_GetProcAddress);
  data->gl = ge::gl::getDefaultContext();
  ge::gl::setHighDebugMessage();

  data->gl->glEnable(GL_DEPTH_TEST);
  data->gl->glDepthFunc(GL_LEQUAL);
  data->gl->glDisable(GL_CULL_FACE);
  data->gl->glClearColor(0,1,0,1);

  data->emptyVAO = std::make_shared<ge::gl::VertexArray>();

  TwInit(TW_OPENGL_CORE,nullptr);
  TwWindowSize(data->window->getWidth(),data->window->getHeight());

  auto &kernel = data->kernel;
  kernel.typeRegister->addType<float*>();
  kernel.addAtomicType(
      "SharedAssimpModel",
      sizeof(std::shared_ptr<AssimpModel>),
      nullptr,
      [](void*ptr){((std::shared_ptr<AssimpModel>*)ptr)->~shared_ptr();});

  kernel.addAtomicClass<ge::gl::Context>("GL");

  kernel.addFunction({"assimpLoader","fileName"},assimpLoader);
  kernel.addFunction({"bla","output","input"},bla);
  kernel.addFunction({"addOneToXIf","output","input"},addOneToXIf,addOneToXIfSignaling);
  //script part
  kernel.addArrayType("vec3",3,"f32");
  kernel.addArrayType("mat4",16,"f32");
  kernel.addArrayType(ge::de::keyword<float[16]>(),16,"f32");
  registerPlugin(&kernel);

  kernel.addVariable("program.version"       ,std::string("#version 450\n"));
  kernel.addVariable("program.vertexShader"  ,std::string("vertex.vp")     );
  kernel.addVariable("program.fragmentShader",std::string("fragment.fp")   );
  kernel.addVariable("program.defines"       ,std::string("")              );
  kernel.addVariable("camera.position"       ,glm::vec3(0.f));
  kernel.addVariable("camera.fovy"           ,glm::radians<float>(90.f));
  kernel.addVariable("camera.near"           ,1.f);
  kernel.addVariable("camera.far"            ,1000.f);
  kernel.addVariable("camera.aspect"         ,1.f);
  kernel.addVariable("camera.rotX"           ,0.f);
  kernel.addVariable("camera.rotY"           ,0.f);
  kernel.addVariable("camera.rotZ"           ,0.f);
  kernel.addVariable("camera.projection"     ,glm::mat4(1.f));
  kernel.addVariable("camera.viewRotation"   ,glm::mat4(1.f));
  kernel.addVariable("camera.view"           ,glm::mat4(1.f));
  kernel.addVariable("camera.speed"          ,0.01f);
  kernel.addVariable("camera.sensitivity"    ,0.01f);
  kernel.addVariable("shaderDirectory"       ,std::string("shaders/"));
  kernel.addVariable("testString"            ,std::string("ahoj"));
  kernel.addVariable("modelFile"             ,std::string("/media/windata/ft/prace/models/cube/cube.obj"));
  kernel.addVariable("gl"                    ,ge::gl::Context{});
  keyboard::registerKeyboard(&kernel);
  mouse::registerMouse(&kernel);

  kernel.addFunction({"glDrawArrays","mode","first","count"},&ge::gl::Context::glDrawArrays);
  kernel.addFunction({"computeProjection","projectionMatrix","fovy","aspect","near","far"},glm::perspective<float>);
  kernel.addFunction({"computeViewRotation","viewRotation","rotx","roty","rotz"},computeViewRotation);
  kernel.addFunction({"computeView","viewMatrix","viewRotation","position"},computeView);
  kernel.addFunction({"cameraForward","position","viewRotation","position","speed","trigger"},cameraForward);
  kernel.addFunction({"cameraBack","position","viewRotation","position","speed","trigger"},cameraBack);
  kernel.addFunction({"cameraLeft","position","viewRotation","position","speed","trigger"},cameraLeft);
  kernel.addFunction({"cameraRight","position","viewRotation","position","speed","trigger"},cameraRight);
  kernel.addFunction({"cameraUp","position","viewRotation","position","speed","trigger"},cameraUp);
  kernel.addFunction({"cameraDown","position","viewRotation","position","speed","trigger"},cameraDown);
  kernel.addFunction({"cameraAddXRotation","angle","angle","sensitivity","rel","trigger"},cameraAddXRotation);
  kernel.addFunction({"cameraAddYRotation","angle","angle","sensitivity","rel","trigger"},cameraAddYRotation);
  kernel.addFunction({"clearMouseRel","zero"},clearMouseRel);
  {
    auto a = kernel.createFunctionNodeFactory("shaderSourceLoader");
    auto b = kernel.createFunctionNodeFactory("shaderSourceLoader");
    auto aa = kernel.createFunctionNodeFactory("createVertexShader",{a});
    auto bb = kernel.createFunctionNodeFactory("createFragmentShader",{b});
    auto c = kernel.createFunctionNodeFactory("createProgram2",{aa,bb});
    auto fac = kernel.createCompositeFunctionFactory(c,{{a,b},{a,b},{a},{a},{b},{b}},{{0,0},{2,2},{1},{3},{1},{3}});
    kernel.addCompositeFunction("createVSFSProgram",{"sharedProgram","version","vsDefines","vertexSourceFiles","fsDefines","fragmentSourceFiles"},fac);
  }
  {
    auto a = kernel.createFunctionNodeFactory("shaderSourceLoader");
    auto b = kernel.createFunctionNodeFactory("shaderSourceLoader");
    auto c = kernel.createFunctionNodeFactory("shaderSourceLoader");
    auto aa = kernel.createFunctionNodeFactory("createVertexShader",{a});
    auto bb = kernel.createFunctionNodeFactory("createGeometryShader",{b});
    auto cc = kernel.createFunctionNodeFactory("createFragmentShader",{c});
    auto d = kernel.createFunctionNodeFactory("createProgram3",{aa,bb,cc});
    auto fac = kernel.createCompositeFunctionFactory(d,{{a,b,c},{a,b,c},{a},{a},{b},{b},{c},{c}},{{0,0,0},{2,2,2},{1},{3},{1},{3},{1},{3}});
    kernel.addCompositeFunction("createVSGSFSProgram",{"sharedProgram","version","vsDefines","vsSourceFiles","gsDefines","gsSourceFiles","fsDefines","fsSourceFiles"},fac);
  }




  data->prg2 = std::make_shared<ge::de::Body>();
  data->prg2->toBody()->addStatement(kernel.createFce("createVSFSProgram","program.version","shaderDirectory","program.defines","program.vertexShader","program.defines","program.fragmentShader"));

  auto usep = kernel.createFce("Program::use",kernel.createFce("sharedProgram2Program*",data->prg2->toBody()->at(0)->toFunction()->getOutputData()));
  usep->setIgnoreDirty(true);
  usep->setIgnoreInputChanges(true);
  data->prg2->toBody()->addStatement(usep);

  data->prg2->toBody()->addStatement(kernel.createFce("Program::set3fv",
        kernel.createFce("sharedProgram2Program*",data->prg2->toBody()->at(0)->toFunction()->getOutputData()),
        kernel.createVariable<std::string>("position"),
        kernel.createFce("f32[3]2f32*","camera.position"),
        kernel.createVariable<GLsizei>(1)
        ));
  data->prg2->toBody()->addStatement(kernel.createFce("cameraAddXRotation",
        "camera.rotX","camera.sensitivity","mouse.yrel","mouse.left","camera.rotX"));
  data->prg2->toBody()->addStatement(kernel.createFce("cameraAddYRotation",
        "camera.rotY","camera.sensitivity","mouse.xrel","mouse.left","camera.rotY"));
  auto clearxrel = kernel.createFce("clearMouseRel","mouse.xrel");
  clearxrel->setIgnoreDirty(true);
  clearxrel->setIgnoreInputChanges(true);
  data->prg2->toBody()->addStatement(clearxrel);
  auto clearyrel = kernel.createFce("clearMouseRel","mouse.yrel");
  clearyrel->setIgnoreDirty(true);
  clearyrel->setIgnoreInputChanges(true);
  data->prg2->toBody()->addStatement(clearyrel);

  data->prg2->toBody()->addStatement(kernel.createFce("cameraForward",
        "camera.viewRotation","camera.position","camera.speed","keyboard.W","camera.position"));
  data->prg2->toBody()->addStatement(kernel.createFce("cameraBack",
        "camera.viewRotation","camera.position","camera.speed","keyboard.S","camera.position"));
  data->prg2->toBody()->addStatement(kernel.createFce("cameraLeft",
        "camera.viewRotation","camera.position","camera.speed","keyboard.A","camera.position"));
  data->prg2->toBody()->addStatement(kernel.createFce("cameraRight",
        "camera.viewRotation","camera.position","camera.speed","keyboard.D","camera.position"));
  data->prg2->toBody()->addStatement(kernel.createFce("cameraUp",
        "camera.viewRotation","camera.position","camera.speed","keyboard.Space","camera.position"));
  data->prg2->toBody()->addStatement(kernel.createFce("cameraDown",
        "camera.viewRotation","camera.position","camera.speed","keyboard.Left Shift","camera.position"));

  data->prg2->toBody()->addStatement(kernel.createFce("computeViewRotation",
        "camera.rotX","camera.rotY","camera.rotZ","camera.viewRotation"));
  data->prg2->toBody()->addStatement(kernel.createFce("computeView",
        "camera.viewRotation","camera.position","camera.view"));
  data->prg2->toBody()->addStatement(kernel.createFce("Program::setMatrix4fv",
        kernel.createFce("sharedProgram2Program*",data->prg2->toBody()->at(0)->toFunction()->getOutputData()),
        kernel.createVariable<std::string>("projection"),
        kernel.createFce("f32[16]2f32*","camera.projection"),
        kernel.createVariable<GLsizei>(1),
        kernel.createVariable<GLboolean>(GL_FALSE)
        ));
  data->prg2->toBody()->addStatement(kernel.createFce("Program::setMatrix4fv",
        kernel.createFce("sharedProgram2Program*",data->prg2->toBody()->at(0)->toFunction()->getOutputData()),
        kernel.createVariable<std::string>("view"),
        kernel.createFce("f32[16]2f32*","camera.view"),
        kernel.createVariable<GLsizei>(1),
        kernel.createVariable<GLboolean>(GL_FALSE)
        ));




  auto drawArrays = kernel.createFce("glDrawArrays","gl",kernel.createVariable<GLenum>(GL_TRIANGLE_STRIP),kernel.createVariable<GLint>(0),kernel.createVariable<GLsizei>(3));
  drawArrays->setIgnoreDirty(true);
  drawArrays->setIgnoreInputChanges(true);
  data->prg2->toBody()->addStatement(drawArrays);


  //data->prg2->toBody()->addStatement(kernel.createFce("addOneToXIf","camera.position","keyboard.A","camera.position"));
  data->prg2->toBody()->addStatement(kernel.createFce("computeProjection","camera.fovy","camera.aspect","camera.near","camera.far","camera.projection"));
  data->prg2->setIgnoreDirty(true);

  data->variableManipulator = std::make_shared<VariableRegisterManipulator>(kernel.variableRegister);

  data->blafce = kernel.createFce("bla","testString");
  data->modelFce = kernel.createFce("assimpLoader","modelFile");
}

void Data::deinit(Data*data){
  (void)data;
  data->variableManipulator = nullptr;
  TwTerminate();
}
