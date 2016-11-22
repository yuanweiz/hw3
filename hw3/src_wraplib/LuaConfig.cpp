#include "LuaConfig.h"
extern "C"{
#include <lauxlib.h>
#include <lualib.h>
}
#include <stdexcept>
#include <assert.h>
#include <cstdio>
#include <string>

void lua_pcallx (lua_State* L,int nargs,int nresults,int msgh){
    const char * err;
    if (LUA_OK!=lua_pcall(L,nargs,nresults,msgh)){
        err = lua_tostring(L,-1);
        throw std::runtime_error(std::string(err));
    }
}

//wrapper for lua_get* functions, NOT exception-safe
template <typename T> T luaToTypeInPcall (lua_State * const L, int index );
//type traits
template <typename T>struct LuaTypeTrait;

static void lua_checkstackx(lua_State * L,int cnt){
    if (!lua_checkstack(L,cnt))
        throw std::runtime_error("can't allocate space for stack element!");
}


int initLuaConfig(lua_State *L){
    const char * fname=static_cast<const char*>(lua_touserdata(L,-1));
    lua_pop(L,1);
    //int ret = luaL_dofile(L,fname);
    luaL_openlibs(L);
    int ret = luaL_loadfile(L, fname) | lua_pcall(L, 0, LUA_MULTRET, 0);

    if (ret!=LUA_OK){
        lua_error(L);
    }
    //if has init() function, call it
    lua_getglobal(L,"init");
    if (lua_isfunction(L,-1)){
        lua_pcall(L,0,1,0);
    }
    else lua_pop(L,1);
    return 0;
}


LuaConfig::LuaConfig(const char* fname)
    :L(luaL_newstate())
{
    //throwing exception here is correct
    if (!L)throw std::runtime_error ("can't create lua_State");
    lua_checkstackx(L,2);

    //any function before pcall should not raise lua exceptions(errors)
    lua_pushcfunction(L,&initLuaConfig);
    lua_pushlightuserdata(L,const_cast<char*>(fname));

    //now we can use error code to handle exception
    lua_pcallx(L,1,1,0);
    //if (LUA_OK!=lua_pcall(L,1,0,0))
    //    throw std::runtime_error("init failed");
}

LuaConfig::~LuaConfig(){
    if (L){
        lua_close(L);
    }
}


template <typename T> 
int getArrayInPcall (lua_State* L){
    using std::vector;
    const char *name = static_cast<const char*>(lua_touserdata(L,-2));
    vector<T> & ret= *static_cast<vector<T>*> (lua_touserdata(L,-1));
    lua_pop(L,2);
    if (!lua_checkstack(L,3)){
        lua_pushstring(L,"no enough stack space");
        lua_error(L); //one for table, two for k/v pair
    }
    lua_getglobal(L,name);
    if (!lua_istable(L,-1)){
        lua_pop(L,1);
        lua_pushstring(L,"no such table(array)");
        lua_error(L);
    }
    lua_pushnil(L);
    while (lua_next (L, -2)!=0){
        //notice here: this function can't have strong exception-safe 
        //guarantee because lua_to* functions may call __setjmp,
        //and left ret to be "half initialized"
        //However, memory leak will not happen because
        //ret is just an on-stack reference,
        //and no RAII is involved. (weak exception safety guaranteed!)
        T val = luaToTypeInPcall<T>(L,-1);
        try {
            ret.push_back (val);
        }
        catch (...){
            //swallow it
            lua_pop(L,1);
            lua_pushstring(L,"vector::push_back throws exception");
            lua_error(L);
        }
        lua_pop(L,1); //pop up the value and keep the key
    }
    return 0;
}


template <typename T>
std::vector<T> LuaConfig::getArray(const char * name){
    lua_checkstackx(L,3);
    std::vector<T> ret;
    lua_pushcfunction(L,&getArrayInPcall<T>);
    lua_pushlightuserdata(L,const_cast<char*>(name));
    lua_pushlightuserdata(L,&ret);
    lua_pcallx(L,2,1,0);
    return ret;
}

//instantiate
std::vector<float> LuaConfig::getFloatArray(const char*name){
    return getArray<float>(name);
}

std::vector<int> LuaConfig::getIntArray(const char*name){
    return getArray<int>(name);
}

