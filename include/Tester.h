#pragma once

#include<iostream>
#include<vector>
#include<geDE/VariableRegister.h>
#include<geDE/Statement.h>

class Tester: public ge::de::Statement{
  public:
    Tester(
        std::shared_ptr<ge::de::VariableRegister>     const&vr               ,
        std::shared_ptr<ge::de::Statement>            const&st      = nullptr,
        std::string                                   const&varName = ""     ,
        std::vector<std::shared_ptr<ge::de::Resource>>const&values  = {}     );
    virtual ~Tester();
    virtual void operator()();
  protected:
    std::shared_ptr<ge::de::VariableRegister>     _vr       ;
    std::shared_ptr<ge::de::Statement>            _statement;
    std::string                                   _varName  ;
    std::vector<std::shared_ptr<ge::de::Resource>>_values   ;
};
