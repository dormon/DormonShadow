#pragma once

#include<AntTweakBar.h>
#include<geDE/VariableRegister.h>
#include<geDE/TypeRegister.h>
#include<geDE/Resource.h>
#include<geDE/StdFunctions.h>

class VariableRegisterManipulator{
  protected:
    TwBar* _bar = nullptr;
    std::shared_ptr<ge::de::VariableRegister> _vr = nullptr;
    class _CallBackData{
      public:
        std::string _name = "";
        std::shared_ptr<ge::de::VariableRegister> _vr = nullptr;
        _CallBackData(
            std::string name = "",
            std::shared_ptr<ge::de::VariableRegister> const& vr = nullptr){
          this->_name = name;
          this->_vr   = vr;
        }
    };
    std::vector<_CallBackData*>_callbackData;
    template<typename T>
      static void _get(const void*value,void*data){
        //std::cerr<<"get data: "<<data<<std::endl;
        //std::cerr<<"get value: "<<value<<std::endl;
        _CallBackData*cd=(_CallBackData*)data;
        //std::cerr<<"alue of "<<cd->_name<<": "<<*((T*)value)<<std::endl;
        if((T&)(*cd->_vr->getVariable(cd->_name)->getOutputData())!=*((T*)value)){
          cd->_vr->getVariable(cd->_name)->update(*((T*)value));
        }
      }
    template<typename T>
      static void _set(void*value,void*data){
        //std::cerr<<"set value: "<<value<<std::endl;
        //std::cerr<<"set data: "<<data<<std::endl;
        _CallBackData*cd=(_CallBackData*)data;
        //std::cerr<<"set cd->_name: "<<cd->_name<<std::endl;
        *((T*)value)=(T)(*cd->_vr->getVariable(cd->_name)->getOutputData());
      }
    void _addRegister(std::shared_ptr<ge::de::VariableRegister>const&vr,std::string group,std::string outGroup,std::string notGroup);
  public:
    VariableRegisterManipulator(std::shared_ptr<ge::de::VariableRegister> const&vr);
    virtual ~VariableRegisterManipulator();
};
