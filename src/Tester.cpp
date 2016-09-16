#include<Tester.h>
#include<geDE/Resource.h>

#include<unistd.h>

using namespace ge::de;

Tester::Tester(
    std::shared_ptr<ge::de::VariableRegister>     const&v    ,
    std::shared_ptr<ge::de::Statement>            const&st   ,
    std::vector<std::string>                      const&names,
    std::vector<std::shared_ptr<ge::de::Resource>>const&vls  ):Statement(FUNCTION,true),_vr(v),_statement(st),_varNames(names),_values(vls){

}

Tester::~Tester(){
}

void Tester::operator()(){
  assert(this!=nullptr);
  assert(this->_vr!=nullptr);
  if(!this->_statement)return;
  if(this->_varNames.size()==0){
    (*this->_statement)();
    return;
  }
  for(size_t i=0;i<this->_values.size();i+=this->_varNames.size()){
    size_t varNumber = 0;
    for(auto const&vn:this->_varNames){
      auto var = this->_vr->getVariable(vn);
      assert(var!=nullptr);
      *var = *this->_values.at(i+varNumber);
      varNumber++;
    }
    (*this->_statement)();
  }
}


