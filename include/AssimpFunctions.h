#pragma once
#pragma once

#include<geDE/Keyword.h>
#include<assimp/cimport.h>
#include<assimp/scene.h>
#include<assimp/postprocess.h>

class AssimpModel{
  public:
    aiScene const*model = nullptr;
    AssimpModel(aiScene const*m){
      this->model = m;
    }
    ~AssimpModel(){
      if(this->model)aiReleaseImport(this->model);
    }
};

namespace ge{
  namespace de{
    class Kernel;
    template<>inline std::string keyword<std::shared_ptr<AssimpModel>>(){return"SharedAssimpModel";}
  }
}

void registerAssimpPlugin(ge::de::Kernel*kernel);

