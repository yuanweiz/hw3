#include "Light.h"
#include <string.h>
#include <assert.h>

#include <algorithm>
#include <utility>
#include <stdio.h>
//#include <string>
static const char * concat(const char * name, const char * suffix){
    //auto cnt = strncpy(buf,name,sizeof buf);
    static char buf[1024];
    auto len1 = strlen(name);
    auto len2 = strlen(suffix);
    assert (len1+len2 +1 <= sizeof (buf) );
    ::memcpy(buf,name,len1);
    ::memcpy(buf+len1,suffix,len2);
    buf[len1+len2]='\0';
    return buf;
}

Light::Light (Program * program, const char * name):
    uDiffuseColor(program, concat(name,".diffuseColor")),
    uSpecularColor(program, concat(name,".specularColor")),
    uPosition(program, concat(name,".position"))
{
}

LightList::LightList (Program * program, const char * name, int sz)
{
    char localbuf[1024];
    for (int i=0;i<sz;++i){
       _snprintf(localbuf,sizeof(localbuf),"%s[%d]",name,i);
       Light light (program,localbuf);
       lights_.push_back( std::move(light));
    }
}
