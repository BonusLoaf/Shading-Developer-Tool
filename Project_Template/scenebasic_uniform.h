#ifndef SCENEBASIC_UNIFORM_H
#define SCENEBASIC_UNIFORM_H

#include "helper/scene.h"

#include <glad/glad.h>
#include "helper/glslprogram.h"
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include "helper/objmesh.h"
#include "helper/skybox.h"
#include <glm/glm.hpp>

class SceneBasic_Uniform : public Scene
{
private:

    glm::mat4 rotationMatrix;

    GLSLProgram prog;

    float angle, tPrev, rotSpeed;

    SkyBox sky;


    std::unique_ptr<ObjMesh> pyramid;

    std::unique_ptr<ObjMesh> staff;

    void setMatrices();

    void compile();

public:
    SceneBasic_Uniform();

    void initScene();
    void update( float t );
    void render();
    void resize(int, int);
};

#endif // SCENEBASIC_UNIFORM_H
