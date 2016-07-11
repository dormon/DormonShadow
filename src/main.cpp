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
  std::shared_ptr<ge::de::Function>prg = nullptr;
  std::shared_ptr<ge::de::Function>blafce = nullptr;
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
  (*this->data->prg)();
  (*(std::shared_ptr<ge::gl::Program>*)*this->data->prg->getOutputData())->use();
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

std::string keyName(int key){
  if(key==SDLK_PERIOD)return "period";
  if(key==SDLK_KP_PERIOD)return "Keypad period";
  if(key==SDLK_QUOTE)return "quote";
  if(key==SDLK_BACKQUOTE)return "backquote";
  if(key==SDLK_QUOTEDBL)return "quotedbl";
  if(key==SDLK_ASTERISK)return "asterisk";
  return SDL_GetKeyName(key);
}

bool Data::KeyDownCallback::operator()(ge::util::EventDataPointer const&event){
  auto sdlEventData = (ge::util::SDLEventData const*)(event);
  std::stringstream ss;
  ss<<"keyboard.";
  ss<<keyName(sdlEventData->event.key.keysym.sym);
  auto &kernel = this->data->kernel;
  if(kernel.variableRegister->hasVariable(ss.str()))
    kernel.variableRegister->getVariable(ss.str())->update(true);
  return true;
}

bool Data::KeyUpCallback::operator()(ge::util::EventDataPointer const&event){
  auto sdlEventData = (ge::util::SDLEventData const*)(event);
  std::stringstream ss;
  ss<<"keyboard.";
  ss<<keyName(sdlEventData->event.key.keysym.sym);
  auto &kernel = this->data->kernel;
  if(kernel.variableRegister->hasVariable(ss.str()))
    kernel.variableRegister->getVariable(ss.str())->update(false);
  return true;
}


std::string bla(std::string str){
  std::cout<<str<<std::endl;
  return str;
}

namespace ge{
  namespace de{
    template<>inline std::string TypeRegister::getTypeKeyword<glm::vec3>(){return "vec3";}
  }
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


  //script part
  auto fceLoader = kernel.createFunction("assimpLoader",{"modelFile"},"AssimpScene");

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

  data->prg = kernel.createFunction("createVSFSProgram",{"program.version","shaderDirectory","program.defines","program.vertexShader","program.defines","program.fragmentShader"},"SharedProgram");

  kernel.addArrayType("vec3",3,"f32");

  //kernel.addVariable("camera.position",kernel.createResource("vec3"));