static int getLuaTableInPcall(lua_State*L){
    const char *name = static_cast<const char*>(lua_touserdata(L,-2));
    auto & ret= *static_cast<LuaTable*> (lua_touserdata(L,-1));
    lua_pop(L,2);
    if (!lua_checkstack(L,3)){
        lua_pushstring(L,"no enough stack space");
        lua_error(L); //one for table, two for k/v pair
    }
    lua_getglobal(L,name);
    if (!lua_istable(L,-1)){
        lua_pop(L,1);
        lua_pushstring(L,"no such table(array)");
        lua_error(L);
    }
    lua_pushnil(L);
    while (lua_next (L, -2)!=0){
        int type = lua_type(L,-1);
        switch (type){
            case LUA_TNUMBER:
                ret.push_back(lua_tonumber(L,-1));
                break;
            case LUA_TBOOLEAN:
                ret.push_back((bool)lua_toboolean(L,-1));
                break;
            case LUA_TSTRING:
                ret.push_back<std::string>(lua_tostring(L,-1));
                break;
            default:
                break;
        }
        lua_pop(L,1); //pop up the value and keep the key
    }
    //FIXME:pop???
    return 0;
}

LuaTable LuaConfig::getLuaTable (const char* name){
    LuaTable ret;
    lua_pushcfunction(L,&getLuaTableInPcall);
    lua_pushlightuserdata(L,(char*)name);
    lua_pushlightuserdata(L, &ret);
    lua_pcallx(L,2,1,0);
    return ret;
}

template <> struct LuaTypeTrait<int>{ const static int LUATYPE=LUA_TNUMBER ;};
template <> struct LuaTypeTrait<double>{ const static int LUATYPE=LUA_TNUMBER ;};
template <> struct LuaTypeTrait<float>{ const static int LUATYPE=LUA_TNUMBER ;};
template <> struct LuaTypeTrait<bool>{ const static int LUATYPE=LUA_TBOOLEAN ;};
template <> struct LuaTypeTrait<std::string> {const static int LUATYPE=LUA_TSTRING; };
template <> struct LuaTypeTrait<const char*> {const static int LUATYPE=LUA_TSTRING; };

template <> int luaToTypeInPcall<int> (lua_State* const L, int index){
    return static_cast<int>( .5+lua_tonumber (L,index) );
}
template <> double luaToTypeInPcall<double> (lua_State* const L, int index){
    return  lua_tonumber (L,index) ;
}
template <> float luaToTypeInPcall<float> (lua_State* const L, int index){
    return  lua_tonumber (L,index) ;
}
template <> bool luaToTypeInPcall<bool> (lua_State* const L, int index){
    int val =  lua_toboolean(L,index);
    bool ret = static_cast<bool>(val);
    return ret;
}
template <> const char* luaToTypeInPcall<const char*> (lua_State* const L,int index){
    return lua_tostring(L,index);
}
template <> std::string luaToTypeInPcall<std::string> (lua_State* const L,int index){
    return std::string(lua_tostring(L,index));
}


//this function checks stack size and type
template <typename T>
int luaGetValueInPcall (lua_State*L){
    const char *name = static_cast<const char*>
                    (lua_touserdata(L,-1));
    lua_pop(L,1);
    lua_checkstack(L,1);
    const int type = LuaTypeTrait<T>::LUATYPE;
    if (lua_getglobal(L,name)!= type){
        lua_pop(L,1);
        lua_pushstring (L,"Type mismatch");
        lua_error(L);
    }
    return 1;
}


template <typename T> 
T luaGetValue (lua_State * const L, const char * name){
    lua_checkstackx(L,2);
    lua_pushcfunction(L,&luaGetValueInPcall<T>);
    lua_pushlightuserdata(L, const_cast<char*>(name));
    lua_pcallx(L,1,1,0);
    T val =  luaToTypeInPcall<T>(L,-1);
    lua_pop(L,1);
    return val;
}

int LuaConfig::getInt(const char* name){
    return luaGetValue<int>(L,name);
}
double LuaConfig::getDouble(const char* name){
    return luaGetValue<double>(L,name);
}
bool LuaConfig::getBool(const char *name){
    return luaGetValue<bool>(L,name);
}
std::string LuaConfig::getString(const char *name){
    return luaGetValue<const char*>(L,name);
}

