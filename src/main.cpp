#include<limits>
#include<string>
#include<geGL/geGL.h>
#include<geGL/Shader.h>
#include<geGL/Program.h>
#include<geDE/TypeRegister.h>
#include<geDE/VariableRegister.h>
#include<geDE/FunctionRegister.h>
#include<geDE/StdFunctions.h>
#include<geUtil/CopyArgumentManager2VariableRegister.h>
#include<geUtil/ArgumentManager/ArgumentManager.h>
#include<geAd/SDLWindow/SDLWindow.h>
#include<geAd/SDLWindow/SDLEventProc.h>
#include<geAd/SDLWindow/EventHandlerInterface.h>
#include<geAd/SDLWindow/EventCallbackInterface.h>
#include<geAd/SDLWindow/CallbackInterface.h>
#include<geAd/SDLWindow/SDLEventData.h>

#include<AntTweakBar.h>

#include<VariableRegisterManipulator.h>

struct Data{
  std::shared_ptr<ge::de::TypeRegister>             typeRegister     = nullptr;
  std::shared_ptr<ge::de::FunctionRegister>         functionRegister = nullptr;
  std::shared_ptr<ge::de::NameRegister>             nameRegister     = nullptr;
  std::shared_ptr<ge::de::VariableRegister>         variableRegister = nullptr;
  std::shared_ptr<ge::gl::opengl::FunctionProvider> gl           = nullptr;
  std::shared_ptr<ge::util::SDLEventProc>           mainLoop     = nullptr;
  std::shared_ptr<ge::util::SDLWindow>              window       = nullptr;
  std::shared_ptr<ge::gl::Program>program0 = nullptr;
  std::shared_ptr<ge::gl::Program>program1 = nullptr;
  std::shared_ptr<ge::gl::VertexArray>emptyVAO = nullptr;
  std::shared_ptr<VariableRegisterManipulator>variableManipulator = nullptr;
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
  TwBar*Bar;
  float Speed;
};



int main(int argc,char*argv[]){
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
  this->data->program0->use();
  this->data->gl->glDrawArrays(GL_TRIANGLE_STRIP,0,3);
  this->data->program1->use();
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
  if(handledByAnt){
    return true;
  }
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
  auto vp0 = std::make_shared<ge::gl::Shader>(GL_VERTEX_SHADER,
      "#version 450\n",
      "void main(){gl_Position = vec4(gl_VertexID%2,gl_VertexID/2,0,1);}");
  auto vp1 = std::make_shared<ge::gl::Shader>(GL_VERTEX_SHADER,
      "#version 450\n",
      "void main(){gl_Position = vec4(gl_VertexID%2-1,gl_VertexID/2-1,0,1);}");
  auto fp0 = std::make_shared<ge::gl::Shader>(GL_FRAGMENT_SHADER,
      "#version 450\n",
      "out vec4 fColor;\n"
      "void main(){fColor = vec4(1);}");
  data->program0 = std::make_shared<ge::gl::Program>(vp0,fp0);
  data->program1 = std::make_shared<ge::gl::Program>(vp1,fp0);

  //change fragment shader, both programs should be affected
  fp0->compile("#version 450\n",
      "out vec4 fColor;\n"
      "void main(){fColor = vec4(0,0,1,0);}");

  data->emptyVAO = std::make_shared<ge::gl::VertexArray>();

  TwInit(TW_OPENGL_CORE,nullptr);
  TwWindowSize(data->window->getWidth(),data->window->getHeight());
  data->Bar=TwNewBar("TweakBar");
  TwAddVarRW(data->Bar,"Speed"  ,TW_TYPE_FLOAT  ,&data->Speed     ," label='Movement speed' min=0 max=2 step=0.01"  );

  data->variableManipulator = std::make_shared<VariableRegisterManipulator>(data->variableRegister);
}

void Data::deinit(Data*data){
  (void)data;
  data->variableManipulator = nullptr;
  TwTerminate();
}