  kernel.addVariable("camera.position",glm::vec3(10.f));
  const int keys[]={
    SDLK_RETURN,
    SDLK_ESCAPE,
    SDLK_BACKSPACE,
    SDLK_TAB,
    SDLK_SPACE,
    SDLK_EXCLAIM,
    SDLK_QUOTEDBL,
    SDLK_HASH,
    SDLK_PERCENT,
    SDLK_DOLLAR,
    SDLK_AMPERSAND,
    SDLK_QUOTE,
    SDLK_LEFTPAREN,
    SDLK_RIGHTPAREN,
    SDLK_ASTERISK,
    SDLK_PLUS,
    SDLK_COMMA,
    SDLK_MINUS,
    SDLK_PERIOD,
    SDLK_SLASH,
    SDLK_0,
    SDLK_1,
    SDLK_2,
    SDLK_3,
    SDLK_4,
    SDLK_5,
    SDLK_6,
    SDLK_7,
    SDLK_8,
    SDLK_9,
    SDLK_COLON,
    SDLK_SEMICOLON,
    SDLK_LESS,
    SDLK_EQUALS,
    SDLK_GREATER,
    SDLK_QUESTION,
    SDLK_AT,
    SDLK_LEFTBRACKET,
    SDLK_BACKSLASH,
    SDLK_RIGHTBRACKET,
    SDLK_CARET,
    SDLK_UNDERSCORE,
    SDLK_BACKQUOTE,
    SDLK_a,
    SDLK_b,
    SDLK_c,
    SDLK_d,
    SDLK_e,
    SDLK_f,
    SDLK_g,
    SDLK_h,
    SDLK_i,
    SDLK_j,
    SDLK_k,
    SDLK_l,
    SDLK_m,
    SDLK_n,
    SDLK_o,
    SDLK_p,
    SDLK_q,
    SDLK_r,
    SDLK_s,
    SDLK_t,
    SDLK_u,
    SDLK_v,
    SDLK_w,
    SDLK_x,
    SDLK_y,
    SDLK_z,
    SDLK_CAPSLOCK,
    SDLK_F1,
    SDLK_F2,
    SDLK_F3,
    SDLK_F4,
    SDLK_F5,
    SDLK_F6,
    SDLK_F7,
    SDLK_F8,
    SDLK_F9,
    SDLK_F10,
    SDLK_F11,
    SDLK_F12,
    SDLK_PRINTSCREEN,
    SDLK_SCROLLLOCK,
    SDLK_PAUSE,
    SDLK_INSERT,
    SDLK_HOME,
    SDLK_PAGEUP,
    SDLK_DELETE,
    SDLK_END,
    SDLK_PAGEDOWN,
    SDLK_RIGHT,
    SDLK_LEFT,
    SDLK_DOWN,
    SDLK_UP,
    SDLK_NUMLOCKCLEAR,
    SDLK_KP_DIVIDE,
    SDLK_KP_MULTIPLY,
    SDLK_KP_MINUS,
    SDLK_KP_PLUS,
    SDLK_KP_ENTER,
    SDLK_KP_1,
    SDLK_KP_2,
    SDLK_KP_3,
    SDLK_KP_4,
    SDLK_KP_5,
    SDLK_KP_6,
    SDLK_KP_7,
    SDLK_KP_8,
    SDLK_KP_9,
    SDLK_KP_0,
    SDLK_KP_PERIOD,
    SDLK_APPLICATION,
    SDLK_POWER,
    SDLK_KP_EQUALS,
    SDLK_F13,
    SDLK_F14,
    SDLK_F15,
    SDLK_F16,
    SDLK_F17,
    SDLK_F18,
    SDLK_F19,
    SDLK_F20,
    SDLK_F21,
    SDLK_F22,
    SDLK_F23,
    SDLK_F24,
    SDLK_EXECUTE,
    SDLK_HELP,
    SDLK_MENU,
    SDLK_SELECT,
    SDLK_STOP,
    SDLK_AGAIN,
    SDLK_UNDO,
    SDLK_CUT,
    SDLK_COPY,
    SDLK_PASTE,
    SDLK_FIND,
    SDLK_MUTE,
    SDLK_VOLUMEUP,
    SDLK_VOLUMEDOWN,
    SDLK_KP_COMMA,
    SDLK_KP_EQUALSAS400,
    SDLK_ALTERASE,
    SDLK_SYSREQ,
    SDLK_CANCEL,
    SDLK_CLEAR,
    SDLK_PRIOR,
    SDLK_RETURN2,
    SDLK_SEPARATOR,
    SDLK_OUT,
    SDLK_OPER,
    SDLK_CLEARAGAIN,
    SDLK_CRSEL,
    SDLK_EXSEL,
    SDLK_KP_00,
    SDLK_KP_000,
    SDLK_THOUSANDSSEPARATOR,
    SDLK_DECIMALSEPARATOR,
    SDLK_CURRENCYUNIT,
    SDLK_CURRENCYSUBUNIT,
    SDLK_KP_LEFTPAREN,
    SDLK_KP_RIGHTPAREN,
    SDLK_KP_LEFTBRACE,
    SDLK_KP_RIGHTBRACE,
    SDLK_KP_TAB,
    SDLK_KP_BACKSPACE,
    SDLK_KP_A,
    SDLK_KP_B,
    SDLK_KP_C,
    SDLK_KP_D,
    SDLK_KP_E,
    SDLK_KP_F,
    SDLK_KP_XOR,
    SDLK_KP_POWER,
    SDLK_KP_PERCENT,
    SDLK_KP_LESS,
    SDLK_KP_GREATER,
    SDLK_KP_AMPERSAND,
    SDLK_KP_DBLAMPERSAND,
    SDLK_KP_VERTICALBAR,
    SDLK_KP_DBLVERTICALBAR,
    SDLK_KP_COLON,
    SDLK_KP_HASH,
    SDLK_KP_SPACE,
    SDLK_KP_AT,
    SDLK_KP_EXCLAM,
    SDLK_KP_MEMSTORE,
    SDLK_KP_MEMRECALL,
    SDLK_KP_MEMCLEAR,
    SDLK_KP_MEMADD,
    SDLK_KP_MEMSUBTRACT,
    SDLK_KP_MEMMULTIPLY,
    SDLK_KP_MEMDIVIDE,
    SDLK_KP_PLUSMINUS,
    SDLK_KP_CLEAR,
    SDLK_KP_CLEARENTRY,
    SDLK_KP_BINARY,
    SDLK_KP_OCTAL,
    SDLK_KP_DECIMAL,
    SDLK_KP_HEXADECIMAL,
    SDLK_LCTRL,
    SDLK_LSHIFT,
    SDLK_LALT,
    SDLK_LGUI,
    SDLK_RCTRL,
    SDLK_RSHIFT,
    SDLK_RALT,
    SDLK_RGUI,
    SDLK_MODE,
    SDLK_AUDIONEXT,
    SDLK_AUDIOPREV,
    SDLK_AUDIOSTOP,
    SDLK_AUDIOPLAY,
    SDLK_AUDIOMUTE,
    SDLK_MEDIASELECT,
    SDLK_WWW,
    SDLK_MAIL,
    SDLK_CALCULATOR,
    SDLK_COMPUTER,
    SDLK_AC_SEARCH,
    SDLK_AC_HOME,
    SDLK_AC_BACK,
    SDLK_AC_FORWARD,
    SDLK_AC_STOP,
    SDLK_AC_REFRESH,
    SDLK_AC_BOOKMARKS,
    SDLK_BRIGHTNESSDOWN,
    SDLK_BRIGHTNESSUP,
    SDLK_DISPLAYSWITCH,
    SDLK_KBDILLUMTOGGLE,
    SDLK_KBDILLUMDOWN,
    SDLK_KBDILLUMUP,
    SDLK_EJECT,
    SDLK_SLEEP,
  };


  for(size_t i=0;i<sizeof(keys)/sizeof(keys[0]);++i){
    std::stringstream ss;
    ss<<"keyboard.";
    ss<<keyName(keys[i]);
    kernel.addVariable(ss.str(),(bool)false);
  }


  data->variableManipulator = std::make_shared<VariableRegisterManipulator>(kernel.variableRegister);

  data->blafce = kernel.createFunction("bla",{"testString"},"string");
}

void Data::deinit(Data*data){
  (void)data;
  data->variableManipulator = nullptr;
  data->model = nullptr;
  TwTerminate();
}
