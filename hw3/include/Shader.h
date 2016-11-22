#ifndef __SHADER_H
#define __SHADER_H
#include <GL/glew.h>
#include <stdexcept>
#include "Noncopyable.h"
#include "Error.h"
// Light wrapper around a GL shader (can be geometry/vertex/fragment shader)
// handle. Automatically allocates and deallocates. Can be casted to GLuint.
class GlShader : Noncopyable {
protected:
  GLuint handle_;

public:
  GlShader(GLenum shaderType) {
    handle_ = glCreateShader(shaderType); // create shader handle
    if (handle_ == 0)
      throw std::runtime_error("glCreateShader fails");
    checkGlErrors(__FILE__, __LINE__);
  }

  ~GlShader() {
    glDeleteShader(handle_);
  }

  // Casts to GLuint so can be used directly by glCompile etc
  operator GLuint() const {
    return handle_;
  }
};
#endif// __SHADER_H
