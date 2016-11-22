#ifndef __ERROR_H
#define __ERROR_H
// Check if there has been an error inside OpenGL and if yes, print the error and
// through a runtime_error exception.
void checkGlErrors(const char* filename, int lineno);

// clear all the GL Error flags
void ignoreGlErrors();
#endif

