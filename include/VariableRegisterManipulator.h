#pragma once

#include<vector>
#include<AntTweakBar.h>
#include<geDE/VariableRegister.h>
#include<geDE/TypeRegister.h>
#include<geDE/Resource.h>
#include<geDE/StdFunctions.h>

class CallbackData;
class VariableRegisterManipulator{
  public:
      VariableRegisterManipulator(std::shared_ptr<ge::de::VariableRegister> const&vr);
      virtual ~VariableRegisterManipulator();
  protected:
    TwBar* _bar = nullptr;
    std::shared_ptr<ge::de::VariableRegister> _vr = nullptr;
    std::vector<CallbackData*>_callbackData;
    bool _addRegister(
        std::shared_ptr<ge::de::VariableRegister>const&vr,
        std::string group,
        std::string notGroup);
    bool _addVariable(
        std::string varName,
        std::shared_ptr<ge::de::Resource>const&var,
        std::shared_ptr<ge::de::VariableRegister>const&vr,
        std::string group,
        std::vector<size_t>&path,
        ge::de::TypeId const&typeId);
};
