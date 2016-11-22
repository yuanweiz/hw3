#include "Program.h"
#include "glsupport.h"
Program::Program (const char *vs,const char *fs){
    handle_ = glCreateProgram();
    if (handle_ == 0)
      throw std::runtime_error("glCreateProgram fails");
    checkGlErrors(__FILE__, __LINE__);
    readAndCompileShader(handle_, vs,fs);
}
