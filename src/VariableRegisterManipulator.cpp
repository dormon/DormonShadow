#include<VariableRegisterManipulator.h>
#include<geDE/Resource.h>

using namespace ge::de;

class CallbackData{
  public:
    std::string _name = "";
    std::shared_ptr<VariableRegister>_vr = nullptr;
    std::vector<size_t>_path;
    CallbackData(
        std::string                      const&name = ""     ,
        std::shared_ptr<VariableRegister>const&vr   = nullptr,
        std::vector<size_t>              const&path = {}     ){
      this->_name = name;
      this->_vr   = vr;
      this->_path = path;
    }
};

template<typename T>
void _get(const void*value,void*data){
  auto cd=(CallbackData*)data;
  if((T&)(*cd->_vr->getVariable(cd->_name)->getOutputData())!=*((T*)value)){
    cd->_vr->getVariable(cd->_name)->update(*((T*)value));
  }
}

template<typename T>
void _set(void*value,void*data){
  auto cd=(CallbackData*)data;
  *((T*)value)=(T&)(*cd->_vr->getVariable(cd->_name)->getOutputData());
}

void TW_CALL _getString(const void *value, void * clientData){
  auto cd=(CallbackData*)clientData;
  const std::string *srcPtr = static_cast<const std::string *>(value);
  if((std::string&)(*cd->_vr->getVariable(cd->_name)->getOutputData())!=*((std::string*)srcPtr)){
    cd->_vr->getVariable(cd->_name)->update(*((std::string*)srcPtr));
  }
}

void TW_CALL _setString(void *value, void * clientData){
  auto cd=(CallbackData*)clientData;
  std::string *destPtr = static_cast<std::string *>(value);
  TwCopyStdStringToLibrary(*destPtr,(std::string&)(*cd->_vr->getVariable(cd->_name)->getOutputData()));
}



std::string groupCommand(std::string groupName){
  if(groupName=="")return "";
  std::stringstream ss;
  ss<<" group=\""<<groupName<<"\" ";
  return ss.str();
}

std::string labelCommand(std::string name){
  if(name==""||name=="*")return"";
  std::stringstream ss;
  ss<<" label=\""<<name<<"\" ";
  return ss.str();
}

std::string movingCommand(std::string group,std::string outGroup,std::string notGroup){
  if(group==""||outGroup==""||outGroup==notGroup)return "";
  std::stringstream ss;
  ss<<" "<<notGroup<<"/"<<group<<" group="<<outGroup<<" ";
  return ss.str();
}

std::string closingCommand(std::string group,std::string notGroup){
  if(group=="")return "";
  std::stringstream ss;
  ss<<" "<<notGroup<<"/"<<group<<" opened=false ";
  return ss.str();
}

inline std::string const& to_string(std::string const& s){
  return s;
}

template<typename...ARGS>
std::string stringer(ARGS const&... args){
  std::string result;
  int unpack[]{0, (result += to_string(args), 0)...};
  static_cast<void>(unpack);
  return result;
}

template<typename T>
std::string toString(T const&t){
  std::stringstream ss;
  ss<<t;
  return ss.str();
}

#define CASE(TYPE,ENUM)\
  this->_callbackData.push_back(new CallbackData(varName,vr,path));\
TwAddVarCB(this->_bar,varFullName.c_str(),ENUM,\
    _get<TYPE>,\
    _set<TYPE>,\
    (void*)this->_callbackData.back(),stringer(groupCommand(group),labelCommand(varName),ENUM==TW_TYPE_FLOAT?" step=0.1 ":"").c_str());\
hasVariable = true

bool VariableRegisterManipulator::_addVariable(
    std::string varName,
    std::shared_ptr<Nullary>const&var,
    std::shared_ptr<VariableRegister>const&vr,
    std::string group,
    std::vector<size_t>&path){
  bool hasVariable = false;
  auto typeId = var->getOutputData()->getId();
  auto tr = var->getOutputData()->getManager();
  auto type = tr->getTypeIdType(typeId);
  std::string varFullName = stringer(vr->getFullName(),".",varName);
  for(auto const&x:path)
    varFullName = stringer(varFullName,"[",toString(x),"]");
  for(auto const&x:path)
    varName = stringer(varName,"[",toString(x),"]");
  switch(type){
    case TypeRegister::BOOL:
      CASE(bool,TW_TYPE_BOOLCPP);
      break;
    case TypeRegister::I8:
      CASE(int8_t,TW_TYPE_INT8);
      break;
    case TypeRegister::I16:
      CASE(int16_t,TW_TYPE_INT16);
      break;
    case TypeRegister::I32:
      CASE(int32_t,TW_TYPE_INT32);
      break;
    case TypeRegister::I64:
      break;
    case TypeRegister::U8:
      CASE(uint8_t,TW_TYPE_UINT8);
      break;
    case TypeRegister::U16:
      CASE(uint16_t,TW_TYPE_UINT16);
      break;
    case TypeRegister::U32:
      CASE(uint32_t,TW_TYPE_UINT32);
      break;
    case TypeRegister::U64:
      break;
    case TypeRegister::F32:
      CASE(float,TW_TYPE_FLOAT);
      break;
    case TypeRegister::F64:
      CASE(double,TW_TYPE_DOUBLE);
      break;
    case TypeRegister::STRING:
      this->_callbackData.push_back(new CallbackData(varName,vr));
        TwAddVarCB(this->_bar,varFullName.c_str(),TW_TYPE_STDSTRING,
            _getString,
            _setString,
            (void*)this->_callbackData.back(),stringer(groupCommand(group),labelCommand(varName)).c_str());
      hasVariable = true;
      break;
    case TypeRegister::ARRAY:

      break;
    case TypeRegister::STRUCT:
      break;
    default:
      break;
  }
  return hasVariable;
}

bool VariableRegisterManipulator::_addRegister(std::shared_ptr<VariableRegister>const&vr,std::string group,std::string notGroup){
  bool hasVariable = false;
  for(auto x=vr->varsBegin();x!=vr->varsEnd();++x){
    if(x->first.find(".")!=std::string::npos)continue;
    auto varName = x->first;
    auto var = x->second;
    std::vector<size_t>path;
    hasVariable |= this->_addVariable(varName,var,vr,group,path);
  }
  for(auto g=vr->registersBegin();g!=vr->registersEnd();++g){
    auto hv = this->_addRegister(g->second,vr->getFullName()+"."+g->first,notGroup);//vr->getName());
    TwDefine(movingCommand(vr->getFullName()+"."+g->first,vr->getFullName(),notGroup).c_str());
    if(hv){
      hasVariable = true;
      TwDefine(stringer(closingCommand(vr->getFullName()+"."+g->first,notGroup),labelCommand(g->second->getName())).c_str());
    }
  }
  return hasVariable;
}

VariableRegisterManipulator::VariableRegisterManipulator(
    std::shared_ptr<ge::de::VariableRegister> const&vr){
  this->_bar = TwNewBar(vr->getName().c_str());
  this->_vr = vr;
  this->_addRegister(vr,"",vr->getFullName());
}

VariableRegisterManipulator::~VariableRegisterManipulator(){
  for(auto x:this->_callbackData)
    delete x;
  TwDeleteBar(this->_bar);
}
