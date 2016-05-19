#include<VariableRegisterManipulator.h>
#include<geDE/Resource.h>

using namespace ge::de;

std::string groupCommand(std::string groupName){
  if(groupName=="")return "";
  std::stringstream ss;
  ss<<"group="<<groupName;
  return ss.str();
}

std::string movingCommand(std::string group,std::string outGroup,std::string notGroup){
  if(group==""||outGroup==""||outGroup==notGroup)return "";
  std::stringstream ss;
  ss<<notGroup<<"/"<<group<<" group="<<outGroup;
  return ss.str();
}

std::string closingCommand(std::string group,std::string notGroup){
  if(group=="")return "";
  std::stringstream ss;
  ss<<notGroup<<"/"<<group<<" opened=false";
  return ss.str();
}

void VariableRegisterManipulator::_addRegister(std::shared_ptr<ge::de::VariableRegister>const&vr,std::string group,std::string outGroup,std::string notGroup){
  for(auto x=vr->varsBegin();x!=vr->varsEnd();++x){
    if(x->first.find(".")!=std::string::npos)continue;
    auto type=x->second->getOutputData()->getManager()->getTypeIdType(x->second->getOutputData()->getId());
#define CASE(TYPE,ENUM)\
    this->_callbackData.push_back(new _CallBackData(x->first,vr));\
    TwAddVarCB(this->_bar,x->first.c_str(),ENUM,\
        VariableRegisterManipulator::_get<TYPE>,\
        VariableRegisterManipulator::_set<TYPE>,\
        (void*)this->_callbackData[this->_callbackData.size()-1],groupCommand(group).c_str());\
    TwDefine(movingCommand(group,outGroup,notGroup).c_str());\
    TwDefine(closingCommand(group,notGroup).c_str());\
    break
    switch(type){
      case TypeRegister::BOOL:
        CASE(bool,TW_TYPE_BOOLCPP);
      case TypeRegister::I8:
        CASE(int8_t,TW_TYPE_INT8);
      case TypeRegister::I16:
        CASE(int16_t,TW_TYPE_INT16);
      case TypeRegister::I32:
        CASE(int32_t,TW_TYPE_INT32);
      case TypeRegister::I64:
        break;
      case TypeRegister::U8:
        CASE(uint8_t,TW_TYPE_UINT8);
      case TypeRegister::U16:
        CASE(uint16_t,TW_TYPE_UINT16);
      case TypeRegister::U32:
        CASE(uint32_t,TW_TYPE_UINT32);
      case TypeRegister::U64:
        break;
      case TypeRegister::F32:
        CASE(float,TW_TYPE_FLOAT);
      case TypeRegister::F64:
        CASE(double,TW_TYPE_DOUBLE);
      case TypeRegister::STRING:
        break;
      case TypeRegister::ARRAY:
        break;
      case TypeRegister::STRUCT:
        break;
      default:
        break;
    }
  }
  for(auto g=vr->registersBegin();g!=vr->registersEnd();++g){
    this->_addRegister(g->second,g->first,vr->getName(),notGroup);
  }
}

VariableRegisterManipulator::VariableRegisterManipulator(
    std::shared_ptr<ge::de::VariableRegister> const&vr){
  this->_bar = TwNewBar(vr->getName().c_str());
  this->_vr = vr;
  this->_addRegister(vr,"","",vr->getName());
}

VariableRegisterManipulator::~VariableRegisterManipulator(){
  for(auto x:this->_callbackData)
    delete x;
  TwDeleteBar(this->_bar);
}
