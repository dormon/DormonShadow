#include<limits>
#include<string>
#include<geDE/geDE.h>
//#include<geDE/FunctionNodeFactory.h>
//#include<geDE/CompositeFunctionFactory.h>
#include<geUtil/CopyArgumentManager2VariableRegister.h>
#include<geUtil/ArgumentManager/ArgumentManager.h>
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

#include<main.h>
#include<RegisterKeyboard.h>
#include<RegisterMouse.h>

#include<Camera.h>
#include<Functions.h>
#include<AssimpFunctions.h>

#include<Font.h>

#include<Tester.h>

int32_t nofVertices(std::shared_ptr<AssimpModel>const&mdl){
  if(mdl==nullptr){
    ge::core::printError(GE_CORE_FCENAME,"mdl is nullptr!");
    return 0;
  }
  auto model = mdl->model;
  size_t vertices=0;
  std::vector<float>vertData;
  for(size_t i=0;i<model->mNumMeshes;++i)
    vertices+=model->mMeshes[i]->mNumFaces*3;
  return vertices;
} 

std::shared_ptr<ge::gl::Buffer>assimpModelToVBO(std::shared_ptr<AssimpModel>const&mdl){
  if(mdl==nullptr){
    ge::core::printError(GE_CORE_FCENAME,"mdl is nullptr!");
    return nullptr;
  }
  auto model = mdl->model;
  size_t vertices=nofVertices(mdl);
  std::vector<float>vertData;
  vertData.reserve(vertices*3);
  for(size_t i=0;i<model->mNumMeshes;++i){
    auto mesh = model->mMeshes[i];
    for(size_t j=0;j<mesh->mNumFaces;++j)
      for(size_t k=0;k<3;++k)
        for(size_t l=0;l<3;++l)
          vertData.push_back(mesh->mVertices[mesh->mFaces[j].mIndices[k]][l]);
  }
  return std::make_shared<ge::gl::Buffer>(vertices*sizeof(float)*3,vertData.data());
}

std::shared_ptr<ge::gl::Buffer>assimpModelToNBO(std::shared_ptr<AssimpModel>const&mdl){
  if(mdl==nullptr){
    ge::core::printError(GE_CORE_FCENAME,"mdl is nullptr!");
    return nullptr;
  }
  auto model = mdl->model;
  size_t vertices=nofVertices(mdl);
  std::vector<float>vertData;
  vertData.reserve(vertices*3);
  for(size_t i=0;i<model->mNumMeshes;++i){
    auto mesh = model->mMeshes[i];
    for(size_t j=0;j<mesh->mNumFaces;++j)
      for(size_t k=0;k<3;++k)
        for(size_t l=0;l<3;++l)
          vertData.push_back(mesh->mNormals[mesh->mFaces[j].mIndices[k]][l]);
  }
  return std::make_shared<ge::gl::Buffer>(vertices*sizeof(float)*3,vertData.data());
}

namespace ge{
  namespace de{
    template<>inline std::string keyword<ge::gl::Context>(){return"GL";}
    template<>inline std::string keyword<ge::util::Timer<float>>(){return"Timer";}
    template<>inline std::string keyword<ge::ad::SDLWindow>(){return"SDLWindow";}
  }
}

int32_t clearMouseRel(){return 0;}

uint32_t incrementFrameCounter(uint32_t counter){return counter+1;}

template<typename FROM,typename TO>
TO cast(FROM const&a);

template<>inline int32_t cast(uint32_t const&a){return (int32_t)a;}

void executer(std::shared_ptr<ge::de::Statement>const&statement){
  if(statement==nullptr)return;
  (*statement)();
}

