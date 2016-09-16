#pragma once

#include<geDE/Keyword.h>
#include<glm/glm.hpp>
#include<glm/gtc/matrix_transform.hpp>
#include<glm/gtc/type_ptr.hpp>
#include<glm/gtc/matrix_access.hpp>

namespace ge{
  namespace de{
    class Kernel;
    template<>inline std::string keyword<glm::vec3>(){return ge::de::keyword<float[3]>();}
    template<>inline std::string keyword<glm::vec4>(){return ge::de::keyword<float[4]>();}
    template<>inline std::string keyword<glm::mat3>(){return ge::de::keyword<float[3][3]>();}
    template<>inline std::string keyword<glm::mat4>(){return ge::de::keyword<float[4][4]>();}
  }
}

void registerCameraPlugin(ge::de::Kernel*kernel);
