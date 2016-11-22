#ifndef __BUFFER_OBJECT_H
#define __BUFFER_OBJECT_H
#include <GL/glew.h>
#include "Noncopyable.h"
#include "Error.h"
#include <cstddef>
#include "Program.h"
// Light wrapper around a GL buffer object handle that automatically allocates
// and deallocates. Can be casted to a GLuint.

namespace detail{
template <int BUFFER,typename T>
class GlBufferObject : Noncopyable {
protected:
  GLuint handle_;
  size_t size_;

public:
  //GlBufferObject():handle_(0){}
  //void init(T *data, size_t size){
  GlBufferObject (T*data, size_t sz){
      size_ = sz;
      glGenBuffers(1, &handle_);
      glBindBuffer(BUFFER,handle_);
      glBufferData(BUFFER,sz*sizeof(T),data,GL_STATIC_DRAW);
      checkGlErrors(__FILE__, __LINE__);
  }
  void bind (){
      glBindBuffer(BUFFER,handle_);
  }

  ~GlBufferObject() {
    glDeleteBuffers(1, &handle_);
  }

  size_t size(){return size_;}
  GLuint get(){return handle_;}
};
}

struct VertexPN; //forward declaration
typedef detail::GlBufferObject<GL_ARRAY_BUFFER, GLfloat> VertexBuffer;
typedef detail::GlBufferObject<GL_ARRAY_BUFFER, VertexPN> VertexPNBuffer;
typedef detail::GlBufferObject<GL_ELEMENT_ARRAY_BUFFER, unsigned short> IndexBuffer;

#endif //__BUFFER_OBJECT_H
