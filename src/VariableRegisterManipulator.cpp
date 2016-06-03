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

std::shared_ptr<Resource>address(
    std::shared_ptr<Resource>res,
    std::vector<size_t>const&path){
  std::shared_ptr<Resource>result = res;
  for(auto const&x:path)
    res = (*res)[x];
  return res;
}

template<typename T>
void _get(const void*value,void*data){
  auto cd=(CallbackData*)data;
  auto res = cd->_vr->getVariable(cd->_name)->getOutputData();
  res = address(res,cd->_path);
  if((T&)(*res)!=*((T*)value)){
    (T&)(*res) = *((T*)value);
    cd->_vr->getVariable(cd->_name)->setDirty();
    //cd->_vr->getVariable(cd->_name)->update(*((T*)value));
  }
}

template<typename T>
void _set(void*value,void*data){
  auto cd=(CallbackData*)data;
  auto res = cd->_vr->getVariable(cd->_name)->getOutputData();
  res = address(res,cd->_path);
  *((T*)value)=(T&)(*res);
  //*((T*)value)=(T&)(*cd->_vr->getVariable(cd->_name)->getOutputData());
}

void TW_CALL _getString(const void *value, void * clientData){
  auto cd=(CallbackData*)clientData;
  const std::string *srcPtr = static_cast<const std::string *>(value);
  auto res = cd->_vr->getVariable(cd->_name)->getOutputData();
  res = address(res,cd->_path);

  if((std::string&)(*res)!=*((std::string*)srcPtr)){
    (std::string&)(*res) = *((std::string*)srcPtr);
    cd->_vr->getVariable(cd->_name)->setDirty();
    //cd->_vr->getVariable(cd->_name)->update(*((std::string*)srcPtr));
  }
}

void TW_CALL _setString(void *value, void * clientData){
  auto cd=(CallbackData*)clientData;
  std::string *destPtr = static_cast<std::string *>(value);
  auto res = cd->_vr->getVariable(cd->_name)->getOutputData();
  res = address(res,cd->_path);
  TwCopyStdStringToLibrary(*destPtr,(std::string&)(*res));
  //TwCopyStdStringToLibrary(*destPtr,(std::string&)(*cd->_vr->getVariable(cd->_name)->getOutputData()));
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
    (void*)this->_callbackData.back(),stringer(groupCommand(group),labelCommand(varLabelName),ENUM==TW_TYPE_FLOAT?" step=0.1 ":"").c_str());\
hasVariable = true

bool VariableRegisterManipulator::_addVariable(
    std::string varName,
    std::shared_ptr<Nullary>const&var,
    std::shared_ptr<VariableRegister>const&vr,
    std::string group,
    std::vector<size_t>&path,
    TypeRegister::TypeId const&typeId){
  bool hasVariable = false;
  auto tr = var->getOutputData()->getManager();
  //auto type = tr->getTypeIdType(typeId);
  auto typeName = tr->getTypeIdName(typeId);
  std::string varFullName = stringer(vr->getFullName(),".",varName);
  for(auto const&x:path)
    varFullName = stringer(varFullName,"[",toString(x),"]");
  std::string varLabelName = varName;
  for(auto const&x:path)
    varLabelName = stringer(varLabelName,"[",toString(x),"]");
  if(typeName == TypeRegister::getTypeKeyword<bool>()){
    CASE(bool,TW_TYPE_BOOLCPP);
  }
  if(typeName == TypeRegister::getTypeKeyword<int8_t>()){
    CASE(int8_t,TW_TYPE_INT8);
  }
  if(typeName == TypeRegister::getTypeKeyword<int16_t>()){
    CASE(int16_t,TW_TYPE_INT16);
  }
  if(typeName == TypeRegister::getTypeKeyword<int32_t>()){
    CASE(int32_t,TW_TYPE_INT32);
  }
  if(typeName == TypeRegister::getTypeKeyword<uint8_t>()){
    CASE(uint8_t,TW_TYPE_UINT8);
  }
  if(typeName == TypeRegister::getTypeKeyword<uint16_t>()){
    CASE(uint16_t,TW_TYPE_UINT16);
  }
  if(typeName == TypeRegister::getTypeKeyword<uint32_t>()){
    CASE(uint32_t,TW_TYPE_UINT32);
  }
  if(typeName == TypeRegister::getTypeKeyword<float>()){
    CASE(float,TW_TYPE_FLOAT);
  }
  if(typeName == TypeRegister::getTypeKeyword<double>()){
    CASE(double,TW_TYPE_DOUBLE);
  }
  if(typeName == TypeRegister::getTypeKeyword<std::string>()){
    this->_callbackData.push_back(new CallbackData(varName,vr));
    TwAddVarCB(this->_bar,varFullName.c_str(),TW_TYPE_STDSTRING,
        _getString,
        _setString,
        (void*)this->_callbackData.back(),stringer(groupCommand(group),labelCommand(varName)).c_str());
    hasVariable = true;
  }
  if(tr->getTypeIdType(typeId) == TypeRegister::ARRAY){
    for(size_t i = 0;i<tr->getArraySize(typeId);++i){
      path.push_back(i);
      hasVariable |=this->_addVariable(varName,var,vr,group,path,tr->getArrayElementTypeId(typeId));
      path.pop_back();
    }
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
    hasVariable |= this->_addVariable(varName,var,vr,group,path,var->getOutputData()->getId());
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
