#include<Tester.h>
#include<geDE/Resource.h>

#include<unistd.h>

using namespace ge::de;

Tester::Tester(
    std::shared_ptr<ge::de::VariableRegister>     const&v   ,
    std::shared_ptr<ge::de::Statement>            const&st  ,
    std::string                                   const&name,
    std::vector<std::shared_ptr<ge::de::Resource>>const&vls ):Statement(FUNCTION,true),_vr(v),_statement(st),_varName(name),_values(vls){

}

Tester::~Tester(){
}

void Tester::operator()(){
  assert(this!=nullptr);
  assert(this->_vr!=nullptr);
  if(!this->_vr->hasVariable(this->_varName)){
    if(!this->_statement)return;
    (*this->_statement)();
    return;
  }
  auto var = this->_vr->getVariable(this->_varName);
  for(auto const&x:this->_values){
    assert(x!=nullptr);
    *var = *x;
    (*this->_statement)();
  }
}


