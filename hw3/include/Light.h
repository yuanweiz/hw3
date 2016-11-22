#ifndef __LIGHT_H
#define __LIGHT_H
#include "Uniform.h"
#include <vector>
#include "Noncopyable.h"

class Program;

class Light {
    //float r,g,b;
    public:
        Light(const Light&)=default;
        Light(Program* program, const char* name);
        void setDiffuseColor(float r,float g, float b){
            uDiffuseColor.setValue(r,g,b);
        }
        void setSpecularColor(float r,float g, float b){
            uSpecularColor.setValue(r,g,b);
        }
        void setPosition(float x,float y,float z){
            uPosition.setValue(x,y,z);
        }
    private:
        Uniform3f uDiffuseColor,uSpecularColor;
        Uniform3f uPosition;
};

class LightList {
    public:
        LightList (Program * program, const char* name, int cnt);
        Light & operator [] (int i) {return lights_[i];}
    private:
        std::vector<Light> lights_;
};

#endif
