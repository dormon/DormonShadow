#include<Camera.h>

#include<geDE/Kernel.h>
#include<geDE/RegisterBasicFunction.h>
#include<geCore/Text.h>
#include<geGL/geGL.h>

using namespace ge::de;

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

float cameraAddXRotation(float angle,float sensitivity,int32_t rel,uint32_t height,float fovy,float aspect,bool trigger){
  if(!trigger)return angle;
  (void)aspect;
  angle+=sensitivity*fovy*(float)rel/(float)height;
  //angle+=rel*sensitivity;
  return glm::clamp(angle,-glm::half_pi<float>(),glm::half_pi<float>());
}

float cameraAddYRotation(float angle,float sensitivity,int32_t rel,uint32_t width,float fovy,float aspect,bool trigger){
  if(!trigger)return angle;
  return angle+sensitivity*fovy*aspect*(float)rel/(float)width;
  //return angle+rel*sensitivity;
}

float computeAspectRatio(uint32_t w,uint32_t h){
  return (float)w/(float)h;
}



void registerCameraPlugin(ge::de::Kernel*kernel){
  assert(kernel!=nullptr);
  kernel->addArrayType("vec3",3,"f32");
  kernel->addArrayType("mat4",16,"f32");
  kernel->addArrayType(ge::de::keyword<float[16]>(),16,"f32");

  kernel->addFunction("computeAspectRatio"   ,{"width","height","aspect"},computeAspectRatio);
  kernel->addFunction("computeProjection"    ,{"fovy","aspect","near","far","projectionMatrix"},glm::perspective<float>);
  kernel->addFunction("computeViewRotation"  ,{"rotx","roty","rotz","viewRotation"},computeViewRotation);
  kernel->addFunction("computeView"          ,{"viewRotation","position","viewMatrix"},computeView);
  kernel->addFunction("cameraMove"           ,{"viewRotation","position","speed","direction","trigger","position"},cameraMove,cameraMoveTrigger);
  kernel->addFunction("cameraAddXRotation"   ,{"angle","sensitivity","rel","height","fovy","aspect","trigger","angle"},cameraAddXRotation);
  kernel->addFunction("cameraAddYRotation"   ,{"angle","sensitivity","rel","width","fovy","aspect","trigger","angle"},cameraAddYRotation);

}
