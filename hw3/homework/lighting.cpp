#include <GL/glew.h>
#include "glsupport.h"
#include <stdio.h>
#include "matrix4.h"
#include "quat.h"
#include "geometrymaker.h"

#include "Program.h"
#include "Uniform.h"
#include "BufferObject.h"
#include "LuaConfig.h"
#include "Attribute.h"

#include "Light.h"
#include <iterator>
#include <math.h>

#include <memory>
#define TINYOBJLOADER_IMPLEMENTATION
#include "tiny_obj_loader.h"

using namespace std;
//using VertexPNBuffer = detail::GlBufferObject<GL_ARRAY_BUFFER,VertexPN>;
struct VertexPNTBTG {
	Cvec3f p, n, b, tg;
	Cvec2f t;
	VertexPNTBTG() {}
	VertexPNTBTG(float x, float y, float z, float nx, float ny, float nz) : p(x, y, z), n(nx, ny, nz) {}
	VertexPNTBTG& operator = (const GenericVertex& v) {
		p = v.pos;
		n = v.normal;
		t = v.tex;
		b = v.binormal;
		tg = v.tangent;
		return *this;
	}
};
using vertex_t = VertexPNTBTG;
using VertexPNTBTGBuffer = detail::GlBufferObject<GL_ARRAY_BUFFER,vertex_t>;

//Some Intialization can only be started after glewInit() and glutInit()
//are called, so I can't put these objects into global static scope.
//One possible workaround is to use
//global pointers to main()'s on-stack objects, which won't be disposed
//until the whole program exits.
VertexPNTBTGBuffer *vbo, *floorvbo;
IndexBuffer * ibo, *flooribo;
Program* program;
UniformMatrix4fv *modelView, *projection, *normalMat;
Attribute * normal, *position ,*uv, *tangent, *binormal, *floorUV, *floorNormal;
LuaConfig * config;

//GLuint diffuseTex,normalTex,specularTex;

Light * light;

//Arcball related

int w=500,h=500;
Matrix4 world, rotWorld;
Cvec3 v0,v1;
double eye_x,eye_y,eye_z;
int mouseX,mouseY;
bool mouseDown,mouseUp;//these two have edge trigger semantics
bool pressed; //this has level trigger semantic
void calculateFaceTangent(
        const Cvec3f &v1, const Cvec3f &v2, const Cvec3f &v3, 
        const Cvec2f &texCoord1, const Cvec2f &texCoord2,
        const Cvec2f &texCoord3, Cvec3f &tangent, Cvec3f &binormal) {
    Cvec3f side0 = v1 - v2;
    Cvec3f side1 = v3 - v1;
    Cvec3f normal = cross(side1, side0);
    normalize(normal);
    float deltaV0 = texCoord1[1] - texCoord2[1];
    float deltaV1 = texCoord3[1] - texCoord1[1];
    tangent = side0 * deltaV1 - side1 * deltaV0;
    normalize(tangent);
    float deltaU0 = texCoord1[0] - texCoord2[0];
    float deltaU1 = texCoord3[0] - texCoord1[0];
    binormal = side0 * deltaU1 - side1 * deltaU0;
    normalize(binormal);
    Cvec3f tangentCross = cross(tangent, binormal);
    if (dot(tangentCross, normal) < 0.0f) {
        tangent = tangent * -1;
    }
}

