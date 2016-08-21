#pragma once

#include<cassert>
#include<map>
#include<vector>
#include<iostream>
#include<functional>
#include<typeinfo>
#include<typeindex>


class AllContainer{
  public:
    AllContainer(){
      assert(this!=nullptr);
    }
    virtual ~AllContainer(){
      assert(this!=nullptr);
      for(auto const&x:this->_id2destructor){
        if(this->_data.count(x.first)==0)continue;
        for(auto const&y:this->_data.at(x.first))
          x.second(y);
      }
    }
    template<typename CLASS>
      bool hasType()const{
        assert(this!=nullptr);
        return this->_id2destructor.count(typeid(CLASS))!=0;
      }
    template<typename CLASS>
      bool hasValues()const{
        assert(this!=nullptr);
        if(!hasType<CLASS>())return false;
        return this->_data.count(typeid(CLASS))!=0;
      }
    template<typename CLASS>
      size_t getNofValues()const{
        assert(this!=nullptr);
        assert(this->_data.count(typeid(CLASS))!=0);
        return this->_data.at(typeid(CLASS)).size();
      }
    template<typename CLASS>
      CLASS*getValue(size_t index){
        assert(this!=nullptr);
        assert(this->_data.count(typeid(CLASS))!=0);
        assert(index<this->_data.at(typeid(CLASS)).size());
        return (CLASS*)this->_data.at(typeid(CLASS)).at(index);
      }
    template<typename CLASS>
      std::vector<void*>const&getValues(){
        assert(this!=nullptr);
        assert(this->_data.count(typeid(CLASS))!=0);
        return this->_data.at(typeid(CLASS));
      }
    template<typename CLASS,typename...ARGS>
    size_t addValue(ARGS...args){
      assert(this!=nullptr);
      if(this->_id2destructor.count(typeid(CLASS))==0)
        this->_id2destructor[typeid(CLASS)] = [](void*ptr){((CLASS*)ptr)->~CLASS();delete[](uint8_t*)ptr;};
      if(this->_data.count(typeid(CLASS))==0)
        this->_data[typeid(CLASS)] = std::vector<void*>();
      void*newData = new uint8_t[sizeof(CLASS)];
      assert(newData!=nullptr);
      new(newData)CLASS(args...);
      size_t index = this->_data.at(typeid(CLASS)).size();
      this->_data.at(typeid(CLASS)).push_back(newData);
      return index;
    }
    template<typename CLASS>
    void removeValue(size_t index){
      if(this->_data.count(typeid(CLASS))==0)return;
      auto&data=this->_data.at(typeid(CLASS));
      if(index>=data.size())return;
      this->_id2destructor.at(typeid(CLASS))(data.at(index));
      data.erase(data.begin()+index);
      if(data.size()==0)
        this->_data.erase(typeid(CLASS));
    }
  protected:
    std::map<std::type_index,std::function<void(void*)>>_id2destructor;
    std::map<std::type_index,std::vector<void*>>_data;
};


