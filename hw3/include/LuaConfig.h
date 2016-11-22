#ifndef __CONFIG_H
#define __CONFIG_H
#include <vector>
#include <string>
#include <map>
#include <memory>
extern "C"{
#include <lua.h>
}
#include "Noncopyable.h"
class LuaTable :Noncopyable{
    public: 
        template<typename T> void push_back(const T&);
        template<typename T> T& get(int);
        LuaTable(LuaTable&&);
        LuaTable & operator = (LuaTable&&);
        LuaTable();
        //we have to explicitly out-line here
        //because compiler can't auto-generate
        //destructor for incomplete type
        //unique_ptr<Impl> in this
        //translation unit
        ~LuaTable();
        size_t size();
    private:
        class Impl;
        std::unique_ptr<Impl> pimpl;
};

class LuaConfig:Noncopyable{
    public:
    LuaConfig();
    LuaConfig(const char* filename);
    ~LuaConfig();
    std::vector<int> getIntArray (const char*);
    std::vector<float> getFloatArray (const char*);
    std::vector<double> getDoubleArray (const char*);
    LuaTable getLuaTable(const char*);
    //number-to-bool implicit convertion not permitted
    bool getBool(const char*);
    int getInt  (const char*);
    double getDouble(const char*);
    std::string getString(const char*);
    lua_State * get(){return L;}
    private:
    template <typename T>
    std::vector<T> getArray(const char * name);
    lua_State * L;
};


#endif// __CONFIG_H