void loadObjFile(const char *fileName, std::vector<vertex_t> &outVertices, std::vector<unsigned short> &outIndices) {
    tinyobj::attrib_t attrib;
    std::vector<tinyobj::shape_t> shapes;
    std::vector<tinyobj::material_t> materials;
    std::string err;
    bool ret = tinyobj::LoadObj(&attrib, &shapes, &materials, &err, fileName, NULL, true);
    bool hasTex = !attrib.texcoords.empty();
    if(ret) {
        for(unsigned int i=0; i < shapes.size(); i++) {
            for(unsigned int j=0; j < shapes[i].mesh.indices.size(); j++) {
                unsigned int vertexOffset = shapes[i].mesh.indices[j].vertex_index * 3;
                unsigned int normalOffset = shapes[i].mesh.indices[j].normal_index * 3;
                unsigned int texOffset = shapes[i].mesh.indices[j].texcoord_index * 2;
                vertex_t v;
                v.p[0] = attrib.vertices[vertexOffset];
                v.p[1] = attrib.vertices[vertexOffset+1];
                v.p[2] = attrib.vertices[vertexOffset+2];
                v.n[0] = attrib.normals[normalOffset];
                v.n[1] = attrib.normals[normalOffset+1];
                v.n[2] = attrib.normals[normalOffset+2];
                if (hasTex){
                    v.t[0] = attrib.texcoords[texOffset];
                    v.t[1] = 1.0-attrib.texcoords[texOffset+1];
                }
                outVertices.push_back(v);
                outIndices.push_back(outVertices.size()-1);
            }
        }
        for(unsigned int i=0; i < outVertices.size(); i += 3) {
            Cvec3f tangent;
            Cvec3f binormal;
            calculateFaceTangent(outVertices[i].p, outVertices[i+1].p, outVertices[i+2].p,
                    outVertices[i].t, outVertices[i+1].t, outVertices[i+2].t, tangent, binormal);
            outVertices[i].tg = tangent;
            outVertices[i+1].tg = tangent;
            outVertices[i+2].tg = tangent;
            outVertices[i].b = binormal;
            outVertices[i+1].b = binormal;
            outVertices[i+2].b = binormal;
        }
    } else {
        std::cout << err << std::endl;
        assert(false);
    }
}

void setUniformToTexture2D (Program * p, const char* imgPath, const char* uniformName,GLuint texUnit){
    Uniform1i diffuseu (p,uniformName);
    diffuseu.setValue(texUnit);
    glActiveTexture(GL_TEXTURE0 + texUnit);
    auto tex = loadGLTexture(imgPath);
    glBindTexture( GL_TEXTURE_2D, tex);
}

void mouseClick(int /*button*/,int state,int /*x*/,int/* y*/){
    if(state == GLUT_UP){
        mouseUp = true;
        pressed = false;
    }
    else if (state == GLUT_DOWN){
        mouseDown = true;
        pressed = true;
    }
}

void mouseMove (int x,int y){
    if (!pressed){
        return ;
    }
    double xInWorld = 2.0*x/w - 1;
    double yInWorld = 1.0 - 2.0*y/h;
    if (mouseDown){
        mouseDown = false;
        v0 = normalize(Cvec3(xInWorld,yInWorld,1));
        return;
    }
    else if (mouseUp){
        mouseUp =false;
        v0 = normalize(Cvec3(xInWorld,yInWorld,1));
        world=rotWorld*world;
        rotWorld = Matrix4();
        return;
    }
    v1 = normalize(Cvec3(xInWorld,yInWorld,1));
    Cvec3 op = cross(v0,v1);
    double ip = dot(v0,v1);
    Quat q( ip,op[0],op[1],op[2]);
    rotWorld = quatToMatrix(q);
    return ;
}

void idle(){
    glutPostRedisplay();
}

