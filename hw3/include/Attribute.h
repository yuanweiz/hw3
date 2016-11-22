#ifndef __ATTRIBUTE_H
#define __ATTRIBUTE_H
#include <GL/glew.h>
#include "Noncopyable.h"
class Program;
class Attribute:Noncopyable{
    public:
        Attribute(Program * program, const char* name );
        GLuint get (){return handle_;}
        void enable(){ glEnableVertexAttribArray(handle_);}
        void disable(){ glDisableVertexAttribArray(handle_);}
    private:
        GLuint handle_;
};
#endif// __ATTRIBUTE_H
