attribute vec4 position;
attribute vec4 normal;
attribute vec2 uv;
attribute vec4 tangent;
attribute vec4 binormal;

attribute vec4 flooruv;
attribute vec4 floorNormal;

varying mat3 varyingTBNMatrix;
varying vec3 varyingPosition;
varying vec2 varyingUv;

uniform mat4 mvm;
uniform mat4 p;
uniform mat4 normalMat; //for normal

void main() {
    varyingTBNMatrix = mat3(normalize((normalMat * tangent).xyz), normalize((normalMat *
                    binormal).xyz), normalize((normalMat * normal).xyz));
    vec4 pos_ = position;
    varyingPosition = pos_.xyz;
    varyingUv = uv;
    vec4 pos = p*mvm*pos_;
    gl_Position = pos;
}