void drawModel ( ){
    Matrix4 modelMatrix = Matrix4( rotWorld*world);
    float colMajorMat[16];
    static LuaTable luaProjection = config->getLuaTable("projection"),
                    luaEyePosition = config->getLuaTable("eye");

    double fovy=luaProjection.get<double>(0);
    double aspectRatio=luaProjection.get<double>(1);
    double zNear=luaProjection.get<double>(2);
    double zFar=luaProjection.get<double>(3);
    eye_x = luaEyePosition.get<double>(0);
    eye_y = luaEyePosition.get<double>(1);
    eye_z = luaEyePosition.get<double>(2);
    Matrix4 p= Matrix4::makeProjection(fovy,aspectRatio,zNear,zFar);
    p.writeToColumnMajorMatrix(colMajorMat);
    projection->setValue(1,false,colMajorMat);

    Matrix4 eye=lookFrom(eye_x,eye_y,eye_z,0,1,0);
    Matrix4 modelViewMatrix = inv(eye)* modelMatrix;
    modelViewMatrix.writeToColumnMajorMatrix(colMajorMat);
    modelView->setValue(1,false,colMajorMat);

    //set normalMatrix
    Matrix4 n = normalMatrix(modelViewMatrix);
    n.writeToColumnMajorMatrix(colMajorMat);
    normalMat->setValue(1,false,colMajorMat);

#define offset(T,e) ((void*)&(((T*)0)->e))
    //draw
    vbo->bind();
    glVertexAttribPointer(position->get(), 3, GL_FLOAT, GL_FALSE, sizeof(vertex_t), offset(vertex_t,p));
    glVertexAttribPointer(normal->get(), 3, GL_FLOAT, GL_FALSE, sizeof(vertex_t), offset(vertex_t,n));
    glVertexAttribPointer(uv->get(), 2, GL_FLOAT, GL_FALSE, sizeof(vertex_t), offset(vertex_t,t));
    glVertexAttribPointer(tangent->get(), 3, GL_FLOAT, GL_FALSE, sizeof(vertex_t), offset(vertex_t,tg));
    glVertexAttribPointer(binormal->get(), 3, GL_FLOAT, GL_FALSE, sizeof(vertex_t), offset(vertex_t,b));
    ibo->bind();
    glDrawElements(GL_TRIANGLES,ibo->size(),GL_UNSIGNED_SHORT,0);
    
    floorvbo->bind();
    glVertexAttribPointer(position->get(), 3, GL_FLOAT, GL_FALSE, sizeof(vertex_t), offset(vertex_t,p));
    glVertexAttribPointer(normal->get(), 3, GL_FLOAT, GL_FALSE, sizeof(vertex_t), offset(vertex_t,n));
    glVertexAttribPointer(uv->get(), 2, GL_FLOAT, GL_FALSE, sizeof(vertex_t), offset(vertex_t,t));
    glVertexAttribPointer(tangent->get(), 3, GL_FLOAT, GL_FALSE, sizeof(vertex_t), offset(vertex_t,tg));
    glVertexAttribPointer(binormal->get(), 3, GL_FLOAT, GL_FALSE, sizeof(vertex_t), offset(vertex_t,b));
    flooribo->bind();
    glDrawElements(GL_TRIANGLES,flooribo->size()*0+6,GL_UNSIGNED_SHORT,(void*)(0+24));
#undef offset
}

void display(void)
{
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    drawModel();
	glutSwapBuffers();
}

void reshape(int _w,int _h){
    w=_w;
    h=_h;
    glViewport(0,0,w,h);
}

void init(int *argc, char* argv[])
{
    
    glutInit(argc,argv);
    glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGBA |GLUT_DEPTH);
    glutInitWindowSize(w,h);
    glutCreateWindow("Simple");
    glutDisplayFunc(display);
    glutReshapeFunc(reshape);
    glutMouseFunc(mouseClick);
    glutMotionFunc(mouseMove);
    glutIdleFunc(idle);

    glEnable(GL_BLEND);
    glEnable(GL_CULL_FACE);
    glCullFace(GL_BACK);
    glEnable(GL_DEPTH_TEST);
    glDepthFunc(GL_GREATER);
    glClearDepth( -100.0);
    glReadBuffer(GL_FRONT);
    glBlendFunc(GL_SRC_ALPHA,GL_ONE_MINUS_SRC_ALPHA);
    glClearColor(0.,0.,0.,1.);
    glewInit();
}

