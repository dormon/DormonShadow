#include<AssimpFunctions.h>
#include<geDE/Kernel.h>
#include<geDE/RegisterBasicFunction.h>
#include<geCore/ErrorPrinter.h>

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

bool assimpLoaderFailsafeTrigger(std::shared_ptr<AssimpModel>const&l,std::shared_ptr<AssimpModel>const&n){
  return n!=nullptr && l!=n;
}

void registerAssimpPlugin(ge::de::Kernel*kernel){
  assert(kernel!=nullptr);
  kernel->addAtomicType(
      "SharedAssimpModel",
      sizeof(std::shared_ptr<AssimpModel>),
      nullptr,
      [](void*ptr){((std::shared_ptr<AssimpModel>*)ptr)->~shared_ptr();});
  kernel->addFunction("assimpLoader",{"fileName","assimpModel"},assimpLoader);
  kernel->addFunction("assimpLoaderFailsafe",{"last","new","lastOrNew"},assimpLoaderFailsafe,assimpLoaderFailsafeTrigger);
}
 
