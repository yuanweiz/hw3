#ifndef __PROGRAM_H
#define __PROGRAM_H
#include <GL/glew.h>
#include "Noncopyable.h"
#include "Error.h"
// Light wrapper around GLSL program handle that automatically allocates
// and deallocates. Can be casted to a GLuint.
class Program : Noncopyable {
protected:
  GLuint handle_;

public:
  Program(const char* vs, const char *fs); 

  ~Program() {
    glDeleteProgram(handle_);
  }
  void useThis(){ glUseProgram(handle_);}

  GLuint getAttribute(const char* name){
      return glGetAttribLocation(handle_,name);
  }

  GLuint getUniform(const char * name){
      return glGetUniformLocation(handle_,name);
  }

  GLuint get(){return handle_;}

};
#endif// __PROGRAM_H
