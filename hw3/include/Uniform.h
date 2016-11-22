#ifndef __UNIFORM_H
#define __UNIFORM_H

#include <GL/glew.h>
#include "Noncopyable.h"
#include "Program.h"
#include <assert.h>
#include <cstdio>
template <typename T>
class Uniform {
    public:
    Uniform(Program * program, const char* name)
        :handle_(program->getUniform(name))
    {
        if (handle_ == static_cast<GLuint>(-1)){
            assert(false);
        }
    }
    Uniform(const Uniform &)=default;
    GLuint get(){return handle_;}
    template <typename ... Args>
        void setValue(Args...args){
            static_cast<T*>(this)->func_(handle_,args...);
            auto err = glGetError();
            if (err!=0){
                printf("%d ",err);
                assert(false);
            }
    }
    protected:
    const GLuint handle_;
};

#define UNIFORM_DECLARE_TYPE(name,func)\
class name: public Uniform<name>{\
    public:\
    name(Program *program, const char* varname):\
    Uniform<name>(program,varname)\
        ,func_(func){}\
    const decltype(func) func_;\
}

UNIFORM_DECLARE_TYPE(Uniform1f,glUniform1f);
UNIFORM_DECLARE_TYPE(Uniform2f,glUniform2f);
UNIFORM_DECLARE_TYPE(Uniform3f,glUniform3f);
UNIFORM_DECLARE_TYPE(Uniform4f,glUniform4f);
UNIFORM_DECLARE_TYPE(Uniform1d,glUniform1d);
UNIFORM_DECLARE_TYPE(Uniform2d,glUniform2d);
UNIFORM_DECLARE_TYPE(Uniform3d,glUniform3d);
UNIFORM_DECLARE_TYPE(Uniform4d,glUniform4d);
UNIFORM_DECLARE_TYPE(Uniform1i,glUniform1i);
UNIFORM_DECLARE_TYPE(UniformMatrix4fv,glUniformMatrix4fv);

//class Uniform1f : public Uniform<Uniform1f> {
//    public:
//    Uniform1f(GLuint handle):Uniform<Uniform1f>(handle){}
//    const static decltype(glUniform1f) func_;
//};

#endif// __UNIFORM_H