int main(int argc, char* argv[])
{
    init(&argc,argv);
    //on-stack objects, won't be disposed until program ends
    //#define ptr_to_stack(ptr,ctor) auto ptr##_ =(ctor);//ptr=& ptr##_ ;
    Program program_(  "vertex.glsl", "fragment.glsl");
    LuaConfig config_( "config.lua");
    Attribute position_( &program_,"position");
    Attribute normal_( &program_,"normal");
    Attribute uv_( &program_,"uv");
    Attribute tangent_( &program_,"tangent");
    Attribute binormal_( &program_,"binormal");
    //Attribute floorNormal_( &program_,"floorNormal");
    //Attribute floorUV_( &program_,"flooruv");
    //assert(!glGetError());

    UniformMatrix4fv modelView_(&program_, "mvm"),
        projection_(&program_,"p"),
        normalMat_(&program_,"normalMat");

    LightList lightList_(&program_,"lights",10);


    //Monk
    std::vector<vertex_t>verts;
    std::vector<unsigned short>indices;
    loadObjFile( 
            "Monk_Giveaway.obj",
            verts,indices);
    for (auto & v:verts){
        v.p = v.p *0.05;
    }
    
    VertexPNTBTGBuffer vbo_(verts.data(),verts.size());
    IndexBuffer ibo_(indices.data(),indices.size());

    //floor related
    int vbLen,ibLen;
    getCubeVbIbLen( vbLen,  ibLen) ;
    std::vector<vertex_t> floorVerts(vbLen);
    std::vector<unsigned short> floorIndices(ibLen);
    makeCube(6.0,floorVerts.begin(),floorIndices.begin());
    for (auto & v : floorVerts){
        v.p[1]-=3.0;
    }
    //floorVerts = vector<vertex_t> (floorVerts.begin()+8,floorVerts.begin()+12);
    //floorIndices = vector<unsigned short> (floorIndices.begin()+12, floorIndices.begin()+18);
    VertexPNTBTGBuffer floorvbo_(floorVerts.data(),floorVerts.size());
    IndexBuffer flooribo_(floorIndices.data(),floorVerts.size());
    
    program = &program_;
    position = &position_;
    normal = &normal_;
    uv =&uv_;
    tangent = & tangent_;
    binormal = &binormal_;
    //floorUV = & floorUV_;
    //floorNormal = &floorNormal_;

    modelView = &modelView_;
    projection = &projection_;
    normalMat = & normalMat_;
    vbo = &vbo_;
    ibo = &ibo_;
    flooribo = & flooribo_;
    floorvbo = & floorvbo_;
    config = &config_;

    program->useThis();

    lightList_[0].setPosition(-15.f,15.f,-5.f);
    lightList_[0].setDiffuseColor(.0f,0.4f,0.f);
    lightList_[0].setSpecularColor(1.0f,1.0f,1.f);
    lightList_[1].setPosition(14.f,14.f,-4.f);
    lightList_[1].setDiffuseColor(1.0f,.0f,0.f);
    lightList_[1].setSpecularColor(.0f,.0f,0.f);
    lightList_[2].setPosition(-0.f,15.f,-0.f);
    lightList_[2].setDiffuseColor(.0f,.0f,.0f);
    lightList_[2].setSpecularColor(1.0f,1.0f,1.f);

    //glActiveTexture sets the current active texture unit(GL_TEXTUREi)
    //for each texture unit, it has GL_TEXTURE_2D GL_TEXTURE_3D, etc.
    //I think this global state machine designe really increase the chance of
    //making mistakes!
    setUniformToTexture2D (program,  "Monk_D.tga","diffuseTex" ,0);
    setUniformToTexture2D (program,  "Monk_S.tga","specularTex" ,1);
    setUniformToTexture2D (program,  "Monk_N.tga","normalTex" ,2);
    //setUniformToTexture2D (program, CURRENT_DIR "/floor.jpg","floorTex" ,3);

    glutMainLoop();
	return 0;
}
