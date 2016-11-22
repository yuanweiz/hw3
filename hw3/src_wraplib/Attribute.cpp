#include "Attribute.h"
#include "Program.h"
#include "Error.h"
Attribute::Attribute(Program * program, const char* name)
    :handle_(program->getAttribute(name))
{
    checkGlErrors(__FILE__,__LINE__);
    this->enable();
}