bool Application::init(int argc,char*argv[]){
  auto argm = std::make_shared<ge::util::ArgumentManager>(argc-1,argv+1);
  ge::util::copyArgumentManager2VariableRegister(this->kernel.variableRegister,*argm,this->kernel.functionRegister);

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
  this->window->setEventCallback(SDL_MOUSEWHEEL     ,Application::mouseWheel,this);
  this->window->setWindowEventCallback(SDL_WINDOWEVENT_RESIZED,Application::resize,this);
  this->mainLoop->addWindow("primaryWindow",this->window);

  this->window->makeCurrent("rendering");

  ge::gl::init(SDL_GL_GetProcAddress);
  this->gl = ge::gl::getDefaultContext();
  ge::gl::setHighDebugMessage();

  this->gl->glEnable(GL_DEPTH_TEST);
  this->gl->glDepthFunc(GL_LEQUAL);
  this->gl->glDisable(GL_CULL_FACE);
  this->gl->glClearColor(0,.5,0,1);

  TwInit(TW_OPENGL_CORE,nullptr);
  TwWindowSize(this->window->getWidth(),this->window->getHeight());

  this->editor = std::make_shared<gde::Editor>(*this->gl,glm::uvec2(this->window->getWidth(),this->window->getHeight()));

  kernel.typeRegister->addType<float*>();
  kernel.addAtomicClass<ge::util::Timer<float>>("Timer");

  kernel.addAtomicClass<ge::gl::Context>("GL");

  kernel.addAtomicClass<ge::ad::SDLWindow>();
  kernel.typeRegister->addType<ge::ad::SDLWindow*>();

  //script part
  registerCameraPlugin(&kernel);
  registerPlugin(&kernel);
  registerAssimpPlugin(&kernel);
  kernel.addFunction("assimpModelToVBO",{"assimpModel","vbo"},assimpModelToVBO);
  kernel.addFunction("assimpModelToNBO",{"assimpModel","vbo"},assimpModelToNBO);
  kernel.addFunction("nofVertices",{"assimpModel","number"},nofVertices);

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

  kernel.addVariable("window.width"          ,this->window->getWidth());
  kernel.addVariable("window.height"         ,this->window->getHeight());
  kernel.addVariable("programs.createGBuffer.version"       ,std::string("#version 450\n"));
  kernel.addVariable("programs.createGBuffer.vertexShader"  ,std::string("createGBuffer.vp"));
  kernel.addVariable("programs.createGBuffer.fragmentShader",std::string("createGBuffer.vp"));
  kernel.addVariable("programs.createGBuffer.vpDefines"     ,std::string("#define VERTEX_SHADER\n"  ));
  kernel.addVariable("programs.createGBuffer.fpDefines"     ,std::string("#define FRAGMENT_SHADER\n"));
  kernel.addVariable("programs.drawGBuffer.version"       ,std::string("#version 450\n"));
  kernel.addVariable("programs.drawGBuffer.vertexShader"  ,std::string("drawGBuffer.vp"));
  kernel.addVariable("programs.drawGBuffer.fragmentShader",std::string("drawGBuffer.vp"));
  kernel.addVariable("programs.drawGBuffer.vpDefines"     ,std::string("#define VERTEX_SHADER\n"  ));
  kernel.addVariable("programs.drawGBuffer.fpDefines"     ,std::string("#define FRAGMENT_SHADER\n"));

  kernel.addVariable("camera.position"       ,glm::vec3(0.f));
  kernel.addVariable("camera.fovy"           ,glm::radians<float>(90.f));
  kernel.addVariable("camera.near"           ,1.f);
  kernel.addVariable("camera.far"            ,1000.f);
  kernel.addVariable("camera.aspect"         ,(float)this->window->getWidth()/(float)this->window->getHeight());
  kernel.addVariable("camera.rotX"           ,0.f);
  kernel.addVariable("camera.rotY"           ,0.f);
  kernel.addVariable("camera.rotZ"           ,0.f);
  kernel.addVariable("camera.projection"     ,glm::mat4(1.f));
  kernel.addVariable("camera.viewRotation"   ,glm::mat4(1.f));
  kernel.addVariable("camera.view"           ,glm::mat4(1.f));
  kernel.addVariable("camera.speed"          ,0.01f);
  kernel.addVariable("camera.sensitivity"    ,3.0f);
  kernel.addVariable("shaderDirectory"       ,std::string("shaders/"));
  //kernel.addVariable("modelFileName"         ,std::string("/media/dormon/Data/models/o/o.3ds"));//windata/ft/prace/models/cube/cube.obj"));
  kernel.addVariable("modelFileName"         ,std::string("/media/windata/ft/prace/models/cube/cube.obj"));
  kernel.addEmptyVariable("model","SharedAssimpModel");
  kernel.addEmptyVariable("modelVertices","SharedBuffer");
  kernel.addEmptyVariable("modelNormals","SharedBuffer");
  kernel.addVariable("modelVAO"              ,std::make_shared<ge::gl::VertexArray>());
  kernel.addVariable("emptyVAO"              ,std::make_shared<ge::gl::VertexArray>());
  kernel.addVariable("nofModelVertices"      ,(int32_t)0);
  kernel.addVariable("gl"                    ,ge::gl::Context{});
  kernel.addVariable("frameCounter"          ,(uint32_t)0);
  kernel.addEmptyVariable("timer","Timer");
  kernel.addVariable("sdlwindow"             ,&*this->window);
  kernel.addVariable("time"                  ,0.0f);
  kernel.addVariable("frameTime"             ,0.0f);
  kernel.addVariable("fps"                   ,0.0f);
  kernel.addEmptyVariable("stype","ShaderType");

  kernel.addEmptyVariable("deferred.position","SharedTexture");
  kernel.addEmptyVariable("deferred.normal"  ,"SharedTexture");
  kernel.addEmptyVariable("deferred.color"   ,"SharedTexture");
  kernel.addEmptyVariable("deferred.shadowMask","SharedTexture");
  kernel.addEmptyVariable("deferred.depth"   ,"SharedTexture");
  kernel.addVariable     ("deferred.fbo"     ,std::make_shared<ge::gl::Framebuffer>());
  keyboard::registerKeyboard(&kernel);
  mouse::registerMouse(&kernel);

  kernel.addFunction("Timer::elapsedFromLast" ,{"time"},&ge::util::Timer<float>::elapsedFromLast );
  kernel.addFunction("Timer::elapsedFromStart",{"time"},&ge::util::Timer<float>::elapsedFromStart);
  kernel.addFunction("glDrawArrays"         ,{"mode","first","count"},&ge::gl::Context::glDrawArrays);
  kernel.addFunction("glClear"              ,{"mask"},&ge::gl::Context::glClear);
  kernel.addFunction("glViewport"           ,{"mask"},&ge::gl::Context::glViewport);
  kernel.addFunction("clearMouseRel"        ,{"zero"},clearMouseRel);
  kernel.addFunction("incrementFrameCounter",{"counter","counter"},incrementFrameCounter);
  kernel.addFunction("cast<u32,i32>",{"u32","i32"},cast<uint32_t,int32_t>);
  kernel.addFunction("SDLWindow::swap",{"sdlwindow"},&ge::ad::SDLWindow::swap);
  kernel.addFunction("TwDraw",{},&TwDraw);
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



  auto idleScript = std::make_shared<ge::de::Body>(true);
  idleScript->toBody()->addStatement(
      kernel.createFce("assimpLoaderFailsafe","model",kernel.createFce("assimpLoader","modelFileName"),"model"));

  //idleScript->toBody()->addStatement(
  //    kernel.createFce("assimpLoader","modelFileName","model"));

  idleScript->toBody()->addStatement(
      kernel.createFce("nofVertices","model","nofModelVertices"));
  idleScript->toBody()->addStatement(
      kernel.createFce("assimpModelToVBO","model","modelVertices"));
  idleScript->toBody()->addStatement(
      kernel.createFce("assimpModelToNBO","model","modelNormals"));
  idleScript->toBody()->addStatement(
      kernel.createFce("VertexArray::addAttrib",
        kernel.createFce("cast<SharedVertexArray,VertexArray*>","modelVAO"),
        "modelVertices",
        kernel.createVariable<GLuint>(0),
        kernel.createVariable<GLint>(3),
        kernel.createVariable<GLenum>(GL_FLOAT),
        kernel.createVariable<GLsizei>(0),
        kernel.createVariable<GLintptr>(0),
        kernel.createVariable<GLboolean>(GL_FALSE),
        kernel.createVariable<GLuint>(0),
        kernel.createVariable<int32_t>(ge::gl::VertexArray::NONE)));
  idleScript->toBody()->addStatement(
      kernel.createFce("VertexArray::addAttrib",
        kernel.createFce("cast<SharedVertexArray,VertexArray*>","modelVAO"),
        "modelNormals",
        kernel.createVariable<GLuint>(1),
        kernel.createVariable<GLint>(3),
        kernel.createVariable<GLenum>(GL_FLOAT),
        kernel.createVariable<GLsizei>(0),
        kernel.createVariable<GLintptr>(0),
        kernel.createVariable<GLboolean>(GL_FALSE),
        kernel.createVariable<GLuint>(0),
        kernel.createVariable<int32_t>(ge::gl::VertexArray::NONE)));

  auto createGBufferStatementIndex = idleScript->toBody()->addStatement(
      kernel.createFce(
        "createVSFSProgram",
        "programs.createGBuffer.version",
        "shaderDirectory",
        "programs.createGBuffer.vpDefines",
        "programs.createGBuffer.vertexShader",
        "programs.createGBuffer.fpDefines",
        "programs.createGBuffer.fragmentShader"));
  auto drawGBufferStatementIndex = idleScript->toBody()->addStatement(
      kernel.createFce(
        "createVSFSProgram",
        "programs.drawGBuffer.version",
        "shaderDirectory",
        "programs.drawGBuffer.vpDefines",
        "programs.drawGBuffer.vertexShader",
        "programs.drawGBuffer.fpDefines",
        "programs.drawGBuffer.fragmentShader"));

  idleScript->toBody()->addStatement(
      kernel.createAlwaysExecFce("VertexArray::bind",kernel.createFce("cast<SharedVertexArray,VertexArray*>","modelVAO")));
  idleScript->toBody()->addStatement(kernel.createFce("cameraAddXRotation",
        "camera.rotX","camera.sensitivity","mouse.yrel","window.height","camera.fovy","camera.aspect","mouse.left","camera.rotX"));
  idleScript->toBody()->addStatement(kernel.createFce("cameraAddYRotation",
        "camera.rotY","camera.sensitivity","mouse.xrel","window.width","camera.fovy","camera.aspect","mouse.left","camera.rotY"));
  idleScript->toBody()->addStatement(
      kernel.createAlwaysExecFce("clearMouseRel","mouse.xrel"));
  idleScript->toBody()->addStatement(
      kernel.createAlwaysExecFce("clearMouseRel","mouse.yrel"));
  idleScript->toBody()->addStatement(kernel.createFce("cameraMove",
        "camera.viewRotation","camera.position","camera.speed",kernel.createVariable<int32_t>(-3),"keyboard.W","camera.position"));
  idleScript->toBody()->addStatement(kernel.createFce("cameraMove",
        "camera.viewRotation","camera.position","camera.speed",kernel.createVariable<int32_t>(+3),"keyboard.S","camera.position"));
  idleScript->toBody()->addStatement(kernel.createFce("cameraMove",
        "camera.viewRotation","camera.position","camera.speed",kernel.createVariable<int32_t>(-1),"keyboard.A","camera.position"));
  idleScript->toBody()->addStatement(kernel.createFce("cameraMove",
        "camera.viewRotation","camera.position","camera.speed",kernel.createVariable<int32_t>(+1),"keyboard.D","camera.position"));
  idleScript->toBody()->addStatement(kernel.createFce("cameraMove",
        "camera.viewRotation","camera.position","camera.speed",kernel.createVariable<int32_t>(+2),"keyboard.Space","camera.position"));
  idleScript->toBody()->addStatement(kernel.createFce("cameraMove",
        "camera.viewRotation","camera.position","camera.speed",kernel.createVariable<int32_t>(-2),"keyboard.Left Shift","camera.position"));
  idleScript->toBody()->addStatement(kernel.createFce("computeViewRotation",
        "camera.rotX","camera.rotY","camera.rotZ","camera.viewRotation"));
  idleScript->toBody()->addStatement(kernel.createFce("computeView",
        "camera.viewRotation","camera.position","camera.view"));
  idleScript->toBody()->addStatement(
      kernel.createFce("computeAspectRatio","window.width","window.height","camera.aspect"));
  idleScript->toBody()->addStatement(
      kernel.createFce("computeProjection","camera.fovy","camera.aspect","camera.near","camera.far","camera.projection"));

  idleScript->toBody()->addStatement(
      kernel.createFce(
        "createTexture",
        kernel.createVariable<GLenum>(GL_TEXTURE_2D),
        kernel.createVariable<GLenum>(GL_RGBA32F),
        kernel.createVariable<GLsizei>(1),
        kernel.createFce("cast<u32,i32>","window.width"),
        kernel.createFce("cast<u32,i32>","window.height"),
        kernel.createVariable<GLsizei>(0),
        "deferred.position"));
  idleScript->toBody()->addStatement(
      kernel.createFce(
        "createTexture",
        kernel.createVariable<GLenum>(GL_TEXTURE_2D),
        kernel.createVariable<GLenum>(GL_RGBA32F),
        kernel.createVariable<GLsizei>(1),
        kernel.createFce("cast<u32,i32>","window.width"),
        kernel.createFce("cast<u32,i32>","window.height"),
        kernel.createVariable<GLsizei>(0),
        "deferred.normal"));
  idleScript->toBody()->addStatement(
      kernel.createFce(
        "createTexture",
        kernel.createVariable<GLenum>(GL_TEXTURE_2D),
        kernel.createVariable<GLenum>(GL_RGBA16UI),
        kernel.createVariable<GLsizei>(1),
        kernel.createFce("cast<u32,i32>","window.width"),
        kernel.createFce("cast<u32,i32>","window.height"),
        kernel.createVariable<GLsizei>(0),
        "deferred.color"));
  idleScript->toBody()->addStatement(
      kernel.createFce(
        "createTexture",
        kernel.createVariable<GLenum>(GL_TEXTURE_RECTANGLE),
        kernel.createVariable<GLenum>(GL_DEPTH24_STENCIL8),
        kernel.createVariable<GLsizei>(1),
        kernel.createFce("cast<u32,i32>","window.width"),
        kernel.createFce("cast<u32,i32>","window.height"),
        kernel.createVariable<GLsizei>(0),
        "deferred.depth"));
  idleScript->toBody()->addStatement(
      kernel.createFce(
        "createTexture",
        kernel.createVariable<GLenum>(GL_TEXTURE_2D),
        kernel.createVariable<GLenum>(GL_R32F),
        kernel.createVariable<GLsizei>(1),
        kernel.createFce("cast<u32,i32>","window.width"),
        kernel.createFce("cast<u32,i32>","window.height"),
        kernel.createVariable<GLsizei>(0),
        "deferred.shadowMask"));
  idleScript->toBody()->addStatement(
      kernel.createFce(
        "Framebuffer::attachTexture",
        kernel.createFce("cast<SharedFramebuffer,Framebuffer*>","deferred.fbo"),
        kernel.createVariable<GLenum>(GL_COLOR_ATTACHMENT0),
        "deferred.color",
        kernel.createVariable<GLsizei>(0),
        kernel.createVariable<GLsizei>(-1)));
  idleScript->toBody()->addStatement(
      kernel.createFce(
        "Framebuffer::attachTexture",
        kernel.createFce("cast<SharedFramebuffer,Framebuffer*>","deferred.fbo"),
        kernel.createVariable<GLenum>(GL_COLOR_ATTACHMENT1),
        "deferred.position",
        kernel.createVariable<GLsizei>(0),
        kernel.createVariable<GLsizei>(-1)));
  idleScript->toBody()->addStatement(
      kernel.createFce(
        "Framebuffer::attachTexture",
        kernel.createFce("cast<SharedFramebuffer,Framebuffer*>","deferred.fbo"),
        kernel.createVariable<GLenum>(GL_COLOR_ATTACHMENT2),
        "deferred.normal",
        kernel.createVariable<GLsizei>(0),
        kernel.createVariable<GLsizei>(-1)));
  idleScript->toBody()->addStatement(
      kernel.createFce(
        "Framebuffer::attachTexture",
        kernel.createFce("cast<SharedFramebuffer,Framebuffer*>","deferred.fbo"),
        kernel.createVariable<GLenum>(GL_DEPTH_ATTACHMENT),
        "deferred.depth",
        kernel.createVariable<GLsizei>(0),
        kernel.createVariable<GLsizei>(-1)));
  idleScript->toBody()->addStatement(
      kernel.createFce(
        "Framebuffer::attachTexture",
        kernel.createFce("cast<SharedFramebuffer,Framebuffer*>","deferred.fbo"),
        kernel.createVariable<GLenum>(GL_STENCIL_ATTACHMENT),
        "deferred.depth",
        kernel.createVariable<GLsizei>(0),
        kernel.createVariable<GLsizei>(-1)));
  idleScript->toBody()->addStatement(
      kernel.createFce(
        "Framebuffer::drawBuffers3",
        kernel.createFce("cast<SharedFramebuffer,Framebuffer*>","deferred.fbo"),
        kernel.createVariable<GLenum>(GL_COLOR_ATTACHMENT0),
        kernel.createVariable<GLenum>(GL_COLOR_ATTACHMENT1),
        kernel.createVariable<GLenum>(GL_COLOR_ATTACHMENT2)));



  idleScript->toBody()->addStatement(
      kernel.createAlwaysExecFce("glViewport",
        "gl",
        kernel.createVariable<GLint>(0),
        kernel.createVariable<GLint>(0),
        kernel.createFce("cast<u32,i32>","window.width"),
        kernel.createFce("cast<u32,i32>","window.height")));

  idleScript->toBody()->addStatement(
      kernel.createAlwaysExecFce(
        "Framebuffer::bind",
        kernel.createFce("cast<SharedFramebuffer,Framebuffer*>","deferred.fbo"),
        kernel.createVariable<GLenum>(GL_FRAMEBUFFER)));
  idleScript->toBody()->addStatement(
      kernel.createAlwaysExecFce("glClear","gl",kernel.createVariable<GLbitfield>(GL_COLOR_BUFFER_BIT|GL_DEPTH_BUFFER_BIT)));

  idleScript->toBody()->addStatement(
      kernel.createAlwaysExecFce("Program::use",kernel.createFce("cast<SharedProgram,Program*>",idleScript->toBody()->at(createGBufferStatementIndex)->toFunction()->getOutputData())));
  idleScript->toBody()->addStatement(kernel.createFce("Program::setMatrix4fv",
        kernel.createFce("cast<SharedProgram,Program*>",idleScript->toBody()->at(createGBufferStatementIndex)->toFunction()->getOutputData()),
        kernel.createVariable<std::string>("projection"),
        kernel.createFce("cast<f32[16],f32*>","camera.projection"),
        kernel.createVariable<GLsizei>(1),
        kernel.createVariable<GLboolean>(GL_FALSE)
        ));
  idleScript->toBody()->addStatement(kernel.createFce("Program::setMatrix4fv",
        kernel.createFce("cast<SharedProgram,Program*>",idleScript->toBody()->at(createGBufferStatementIndex)->toFunction()->getOutputData()),
        kernel.createVariable<std::string>("view"),
        kernel.createFce("cast<f32[16],f32*>","camera.view"),
        kernel.createVariable<GLsizei>(1),
        kernel.createVariable<GLboolean>(GL_FALSE)
        ));
  idleScript->toBody()->addStatement(
      kernel.createAlwaysExecFce("glDrawArrays","gl",kernel.createVariable<GLenum>(GL_TRIANGLES),kernel.createVariable<GLint>(0),"nofModelVertices"));
  idleScript->toBody()->addStatement(
      kernel.createAlwaysExecFce("VertexArray::unbind",kernel.createFce("cast<SharedVertexArray,VertexArray*>","modelVAO")));
  idleScript->toBody()->addStatement(kernel.createFce("incrementFrameCounter","frameCounter","frameCounter"));
  idleScript->toBody()->addStatement(kernel.createAlwaysExecFce("Timer::elapsedFromStart","timer","time"));

  idleScript->toBody()->addStatement(
      kernel.createAlwaysExecFce(
        "Framebuffer::unbind",
        kernel.createFce("cast<SharedFramebuffer,Framebuffer*>","deferred.fbo"),
        kernel.createVariable<GLenum>(GL_FRAMEBUFFER)));
  idleScript->toBody()->addStatement(
      kernel.createAlwaysExecFce("glClear","gl",kernel.createVariable<GLbitfield>(GL_DEPTH_BUFFER_BIT)));

  idleScript->toBody()->addStatement(
      kernel.createAlwaysExecFce("VertexArray::bind",kernel.createFce("cast<SharedVertexArray,VertexArray*>","emptyVAO")));
  idleScript->toBody()->addStatement(
      kernel.createAlwaysExecFce("Program::use",kernel.createFce("cast<SharedProgram,Program*>",idleScript->toBody()->at(drawGBufferStatementIndex)->toFunction()->getOutputData())));
  idleScript->toBody()->addStatement(
      kernel.createAlwaysExecFce("Texture::bind",kernel.createFce("cast<SharedTexture,Texture*>","deferred.color"),kernel.createVariable<GLuint>(0)));
  idleScript->toBody()->addStatement(
      kernel.createAlwaysExecFce("Texture::bind",kernel.createFce("cast<SharedTexture,Texture*>","deferred.position"),kernel.createVariable<GLuint>(1)));
  idleScript->toBody()->addStatement(
      kernel.createAlwaysExecFce("Texture::bind",kernel.createFce("cast<SharedTexture,Texture*>","deferred.normal"),kernel.createVariable<GLuint>(2)));
  idleScript->toBody()->addStatement(
      kernel.createAlwaysExecFce("Texture::bind",kernel.createFce("cast<SharedTexture,Texture*>","deferred.shadowMask"),kernel.createVariable<GLuint>(3)));
  idleScript->toBody()->addStatement(
      kernel.createAlwaysExecFce("glDrawArrays","gl",kernel.createVariable<GLenum>(GL_TRIANGLE_STRIP),kernel.createVariable<GLint>(0),kernel.createVariable<GLsizei>(4)));
  idleScript->toBody()->addStatement(
      kernel.createAlwaysExecFce("VertexArray::unbind",kernel.createFce("cast<SharedVertexArray,VertexArray*>","emptyVAO")));


  idleScript->toBody()->addStatement(kernel.createAlwaysExecFce("TwDraw"));
  idleScript->toBody()->addStatement(kernel.createAlwaysExecFce("SDLWindow::swap","sdlwindow"));

  this->idleScript = std::make_shared<Tester>(kernel.variableRegister,idleScript,
      std::vector<std::string>({"camera.fovy"}),
      std::vector<std::shared_ptr<ge::de::Resource>>({
      kernel.createVariable<float>(1.0f*glm::half_pi<float>()),
      kernel.createVariable<float>(1.1f*glm::half_pi<float>()),
      kernel.createVariable<float>(1.2f*glm::half_pi<float>()),
      kernel.createVariable<float>(1.3f*glm::half_pi<float>()),
      kernel.createVariable<float>(1.4f*glm::half_pi<float>()),
      kernel.createVariable<float>(1.5f*glm::half_pi<float>()),
      kernel.createVariable<float>(1.6f*glm::half_pi<float>()),
      kernel.createVariable<float>(1.7f*glm::half_pi<float>()),
      kernel.createVariable<float>(1.8f*glm::half_pi<float>()),
      kernel.createVariable<float>(1.9f*glm::half_pi<float>()),
        }));
  this->idleScript = idleScript;

  this->variableManipulator = std::make_shared<VariableRegisterManipulator>(kernel.variableRegister,kernel.nameRegister);

  return true;
}




