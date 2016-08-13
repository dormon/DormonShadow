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
#include<geUtil/Timer.h>

#include<AntTweakBar.h>


#include<assimp/cimport.h>
#include<assimp/scene.h>
#include<assimp/postprocess.h>

#include<glm/glm.hpp>
#include<glm/gtc/matrix_transform.hpp>
#include<glm/gtc/type_ptr.hpp>
#include<glm/gtc/matrix_access.hpp>


#include<VariableRegisterManipulator.h>
#include<RegisterKeyboard.h>
#include<RegisterMouse.h>
#include<Functions.h>

struct Application{
  ge::de::Kernel kernel;
  std::shared_ptr<ge::gl::Context>            gl                  = nullptr;
  std::shared_ptr<ge::ad::SDLMainLoop>        mainLoop            = nullptr;
  std::shared_ptr<ge::ad::SDLWindow>          window              = nullptr;
  std::shared_ptr<ge::gl::VertexArray>        emptyVAO            = nullptr;
  std::shared_ptr<VariableRegisterManipulator>variableManipulator = nullptr;
  std::shared_ptr<ge::de::Statement>          prg2                = nullptr;
  ~Application();
  bool init(int argc,char*argv[]);
  static void idle(void*);
  static bool eventHandler(SDL_Event const&,void*);
  template<bool DOWN>
    static bool key(SDL_Event const&,void*);
  template<bool DOWN>
    static bool mouseButton(SDL_Event const&,void*);
  static bool mouseMotion(SDL_Event const&,void*);
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

std::shared_ptr<ge::gl::Buffer>assimpModelToVBO(std::shared_ptr<AssimpModel>const&mdl){
  auto model = mdl->model;
  size_t vertices=0;
  std::vector<float>vertData;
  for(size_t m=0;m<model->mNumMeshes;++m)
    vertices+=model->mMeshes[m]->mNumVertices;
  vertData.resize(vertices*3);
  for(size_t i=0;i<model->mNumMeshes;++i){
    auto mesh = model->mMeshes[i];
    for(size_t j=0;j<mesh->mNumVertices;++j){
      vertData.push_back(mesh->mVertices[j].x);
      vertData.push_back(mesh->mVertices[j].y);
      vertData.push_back(mesh->mVertices[j].z);
    }
  }
  auto result = std::make_shared<ge::gl::Buffer>(vertices*sizeof(float)*3,vertData.data());
  return result;
}

uint32_t nofVertices(std::shared_ptr<AssimpModel>const&mdl){
  auto model = mdl->model;
  size_t vertices=0;
  std::vector<float>vertData;
  for(size_t m=0;m<model->mNumMeshes;++m)
    vertices+=model->mMeshes[m]->mNumVertices;
  return vertices;
} 

namespace ge{
  namespace de{
    GE_DE_ADD_KEYWORD(std::shared_ptr<AssimpModel>,"SharedAssimpModel")
    GE_DE_ADD_KEYWORD(std::shared_ptr<ge::gl::Buffer>,"SharedBuffer")
    GE_DE_ADD_KEYWORD(glm::vec3,ge::de::keyword<float[3]>())
    GE_DE_ADD_KEYWORD(glm::vec4,ge::de::keyword<float[4]>())
    GE_DE_ADD_KEYWORD(glm::mat3,ge::de::keyword<float[3][3]>())
    GE_DE_ADD_KEYWORD(glm::mat4,ge::de::keyword<float[4][4]>())
    GE_DE_ADD_KEYWORD(ge::gl::Context,"GL")
    GE_DE_ADD_KEYWORD(ge::util::Timer<float>,"Timer")
  }
}

std::shared_ptr<AssimpModel>assimpLoader(std::string const&name){
  auto model = aiImportFile(name.c_str(),aiProcess_Triangulate|aiProcess_GenNormals|aiProcess_SortByPType);
  if(model==nullptr){
    ge::core::printError(GE_CORE_FCENAME,"Can't open file",name);
    return nullptr;
  }
  return std::make_shared<AssimpModel>(model);
}

std::shared_ptr<AssimpModel>assimpLoaderFailsafe(std::shared_ptr<AssimpModel>const&last,std::shared_ptr<AssimpModel>const&n){
  if(n==nullptr)return last;
  return n;
}

bool assimpLoaderFailsafeTrigger(std::shared_ptr<AssimpModel>const&,std::shared_ptr<AssimpModel>const&n){
  return n!=nullptr;
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

bool cameraMoveTrigger(glm::mat4 const&,glm::vec3 const&,float,int32_t,bool trigger){
  return trigger;
}

glm::vec3 cameraMove(glm::mat4 const&viewRotation,glm::vec3 const&pos,float speed,int32_t direction,bool){
  return pos+glm::sign(direction)*speed*glm::vec3(glm::row(viewRotation,glm::abs(direction)-1));
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

int32_t clearMouseRel(){return 0;}

uint32_t incrementFrameCounter(uint32_t counter){return counter+1;}

void Application::idle(void*d){
  auto app = (Application*)d;
  app->gl->glClear(GL_COLOR_BUFFER_BIT|GL_DEPTH_BUFFER_BIT);

  app->emptyVAO->bind();
  (*app->prg2)();
  app->emptyVAO->unbind();

  TwDraw();
  app->window->swap();
  //app->kernel.variableRegister->getVariable("time")->update(app->timer.elapsedFromStart());
  //app->kernel.variableRegister->getVariable("frameTime")->update(app->timer.elapsedFromLast());
  //app->kernel.variableRegister->getVariable("fps")->update((uint32_t)*app->kernel.variable("frameCounter")/(float)*app->kernel.variable("time"));
}

bool Application::init(int argc,char*argv[]){
  auto argm = std::make_shared<ge::util::ArgumentManager>(argc-1,argv+1);
  ge::util::copyArgumentManager2VariableRegister(this->kernel.variableRegister,*argm,this->kernel.functionRegister);
  std::cout<<this->kernel.variableRegister->toStr(0,this->kernel.typeRegister)<<std::endl;

  this->mainLoop = std::make_shared<ge::ad::SDLMainLoop>(true);
  this->mainLoop->setIdleCallback(Application::idle,this);
  this->mainLoop->setEventHandler(Application::eventHandler,this);

  this->window   = std::make_shared<ge::ad::SDLWindow>();
  this->window->createContext("rendering",450u,ge::ad::SDLWindow::CORE,ge::ad::SDLWindow::DEBUG);
  this->window->setEventCallback(SDL_KEYDOWN        ,Application::key<true>,this);
  this->window->setEventCallback(SDL_KEYUP          ,Application::key<false>,this);
  this->window->setEventCallback(SDL_MOUSEBUTTONDOWN,Application::mouseButton<true>,this);
  this->window->setEventCallback(SDL_MOUSEBUTTONUP  ,Application::mouseButton<false>,this);
  this->window->setEventCallback(SDL_MOUSEMOTION    ,Application::mouseMotion,this);
  this->mainLoop->addWindow("primaryWindow",this->window);

  this->window->makeCurrent("rendering");

  ge::gl::init(SDL_GL_GetProcAddress);
  this->gl = ge::gl::getDefaultContext();
  ge::gl::setHighDebugMessage();

  this->gl->glEnable(GL_DEPTH_TEST);
  this->gl->glDepthFunc(GL_LEQUAL);
  this->gl->glDisable(GL_CULL_FACE);
  this->gl->glClearColor(0,.5,0,1);

  this->emptyVAO = std::make_shared<ge::gl::VertexArray>();

  TwInit(TW_OPENGL_CORE,nullptr);
  TwWindowSize(this->window->getWidth(),this->window->getHeight());

  kernel.typeRegister->addType<float*>();
  kernel.addAtomicType(
      "SharedAssimpModel",
      sizeof(std::shared_ptr<AssimpModel>),
      nullptr,
      [](void*ptr){((std::shared_ptr<AssimpModel>*)ptr)->~shared_ptr();});
  kernel.addAtomicClass<std::shared_ptr<ge::gl::Buffer>>("SharedBuffer");
  kernel.addAtomicClass<ge::util::Timer<float>>("Timer");

  kernel.addAtomicClass<ge::gl::Context>("GL");

  kernel.addFunction({"assimpLoader","fileName"},assimpLoader);
  kernel.addFunction({"assimpLoaderFailsafe","last","new"},assimpLoaderFailsafe,assimpLoaderFailsafeTrigger);
  kernel.addFunction({"assimpModelToVBO","assimpModel","vbo"},assimpModelToVBO);
  kernel.addFunction({"nofVertices","assimpModel","number"},nofVertices);
  //script part
  kernel.addArrayType("vec3",3,"f32");
  kernel.addArrayType("mat4",16,"f32");
  kernel.addArrayType(ge::de::keyword<float[16]>(),16,"f32");
  registerPlugin(&kernel);
  kernel.addEnumType("ShaderType",{
      GL_VERTEX_SHADER,
      GL_TESS_CONTROL_SHADER,
      GL_TESS_EVALUATION_SHADER,
      GL_GEOMETRY_SHADER,
      GL_FRAGMENT_SHADER,
      GL_COMPUTE_SHADER},{
      "vertex",
      "control",
      "evaluation",
      "geometry",
      "fragment",
      "compute"});

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
  kernel.addVariable("modelFileName"         ,std::string("/media/windata/ft/prace/models/cube/cube.obj"));
  kernel.addEmptyVariable("model","SharedAssimpModel");
  kernel.addVariable("nofModelVertices"      ,(uint32_t)0);
  kernel.addVariable("gl"                    ,ge::gl::Context{});
  kernel.addVariable("frameCounter"          ,(uint32_t)0);
  kernel.addEmptyVariable("timer","Timer");
  kernel.addVariable("time"                  ,0.0f);
  kernel.addVariable("frameTime"             ,0.0f);
  kernel.addVariable("fps"                   ,0.0f);
  kernel.addEmptyVariable("stype","ShaderType");
  keyboard::registerKeyboard(&kernel);
  mouse::registerMouse(&kernel);

  kernel.addFunction({"Timer::elapsedFromLast" ,"time"},&ge::util::Timer<float>::elapsedFromLast );
  kernel.addFunction({"Timer::elapsedFromStart","time"},&ge::util::Timer<float>::elapsedFromStart);
  kernel.addFunction({"glDrawArrays","mode","first","count"},&ge::gl::Context::glDrawArrays);
  kernel.addFunction({"computeProjection","projectionMatrix","fovy","aspect","near","far"},glm::perspective<float>);
  kernel.addFunction({"computeViewRotation","viewRotation","rotx","roty","rotz"},computeViewRotation);
  kernel.addFunction({"computeView","viewMatrix","viewRotation","position"},computeView);
  kernel.addFunction({"cameraMove","position","viewRotation","position","speed","direction","trigger"},cameraMove,cameraMoveTrigger);
  kernel.addFunction({"cameraAddXRotation","angle","angle","sensitivity","rel","trigger"},cameraAddXRotation);
  kernel.addFunction({"cameraAddYRotation","angle","angle","sensitivity","rel","trigger"},cameraAddYRotation);
  kernel.addFunction({"clearMouseRel","zero"},clearMouseRel);
  kernel.addFunction({"incrementFrameCounter","counter","counter"},incrementFrameCounter);
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




  this->prg2 = std::make_shared<ge::de::Body>(true);

  this->prg2->toBody()->addStatement(
      kernel.createFce("assimpLoaderFailsafe","model",kernel.createFce("assimpLoader","modelFileName"),"model"));
  this->prg2->toBody()->addStatement(kernel.createFce("nofVertices","model","nofModelVertices"));
  auto programStatementIndex = this->prg2->toBody()->addStatement(kernel.createFce("createVSFSProgram","program.version","shaderDirectory","program.defines","program.vertexShader","program.defines","program.fragmentShader"));
  this->prg2->toBody()->addStatement(
      kernel.createAlwaysExecFce("Program::use",kernel.createFce("sharedProgram2Program*",this->prg2->toBody()->at(programStatementIndex)->toFunction()->getOutputData())));
  this->prg2->toBody()->addStatement(kernel.createFce("Program::set3fv",
        kernel.createFce("sharedProgram2Program*",this->prg2->toBody()->at(programStatementIndex)->toFunction()->getOutputData()),
        kernel.createVariable<std::string>("position"),
        kernel.createFce("f32[3]2f32*","camera.position"),
        kernel.createVariable<GLsizei>(1)
        ));
  this->prg2->toBody()->addStatement(kernel.createFce("cameraAddXRotation",
        "camera.rotX","camera.sensitivity","mouse.yrel","mouse.left","camera.rotX"));
  this->prg2->toBody()->addStatement(kernel.createFce("cameraAddYRotation",
        "camera.rotY","camera.sensitivity","mouse.xrel","mouse.left","camera.rotY"));
  this->prg2->toBody()->addStatement(
      kernel.createAlwaysExecFce("clearMouseRel","mouse.xrel"));
  this->prg2->toBody()->addStatement(
      kernel.createAlwaysExecFce("clearMouseRel","mouse.yrel"));
  this->prg2->toBody()->addStatement(kernel.createFce("cameraMove",
        "camera.viewRotation","camera.position","camera.speed",kernel.createVariable<int32_t>(-3),"keyboard.W","camera.position"));
  this->prg2->toBody()->addStatement(kernel.createFce("cameraMove",
        "camera.viewRotation","camera.position","camera.speed",kernel.createVariable<int32_t>(+3),"keyboard.S","camera.position"));
  this->prg2->toBody()->addStatement(kernel.createFce("cameraMove",
        "camera.viewRotation","camera.position","camera.speed",kernel.createVariable<int32_t>(-1),"keyboard.A","camera.position"));
  this->prg2->toBody()->addStatement(kernel.createFce("cameraMove",
        "camera.viewRotation","camera.position","camera.speed",kernel.createVariable<int32_t>(+1),"keyboard.D","camera.position"));
  this->prg2->toBody()->addStatement(kernel.createFce("cameraMove",
        "camera.viewRotation","camera.position","camera.speed",kernel.createVariable<int32_t>(+2),"keyboard.Space","camera.position"));
  this->prg2->toBody()->addStatement(kernel.createFce("cameraMove",
        "camera.viewRotation","camera.position","camera.speed",kernel.createVariable<int32_t>(-2),"keyboard.Left Shift","camera.position"));
  this->prg2->toBody()->addStatement(kernel.createFce("computeViewRotation",
        "camera.rotX","camera.rotY","camera.rotZ","camera.viewRotation"));
  this->prg2->toBody()->addStatement(kernel.createFce("computeView",
        "camera.viewRotation","camera.position","camera.view"));
  this->prg2->toBody()->addStatement(kernel.createFce("Program::setMatrix4fv",
        kernel.createFce("sharedProgram2Program*",this->prg2->toBody()->at(programStatementIndex)->toFunction()->getOutputData()),
        kernel.createVariable<std::string>("projection"),
        kernel.createFce("f32[16]2f32*","camera.projection"),
        kernel.createVariable<GLsizei>(1),
        kernel.createVariable<GLboolean>(GL_FALSE)
        ));
  this->prg2->toBody()->addStatement(kernel.createFce("Program::setMatrix4fv",
        kernel.createFce("sharedProgram2Program*",this->prg2->toBody()->at(programStatementIndex)->toFunction()->getOutputData()),
        kernel.createVariable<std::string>("view"),
        kernel.createFce("f32[16]2f32*","camera.view"),
        kernel.createVariable<GLsizei>(1),
        kernel.createVariable<GLboolean>(GL_FALSE)
        ));
  this->prg2->toBody()->addStatement(
      kernel.createAlwaysExecFce("glDrawArrays","gl",kernel.createVariable<GLenum>(GL_TRIANGLE_STRIP),kernel.createVariable<GLint>(0),kernel.createVariable<GLsizei>(3)));
  this->prg2->toBody()->addStatement(kernel.createFce("computeProjection","camera.fovy","camera.aspect","camera.near","camera.far","camera.projection"));
  this->prg2->toBody()->addStatement(kernel.createFce("incrementFrameCounter","frameCounter","frameCounter"));
  this->prg2->toBody()->addStatement(kernel.createAlwaysExecFce("Timer::elapsedFromStart","timer","time"));

  this->variableManipulator = std::make_shared<VariableRegisterManipulator>(kernel.variableRegister,kernel.nameRegister);

  //this->timer.reset();
  return true;
}




int main(int argc,char*argv[]){
  Application app;
  if(!app.init(argc,argv))return EXIT_FAILURE;
  (*app.mainLoop)();
  return EXIT_SUCCESS;
}

bool Application::eventHandler(SDL_Event const&event,void*){
  bool handledByAnt = TwEventSDL(&event,SDL_MAJOR_VERSION,SDL_MINOR_VERSION);
  if(handledByAnt)return true;
  return false;
}

template<bool DOWN>
bool Application::key(SDL_Event const&event,void*d){
  auto app = (Application*)d;
  auto name = keyboard::fullKeyName(event.key.keysym.sym);
  auto &kernel = app->kernel;
  if(kernel.variableRegister->hasVariable(name))
    kernel.variableRegister->getVariable(name)->update(DOWN);
  return true;
}

template<bool DOWN>
bool Application::mouseButton(SDL_Event const&event,void*d){
  auto app = (Application*)d;
  auto name = mouse::fullButtonName(event.button.button);
  auto &kernel = app->kernel;
  if(kernel.variableRegister->hasVariable(name))
    kernel.variableRegister->getVariable(name)->update(DOWN);
  return true;
}

bool Application::mouseMotion(SDL_Event const&event,void*d){
  auto app = (Application*)d;
  auto &kernel = app->kernel;
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

Application::~Application(){
  this->variableManipulator = nullptr;
  TwTerminate();
}
