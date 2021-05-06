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
#include "helper/teapot.h"
#include "helper/torus.h"
#include "helper/plane.h"

#include <imgui-1.79/imgui.h>
#include <imgui-1.79/examples/imgui_impl_glfw.h>
#include <imgui-1.79/examples/imgui_impl_opengl3.h>

#include "GLFW/glfw3.h"
#include "GLFW/glfw3.h"
#include <glm/glm.hpp> //includes GLM
#include "glm/fwd.hpp"
#include <glm/gtc/type_ptr.hpp> // GLM: access to the value_ptr

class SceneBasic_Uniform : public Scene
{
private:

    glm::mat4 rotationMatrix;

    GLSLProgram prog, edgeProg, staffProg;

    GLuint fboHandle;

    GLuint fsQuad;
    GLuint renderFBO, intermediateFBO;
    GLuint renderTex, intermediateTex;


    float angle, tPrev, rotSpeed;

    SkyBox sky;

    Plane plane;


    std::unique_ptr<ObjMesh> pyramid;

    std::unique_ptr<ObjMesh> staff;

    void setMatrices();

    void renderGUI();

    void compile();
    void setupEdgeFBO();
    void setupGaussFBO();

    void pass1();
    void pass2();
    void pass3();
    float gauss(float, float);

    void activateEdgeShader();

public:
    SceneBasic_Uniform(GLFWwindow* sceneRunnerWindow);

    void initScene();
    void update( float t );
    void render();
    void resize(int, int);
    void controls();

};

#endif // SCENEBASIC_UNIFORM_H