int main(int argc,char*argv[]){
  Application app;
  if(!app.init(argc,argv))return EXIT_FAILURE;
  (*app.mainLoop)();
  return EXIT_SUCCESS;
}

void Application::idle(void*d){
  auto app = (Application*)d;
  (*app->idleScript)();
  //TwDraw();
  //app->editor->draw();
  //app->draw2D->draw();
  //app->window->swap();
}

bool Application::eventHandler(SDL_Event const&event,void*){
  //return false;
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
  gde::MouseButton b;
  if(event.button.button == SDL_BUTTON_LEFT)b=gde::LEFT;
  if(event.button.button == SDL_BUTTON_MIDDLE)b=gde::MIDDLE;
  if(event.button.button == SDL_BUTTON_RIGHT)b=gde::RIGHT;
  if(DOWN)
    app->editor->mouseButtonDown(b,event.button.x,app->window->getHeight()-event.button.y-1);
  else
    app->editor->mouseButtonUp(b,event.button.x,app->window->getHeight()-event.button.y-1);
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
  app->editor->mouseMotion(event.motion.xrel,-event.motion.yrel,event.motion.x,app->window->getHeight()-event.motion.y-1);
  return true;

}

bool Application::mouseWheel(SDL_Event const&event,void*d){
  auto app = (Application*)d;
  //auto &kernel = app->kernel;
  //TODO ge::dea
  app->editor->mouseWheel(event.wheel.x,event.wheel.y);
  return true;

}

bool Application::resize(SDL_Event const&event,void*d){
  auto app = (Application*)d;
  auto &kernel = app->kernel;
  kernel.variable("window.width" )->update((uint32_t)event.window.data1);
  kernel.variable("window.height")->update((uint32_t)event.window.data2);

  app->editor->resize(event.window.data1,event.window.data2);
  //app->draw2D->setViewportSize(glm::uvec2(event.window.data1,event.window.data2));

  return true;
}

Application::~Application(){
  this->variableManipulator = nullptr;
  TwTerminate();
}
