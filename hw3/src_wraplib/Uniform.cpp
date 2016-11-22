#include "Uniform.h"
#include "Program.h"
#define DEFINE_UNIFORM_FUNC(name,func) \
    const decltype(gl##name) name::func_ = func

//DEFINE_UNIFORM_FUNC(Uniform1f,glUniform1f);
//DEFINE_UNIFORM_FUNC(Uniform2f,glUniform2f);
//DEFINE_UNIFORM_FUNC(Uniform3f,glUniform3f);
//DEFINE_UNIFORM_FUNC(Uniform4f,glUniform4f);
//DEFINE_UNIFORM_FUNC(Uniform1d,glUniform1d);
//DEFINE_UNIFORM_FUNC(Uniform2d,glUniform2d);
//DEFINE_UNIFORM_FUNC(Uniform3d,glUniform3d);
//DEFINE_UNIFORM_FUNC(Uniform4d,glUniform4d);
//DEFINE_UNIFORM_FUNC(UniformMatrix4fv,glUniformMatrix4fv);

#undef INIT_UNIFORM_FUNC
