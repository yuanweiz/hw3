#ifndef _TEXTURE_H
#define _TEXTURE_H
#include "GL/glew.h"
#include "Noncopyable.h"
#include "Error.h"
// Light wrapper around a GL texture object handle that automatically allocates
// and deallocates. Can be casted to a GLuint.
class GlTexture : Noncopyable {
protected:
  GLuint handle_;

public:
  GlTexture() {
    glGenTextures(1, &handle_);
    checkGlErrors(__FILE__, __LINE__);
  }

  ~GlTexture() {
    glDeleteTextures(1, &handle_);
  }

  // Casts to GLuint so can be used directly by glBindTexture and so on
  operator GLuint () const {
    return handle_;
  }
};
#endif //_TEXTURE_H
