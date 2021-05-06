#include "scenebasic_uniform.h"

#include <imgui-1.79/imgui.h>
#include <imgui-1.79/examples/imgui_impl_glfw.h>
#include <imgui-1.79/examples/imgui_impl_opengl3.h>


#include "GLFW/glfw3.h"
#include "GLFW/glfw3.h"
#include <glm/glm.hpp> //includes GLM
#include "glm/fwd.hpp"
#include <glm/gtc/type_ptr.hpp> // GLM: access to the value_ptr


#include <cstdio>
#include <cstdlib>
#include <sstream>
#include <string>
using std::string;

#include "helper/texture.h"
#include "helper/objmesh.h"
#include <glm/glm.hpp>
#include <iostream>
using std::cerr;
using std::endl;


#include <glm/gtc/matrix_transform.hpp>
using glm::vec3;
using glm::vec4;
using glm::mat4;
using glm::mat3;

int shaderType = 1;

#include "helper/glutils.h"

std::unique_ptr<ObjMesh> pyramid;
std::unique_ptr<ObjMesh> staff;

using glm::vec3;

ImVec4 backgroundColour = ImVec4(0.06f, 0.015f, 0.0f, 1.0f);

float x, z;

GLFWwindow* window;

SceneBasic_Uniform::SceneBasic_Uniform(GLFWwindow* sceneRunnerWindow) : angle(0.0f), tPrev(0.0f), rotSpeed(-0.3f), /*sky(100.0f),*/ shadowMapWidth(512), shadowMapHeight(512), 
plane(40.0f, 40.0f, 2, 2), torus(0.7f * 2.0f, 0.3f * 2.0f, 50, 50), teapot(14, glm::mat4(1.0f))
{
    pyramid = ObjMesh::load("media/pyr.obj", false, true);
    staff = ObjMesh::load("media/staff.obj", true);

    window = sceneRunnerWindow;
   
}


void SceneBasic_Uniform::initScene()
{

    compile();
    glClearColor(0.5f, 0.7, 1.0f, 1.0f);
    glEnable(GL_DEPTH_TEST);
    projection = mat4(1.0f);

        glfwInit();

        glfwMakeContextCurrent(window);

        angle = glm::quarter_pi<float>();

        //view = glm::lookAt(vec3(0.5f, 0.75f, 0.75f), vec3(0.0f, 0.0f, 0.0f), vec3(0.0f, 1.0f, 0.0f));

        setupGaussFBO();

        // Array for full-screen quad
        GLfloat verts[] = {
    -1.0f, -1.0f, 0.0f, 1.0f, -1.0f, 0.0f, 1.0f, 1.0f, 0.0f,
    -1.0f, -1.0f, 0.0f, 1.0f, 1.0f, 0.0f, -1.0f, 1.0f, 0.0f
        };
        GLfloat tc[] = {
        0.0f, 0.0f, 1.0f, 0.0f, 1.0f, 1.0f,
        0.0f, 0.0f, 1.0f, 1.0f, 0.0f, 1.0f
        };

        // Set up the buffers
        unsigned int handle[2];
        glGenBuffers(2, handle);
        glBindBuffer(GL_ARRAY_BUFFER, handle[0]);
        glBufferData(GL_ARRAY_BUFFER, 6 * 3 * sizeof(float), verts, GL_STATIC_DRAW);
        glBindBuffer(GL_ARRAY_BUFFER, handle[1]);
        glBufferData(GL_ARRAY_BUFFER, 6 * 2 * sizeof(float), tc, GL_STATIC_DRAW);
        // Set up the vertex array object
        glGenVertexArrays(1, &fsQuad);
        glBindVertexArray(fsQuad);
        glBindBuffer(GL_ARRAY_BUFFER, handle[0]);
        glVertexAttribPointer((GLuint)0, 3, GL_FLOAT, GL_FALSE, 0, 0);
        glEnableVertexAttribArray(0); // Vertex position
        glBindBuffer(GL_ARRAY_BUFFER, handle[1]);
        glVertexAttribPointer((GLuint)2, 2, GL_FLOAT, GL_FALSE, 0, 0);
        glEnableVertexAttribArray(2); // Texture coordinates
        glBindVertexArray(0);


        prog.setUniform("Light.Position", (glm::vec4(x, 5.0f, 0.0, 0.0f)));
        prog.setUniform("Light.L", vec3(1.0f));
        prog.setUniform("Light.La", vec3(0.6f));

        float weights[5], sum, sigma2 = 8.0f;
        // Compute and sum the weights
        weights[0] = gauss(0, sigma2);
        sum = weights[0];
        for (int i = 1; i < 5; i++) {
            weights[i] = gauss(float(i), sigma2);
            sum += 2 * weights[i];
        }
        // Normalize the weights and set the uniform
        for (int i = 0; i < 5; i++) {
            std::stringstream uniName;
            uniName << "Weight[" << i << "]";
            float val = weights[i] / sum;
            prog.setUniform(uniName.str().c_str(), val);
        }



        ////Shadow Buffers
        //GLuint programHandle = prog.getHandle();
        //pass1Index = glGetSubroutineIndex(programHandle, GL_FRAGMENT_SHADER, "recordDepth");
        //pass1Index = glGetSubroutineIndex(programHandle, GL_FRAGMENT_SHADER, "shadeWithShadow");

        //shadowBias = mat4(
        //    vec4(0.5f, 0.0f, 0.0f, 0.0f),
        //    vec4(0.0f, 0.5f, 0.0f, 0.0f),
        //    vec4(0.0f, 0.0f, 0.5f, 0.0f),
        //    vec4(0.5f, 0.5f, 0.5f, 1.0f)
        //);

        //float c = 1.65f;
        //vec3 lightPos = vec3(0.0f, c * 5.25, c * 7.5); //world coords
        //lightFrustum.orient(lightPos, vec3(0.0f), vec3(0.0f, 1.0f, 0.0f));
        //lightFrustum.setPerspective(50.0f, 1.0f, 1.0f, 25.0f);
        //lightPV = shadowBias * lightFrustum.getProjectionMatrix() * lightFrustum.getViewMatrix();

        //prog.setUniform("Light.Intensity", vec3(0.85f));
        //prog.setUniform("ShadowMap", 0);
        

        //GUI initialisation
        IMGUI_CHECKVERSION();
        ImGui::CreateContext();
        ImGuiIO& io = ImGui::GetIO(); (void)io;


        ImGui::StyleColorsDark();

        ImGui_ImplGlfw_InitForOpenGL(window, true);
        ImGui_ImplOpenGL3_Init();

        

        
        

       // //Locate all textures
       // GLuint skybox = Texture::loadCubeMap("media/skybox/sand", ".png");

       // GLuint pyBricks = Texture::loadTexture("media/texture/bricks.jpg");

       // GLuint staff = Texture::loadTexture("media/texture/red.png");

       // //GLuint normalMap = Texture::loadTexture("media/texture/normalMap.png");

       // GLuint dirt = Texture::loadTexture("media/texture/dirt.png");

       // GLuint sand = Texture::loadTexture("media/texture/sand.png");

       // // Load brick texture file into channel 5
       ///* glActiveTexture(GL_TEXTURE5);
       // glBindTexture(GL_TEXTURE_2D, skybox);*/

       // // Load texture file into channel 1
       // glActiveTexture(GL_TEXTURE1);
       // glBindTexture(GL_TEXTURE_2D, pyBricks);

       // //staffProg.use();

       // // Load texture file into channel 2
       // glActiveTexture(GL_TEXTURE2);
       // glBindTexture(GL_TEXTURE_2D, staff);

       // // Load texture file into channel 3
       // glActiveTexture(GL_TEXTURE3);
       // glBindTexture(GL_TEXTURE_2D, dirt);


       // // Load texture file into channel 4
       // glActiveTexture(GL_TEXTURE4);
       // glBindTexture(GL_TEXTURE_2D, sand);

        


}

float SceneBasic_Uniform::gauss(float x, float sigma2)
{
    double coeff = 1.0 / (glm::two_pi<double>() * sigma2);
    double expon = -(x * x) / (2.0 * sigma2);
    return (float)(coeff * exp(expon));
}

void SceneBasic_Uniform::controls()
{
    if (glfwGetKey(window, GLFW_KEY_1) == GLFW_PRESS)
    {

        shaderType = 1;
        prog.use();


        //angle = glm::radians(90.0f);



        view = glm::lookAt(vec3(0.5f, 0.75f, 0.75f), vec3(5.0f, 0.0f, 0.0f), vec3(0.0f, 1.0f, 0.0f));
        //projection = mat4(1.0f);

        //

        ////Locate all textures
        //GLuint skybox = Texture::loadCubeMap("media/skybox/sand", ".png");

        //GLuint pyBricks = Texture::loadTexture("media/texture/bricks.jpg");

        //GLuint staff = Texture::loadTexture("media/texture/red.png");

        //GLuint normalMap = Texture::loadTexture("media/texture/normalMap.png");

        //GLuint dirt = Texture::loadTexture("media/texture/dirt.png");

        //// Load brick texture file into channel 0
        //glActiveTexture(GL_TEXTURE0);
        //glBindTexture(GL_TEXTURE_2D, skybox);

        //// Load texture file into channel 1
        //glActiveTexture(GL_TEXTURE1);
        //glBindTexture(GL_TEXTURE_2D, pyBricks);


        //// Load texture file into channel 2
        //glActiveTexture(GL_TEXTURE2);
        //glBindTexture(GL_TEXTURE_2D, staff);

        //// Load texture file into channel 3
        //glActiveTexture(GL_TEXTURE3);
        //glBindTexture(GL_TEXTURE_2D, normalMap);

        //// Load texture file into channel 4
        //glActiveTexture(GL_TEXTURE4);
        //glBindTexture(GL_TEXTURE_2D, dirt);


    }
    else if (glfwGetKey(window, GLFW_KEY_2) == GLFW_PRESS)
    {
        shaderType = 2;


        glEnable(GL_DEPTH_TEST);
        projection = mat4(1.0f);
        angle = glm::pi<float>() / 4.0f;
        setupEdgeFBO();
        // Array for full-screen quad
        GLfloat verts[] = {
        -1.0f, -1.0f, 0.0f, 1.0f, -1.0f, 0.0f, 1.0f, 1.0f, 0.0f,
        -1.0f, -1.0f, 0.0f, 1.0f, 1.0f, 0.0f, -1.0f, 1.0f, 0.0f
        };
        GLfloat tc[] = {
        0.0f, 0.0f, 1.0f, 0.0f, 1.0f, 1.0f,
        0.0f, 0.0f, 1.0f, 1.0f, 0.0f, 1.0f
        };
        // Set up the buffers
        unsigned int handle[2];
        glGenBuffers(2, handle);
        glBindBuffer(GL_ARRAY_BUFFER, handle[0]);
        glBufferData(GL_ARRAY_BUFFER, 6 * 3 * sizeof(float), verts, GL_STATIC_DRAW);
        glBindBuffer(GL_ARRAY_BUFFER, handle[1]);
        glBufferData(GL_ARRAY_BUFFER, 6 * 2 * sizeof(float), tc, GL_STATIC_DRAW);
        // Set up the vertex array object
        glGenVertexArrays(1, &fsQuad);
        glBindVertexArray(fsQuad);
        glBindBuffer(GL_ARRAY_BUFFER, handle[0]);
        glVertexAttribPointer((GLuint)0, 3, GL_FLOAT, GL_FALSE, 0, 0);
        glEnableVertexAttribArray(0); // Vertex position
        glBindBuffer(GL_ARRAY_BUFFER, handle[1]);
        glVertexAttribPointer((GLuint)2, 2, GL_FLOAT, GL_FALSE, 0, 0);
        glEnableVertexAttribArray(2); // Texture coordinates
        glBindVertexArray(0);
        edgeProg.setUniform("EdgeThreshold", 0.05f);
        edgeProg.setUniform("Light.L", vec3(1.0f));
        edgeProg.setUniform("Light.La", vec3(0.2f));


        edgeProg.use();

        //Add point light
        edgeProg.setUniform("Light.Position", (view * glm::vec4(x, 5.0f, 0.0, 0.0f)));

        edgeProg.setUniform("Light.L", vec3(0.2f));

        edgeProg.setUniform("Light.La", vec3(0.9f, 0.2f, 0.1f));


    }
    else if (glfwGetKey(window, GLFW_KEY_0) == GLFW_PRESS)
    {
        shaderType = 0;
        glUseProgram(0);
    }

}

void SceneBasic_Uniform::compile()
{
	try {
		prog.compileShader("shader/basicshadow_uniform.vert");
		prog.compileShader("shader/basicshadow_uniform.frag");
		prog.link();
		prog.use();


        edgeProg.compileShader("shader/edge_uniform.vert");
        edgeProg.compileShader("shader/edge_uniform.frag");
        edgeProg.link();
        
        staffProg.compileShader("shader/staff_uniform.vert");
        staffProg.compileShader("shader/staff_uniform.frag");
        staffProg.link();

        //used when rendering light frustum
        solidProg.compileShader("shader/solid.vert", GLSLShader::VERTEX);
        solidProg.compileShader("shader/solid.frag", GLSLShader::FRAGMENT);
        solidProg.link();

        
	} catch (GLSLProgramException &e) {
		cerr << e.what() << endl;
		exit(EXIT_FAILURE);
	}
}

void SceneBasic_Uniform::update( float t )
{
        

    
        float deltaT = t - tPrev;
        if (tPrev == 0.0f)
            deltaT = 0.0f;
        tPrev = t;
        angle += rotSpeed * deltaT;
        if (angle > glm::two_pi<float>())
            angle -= glm::two_pi<float>();
    

    

}


void SceneBasic_Uniform::activateEdgeShader()
{
    shaderType = 2;


    glEnable(GL_DEPTH_TEST);
    projection = mat4(1.0f);
    angle = glm::pi<float>() / 4.0f;
    setupEdgeFBO();
    // Array for full-screen quad
    GLfloat verts[] = {
    -1.0f, -1.0f, 0.0f, 1.0f, -1.0f, 0.0f, 1.0f, 1.0f, 0.0f,
    -1.0f, -1.0f, 0.0f, 1.0f, 1.0f, 0.0f, -1.0f, 1.0f, 0.0f
    };
    GLfloat tc[] = {
    0.0f, 0.0f, 1.0f, 0.0f, 1.0f, 1.0f,
    0.0f, 0.0f, 1.0f, 1.0f, 0.0f, 1.0f
    };
    // Set up the buffers
    unsigned int handle[2];
    glGenBuffers(2, handle);
    glBindBuffer(GL_ARRAY_BUFFER, handle[0]);
    glBufferData(GL_ARRAY_BUFFER, 6 * 3 * sizeof(float), verts, GL_STATIC_DRAW);
    glBindBuffer(GL_ARRAY_BUFFER, handle[1]);
    glBufferData(GL_ARRAY_BUFFER, 6 * 2 * sizeof(float), tc, GL_STATIC_DRAW);
    // Set up the vertex array object
    glGenVertexArrays(1, &fsQuad);
    glBindVertexArray(fsQuad);
    glBindBuffer(GL_ARRAY_BUFFER, handle[0]);
    glVertexAttribPointer((GLuint)0, 3, GL_FLOAT, GL_FALSE, 0, 0);
    glEnableVertexAttribArray(0); // Vertex position
    glBindBuffer(GL_ARRAY_BUFFER, handle[1]);
    glVertexAttribPointer((GLuint)2, 2, GL_FLOAT, GL_FALSE, 0, 0);
    glEnableVertexAttribArray(2); // Texture coordinates
    glBindVertexArray(0);
    edgeProg.setUniform("EdgeThreshold", 0.05f);
    edgeProg.setUniform("Light.L", vec3(1.0f));
    edgeProg.setUniform("Light.La", vec3(0.2f));


    edgeProg.use();

    //Add point light
    edgeProg.setUniform("Light.Position", (view * glm::vec4(x, 5.0f, 0.0, 0.0f)));

    edgeProg.setUniform("Light.L", vec3(0.2f));

    edgeProg.setUniform("Light.La", vec3(0.9f, 0.2f, 0.1f));
}

void SceneBasic_Uniform::renderGUI()
{

	ImGui_ImplOpenGL3_NewFrame();
	ImGui_ImplGlfw_NewFrame();
	ImGui::NewFrame();



	glm::vec3 camZoom = glm::vec3(0, 0, 2);

	static float backgroundValue = 0.0f;
	static int counter = 0;

	ImGui::Begin("Customisation Menu");

	ImGui::Text("---------Camera---------");

    if (ImGui::Button("<"))
    {
        rotSpeed = 0.3;
    }
    ImGui::SameLine();
    if (ImGui::Button("Stop"))
    {
        rotSpeed = 0;
    }
    ImGui::SameLine();
    if (ImGui::Button(">"))
    {
        rotSpeed = -0.3;
    }
    
    ImGui::Text("---------Shaders---------");

    if (ImGui::Button("Edge Shader"))
    {
        activateEdgeShader();
    }


	ImGui::End();


	




	ImGui::Render();
	int display_w, display_h;
	ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

}

void SceneBasic_Uniform::render()
{
    
    glfwMakeContextCurrent(window);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    prog.use();
    //pass 1: shadow map gen
    view = lightFrustum.getViewMatrix();
    projection = lightFrustum.getProjectionMatrix();
    glBindFramebuffer(GL_FRAMEBUFFER, shadowFBO);
    glClear(GL_DEPTH_BUFFER_BIT);
    glViewport(0, 0, shadowMapWidth, shadowMapHeight);
    glUniformSubroutinesuiv(GL_FRAGMENT_SHADER, 1, &pass1Index);
    glEnable(GL_CULL_FACE);
    glCullFace(GL_FRONT);
    glEnable(GL_POLYGON_OFFSET_FILL);
    glPolygonOffset(2.5f, 10.0f);
    drawScene();
    glCullFace(GL_BACK);
    glFlush();


    //pass 2: render
    float c = 2.0f;
    vec3 cameraPos(c * 11.5f * cos(angle), c * 7.0f, c * 11.5f * sin(angle));
    view = glm::lookAt(cameraPos, vec3(0.0), vec3(0.0f, 1.0f, 0.0f));
    prog.setUniform("Light.Position", view * vec4(lightFrustum.getOrigin(), 1.0f));
    projection = glm::perspective(glm::radians(50.0f), (float)width / height, 0.1f, 100.0f);

    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glViewport(0, 0, width, height);
    glUniformSubroutinesuiv(GL_FRAGMENT_SHADER, 1, &pass2Index);
    drawScene();

    //draw the lights frustum
    solidProg.use();
    solidProg.setUniform("Color", vec4(1.0f, 0.0f, 0.0f, 1.0f));
    mat4 mv = view * lightFrustum.getInverseViewMatrix();
    solidProg.setUniform("MVP", projection * mv);
    lightFrustum.render();

    /*pass1();
    pass2();

    if (shaderType == 1)
    {
        prog.use();
        pass3();
    }
    else
    {
        edgeProg.use();
        glFlush();
    }*/

    //staffProg.use();

    controls();
        //prog.use();
        renderGUI();
        glfwPollEvents();
    
       

    


}

void SceneBasic_Uniform::setMatrices()
{
    if (shaderType == 1)
    {
        /*mat4 mv;
        mv = view * model;

        prog.setUniform("ModelViewMatrix", mv);
        prog.setUniform("NormalMatrix", glm::mat3(vec3(mv[0]), vec3(mv[1]), vec3(mv[2])));
        prog.setUniform("MVP", projection * mv);*/

        mat4 mv = view * model;
        prog.setUniform("ModelViewMatrix", mv);
        prog.setUniform("NormalMatrix",
            glm::mat3(vec3(mv[0]), vec3(mv[1]), vec3(mv[2])));
        prog.setUniform("MVP", projection * mv);
        prog.setUniform("ShadowMatrix", lightPV * model);
    }
    else
    {

        mat4 mv = view * model;
        edgeProg.setUniform("ModelMatrix", model);
        edgeProg.setUniform("ModelViewMatrix", mv);
        edgeProg.setUniform("NormalMatrix", glm::mat3(vec3(mv[0]), vec3(mv[1]),
            vec3(mv[2])));
        edgeProg.setUniform("MVP", projection * mv);
    }

}

void SceneBasic_Uniform::resize(int w, int h)
{
    glViewport(0, 0, w, h);
    width = w;
    height = h;
    projection = glm::perspective(glm::radians(70.0f), (float)w / h,
        0.3f, 100.0f);
}


void SceneBasic_Uniform::setupEdgeFBO()
{
    // Generate and bind the framebuffer
    glGenFramebuffers(1, &fboHandle);
    glBindFramebuffer(GL_FRAMEBUFFER, fboHandle);
    // Create the texture object
    glGenTextures(1, &renderTex);
    glBindTexture(GL_TEXTURE_2D, renderTex);
    glTexStorage2D(GL_TEXTURE_2D, 1, GL_RGBA8, width, height);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAX_LEVEL, 0);
    // Bind the texture to the FBO
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D,
        renderTex, 0);
    // Create the depth buffer
    GLuint depthBuf;
    glGenRenderbuffers(1, &depthBuf);
    glBindRenderbuffer(GL_RENDERBUFFER, depthBuf);
    glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT, width, height);
    // Bind the depth buffer to the FBO
    glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT,
        GL_RENDERBUFFER, depthBuf);
    // Set the targets for the fragment output variables
    GLenum drawBuffers[] = { GL_COLOR_ATTACHMENT0 };
    glDrawBuffers(1, drawBuffers);
    // Unbind the framebuffer, and revert to default framebuffer
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
}


void SceneBasic_Uniform::setupGaussFBO()
{
   
    
   





    //----------------------------------------------------------Shadow FBO--------------------------------------
    GLfloat border[] = { 1.0f, 0.0f, 0.0f, 0.0f };
    //the depth buffer texture
    GLuint depthTex;
    glGenTextures(1, &depthTex);
    glBindTexture(GL_TEXTURE_2D, depthTex);
    glTexStorage2D(GL_TEXTURE_2D, 1, GL_DEPTH_COMPONENT24, shadowMapWidth, shadowMapHeight);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);
    glTexParameterfv(GL_TEXTURE_2D, GL_TEXTURE_BORDER_COLOR, border);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_COMPARE_MODE, GL_COMPARE_REF_TO_TEXTURE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_COMPARE_FUNC, GL_LESS);

    //assign the depth buffer texture to texture channel 0
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, depthTex);

    //create and set up the FBO
    glGenFramebuffers(1, &shadowFBO);
    glBindFramebuffer(GL_FRAMEBUFFER, shadowFBO);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT,
        GL_TEXTURE_2D, depthTex, 0);


    GLenum drawBuffers[] = { GL_NONE };
    glDrawBuffers(1, drawBuffers);


    GLenum result = glCheckFramebufferStatus(GL_FRAMEBUFFER);
    if (result == GL_FRAMEBUFFER_COMPLETE)
    {
        printf("Framebuffer is complete. \n");
    }
    else
    {
        printf("Framebuffer is not complete. \n");
    }

    glBindFramebuffer(GL_FRAMEBUFFER, 0);



    // Generate and bind the framebuffer
    glGenFramebuffers(1, &renderFBO);
    glBindFramebuffer(GL_FRAMEBUFFER, renderFBO);
    // Create the texture object
    glGenTextures(1, &renderTex);
    glBindTexture(GL_TEXTURE_2D, renderTex);
    glTexStorage2D(GL_TEXTURE_2D, 1, GL_RGBA8, width, height);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAX_LEVEL, 0);
    // Bind the texture to the FBO
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D,
        renderTex, 0);
    // Create the depth buffer
    GLuint depthBuf;
    glGenRenderbuffers(1, &depthBuf);
    glBindRenderbuffer(GL_RENDERBUFFER, depthBuf);
    glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT, width, height);
    // Bind the depth buffer to the FBO
    glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT,
        GL_RENDERBUFFER, depthBuf);
    // Set the targets for the fragment output variables
    GLenum drawGaussBuffers[] = { GL_COLOR_ATTACHMENT0 };
    glDrawBuffers(1, drawGaussBuffers);
    // Unbind the framebuffer, and revert to default framebuffer
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    // Generate and bind the framebuffer
    glGenFramebuffers(1, &intermediateFBO);
    glBindFramebuffer(GL_FRAMEBUFFER, intermediateFBO);
    // Create the texture object
    glGenTextures(1, &intermediateTex);
    glActiveTexture(GL_TEXTURE0); // Use texture unit 0
    glBindTexture(GL_TEXTURE_2D, intermediateTex);
    glTexStorage2D(GL_TEXTURE_2D, 1, GL_RGBA8, width, height);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAX_LEVEL, 0);
    // Bind the texture to the FBO
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D,
        intermediateTex, 0);
    // No depth buffer needed for this FBO
    // Set the targets for the fragment output variables
    glDrawBuffers(1, drawGaussBuffers);
    // Unbind the framebuffer, and revert to default framebuffer
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

void SceneBasic_Uniform::drawScene()
{

    vec3 color = vec3(0.2f, 0.5f, 0.9f);
    prog.setUniform("Material.Ka", color * 0.05f);
    prog.setUniform("Material.Kd", color);
    prog.setUniform("Material.Ks", vec3(0.9f, 0.9f, 0.9f));
    prog.setUniform("Material.Shininess", 150.0f);
    model = mat4(1.0f);
    model = glm::rotate(model, glm::radians(-90.0f), vec3(1.0f, 0.0f, 0.0f));
    setMatrices();
    teapot.render();

    prog.setUniform("Material.Ka", color * 0.05f);
    prog.setUniform("Material.Kd", color);
    prog.setUniform("Material.Ks", vec3(0.9f, 0.9f, 0.9f));
    prog.setUniform("Material.Shininess", 150.0f);
    model = mat4(1.0f);
    model = glm::translate(model, vec3(0.0f, 2.0f, 5.0f));
    model = glm::rotate(model, glm::radians(-45.0f), vec3(1.0f, 0.0f, 0.0f));
    setMatrices();
    torus.render();


    prog.setUniform("Material.Kd", 0.25f, 0.25f, 0.25f);
    prog.setUniform("Material.Ks", 0.0f, 0.0f, 0.0f);
    prog.setUniform("Material.Ka", 0.05f, 0.05f, 0.05f);
    prog.setUniform("Material.Shininess", 1.0f);
    model = mat4(1.0f);
    setMatrices();
    plane.render();

    model = mat4(1.0f);
    model = glm::translate(model, vec3(-5.0f, 5.0f, 0.0f));
    model = glm::rotate(model, glm::radians(-90.0f), vec3(0.0f, 0.0f, 1.0f));
    setMatrices();
    plane.render();

    model = mat4(1.0f);
    model = glm::translate(model, vec3(0.0f, 5.0f, -5.0f));
    model = glm::rotate(model, glm::radians(90.0f), vec3(1.0f, 0.0f, 0.0f));
    setMatrices();
    plane.render();
    model = mat4(1.0f);
}

void SceneBasic_Uniform::pass1()
{
    if (shaderType == 1)
    {
        prog.use();
        vec3 color = vec3(0.2f, 0.5f, 0.9f);
        prog.setUniform("Pass", 1);

        glBindFramebuffer(GL_FRAMEBUFFER, renderFBO);
        glEnable(GL_DEPTH_TEST);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        view = glm::lookAt(vec3(7.0f * cos(angle), 4.0f, 7.0f * sin(angle)),
            vec3(0.0f, 0.0f, 0.0f), vec3(0.0f, 1.0f, 0.0f));
        projection = glm::perspective(glm::radians(60.0f), (float)width / height,
            0.3f, 100.0f);


        //Sets parameters for model and renders them
        prog.setUniform("texID", 1);
        //prog.setUniform("Light.Position", vec4(0.0f, 0.0f, 0.0f, 1.0f));
        prog.setUniform("Material.Kd", 0.8f, 0.8f, 0.8f);
        prog.setUniform("Material.Ks", 0.2f, 0.2f, 0.2f);
        prog.setUniform("Material.Ka", 0.2f, 0.2f, 0.2f);
        prog.setUniform("Material.Shininess", 5.0f);
        model = mat4(1.0f);
        model = glm::translate(model, vec3(0.0f, -1.0f, 0.0f));
        model = glm::rotate(model, glm::radians(90.0f), vec3(0.0f, 1.0f, 0.0f));
        model = glm::scale(model, glm::vec3(0.3f, 0.3f, 0.3f));
        setMatrices();
        pyramid->render();


        //staffProg.use();

        

        //Sets parameters for model and renders them
        //Also Changes texID to choose a different texture
        prog.setUniform("texID", 2);
       // prog.setUniform("Light.Position", vec4(0.0f, 0.0f, 0.0f, 1.0f));
        prog.setUniform("Material.Kd", 0.4f, 0.4f, 0.4f);
        prog.setUniform("Material.Ks", 0.9f, 0.9f, 0.9f);
        prog.setUniform("Material.Ka", 0.5f, 0.5f, 0.5f);
        prog.setUniform("Material.Shininess", 180.0f);
        model = mat4(1.0f);
        model = glm::translate(model, vec3(4.0f, 0.0f, 0.0f));
        model = glm::rotate(model, glm::radians(90.0f), vec3(0.0f, 1.0f, 0.0f));
        model = glm::scale(model, glm::vec3(25.0f, 12.0f, 25.0f));
        setMatrices();
        staff->render();

        prog.setUniform("texID", 3);
        //prog.setUniform("Light.Position", vec4(0.0f, 0.0f, 0.0f, 1.0f));
        prog.setUniform("Material.Kd", 0.4f, 0.4f, 0.4f);
        prog.setUniform("Material.Ks", 0.0f, 0.0f, 0.0f);
        prog.setUniform("Material.Ka", 0.1f, 0.1f, 0.1f);
        prog.setUniform("Material.Shininess", 1.0f);
        model = mat4(1.0f);
        model = glm::translate(model, vec3(0.0f, -0.75f, 0.0f));
        setMatrices();
        plane.render();

        /*vec3 cameraPos = vec3(7.0f * cos(angle), 2.0f, 7.0f * sin(angle));
        view = glm::lookAt(cameraPos, vec3(0.0f, 0.0f, 0.0f), vec3(0.0f, 1.0f,
            0.0f));*/

        //Sets parameters for skybox and renders it
        /*prog.setUniform("texID", 0);
        model = mat4(1.0f);
        setMatrices();
        sky.render();*/
    }
    else
    {
        edgeProg.use();

        edgeProg.setUniform("Pass", 1);
        glBindFramebuffer(GL_FRAMEBUFFER, fboHandle);
        glEnable(GL_DEPTH_TEST);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        vec3 cameraPos = vec3(7.0f * cos(angle), 2.0f, 7.0f * sin(angle));
        view = glm::lookAt(cameraPos, vec3(0.0f, 0.0f, 0.0f), vec3(0.0f, 1.0f,
            0.0f));
        projection = glm::perspective(glm::radians(60.0f), (float)width / height,
            0.3f, 100.0f);


        //Sets parameters for model and renders them
        prog.setUniform("Light.Position", vec4(0.0f, 0.0f, 0.0f, 1.0f));
        edgeProg.setUniform("Material.Kd", 0.8f, 0.8f, 0.8f);
        edgeProg.setUniform("Material.Ks", 0.2f, 0.2f, 0.2f);
        edgeProg.setUniform("Material.Ka", 0.2f, 0.2f, 0.2f);
        edgeProg.setUniform("Material.Shininess", 5.0f);
        model = mat4(1.0f);
        model = glm::translate(model, vec3(0.0f, -1.0f, 0.0f));
        model = glm::rotate(model, glm::radians(90.0f), vec3(0.0f, 1.0f, 0.0f));
        model = glm::scale(model, glm::vec3(0.3f, 0.3f, 0.3f));
        setMatrices();
        pyramid->render();


        //Sets parameters for model and renders them
        //Also Changes texID to choose a different texture
        prog.setUniform("Light.Position", vec4(0.0f, 0.0f, 0.0f, 1.0f));
        edgeProg.setUniform("texID", 2);
        edgeProg.setUniform("Material.Kd", 0.4f, 0.4f, 0.4f);
        edgeProg.setUniform("Material.Ks", 0.9f, 0.9f, 0.9f);
        edgeProg.setUniform("Material.Ka", 0.5f, 0.5f, 0.5f);
        edgeProg.setUniform("Material.Shininess", 180.0f);
        model = mat4(1.0f);
        model = glm::translate(model, vec3(4.0f, 0.0f, 0.0f));
        model = glm::rotate(model, glm::radians(90.0f), vec3(0.0f, 1.0f, 0.0f));
        model = glm::scale(model, glm::vec3(25.0f, 12.0f, 25.0f));
        setMatrices();
        staff->render();


        /*edgeProg.setUniform("Pass", 1);
        glBindFramebuffer(GL_FRAMEBUFFER, fboHandle);
        glEnable(GL_DEPTH_TEST);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        view = glm::lookAt(vec3(7.0f * cos(angle), 4.0f, 7.0f * sin(angle)),
            vec3(0.0f, 0.0f, 0.0f), vec3(0.0f, 1.0f, 0.0f));
        projection = glm::perspective(glm::radians(60.0f), (float)width / height,
            0.3f, 100.0f);
        edgeProg.setUniform("Light.Position", vec4(0.0f, 0.0f, 0.0f, 1.0f));
        edgeProg.setUniform("Material.Kd", 0.9f, 0.9f, 0.9f);
        edgeProg.setUniform("Material.Ks", 0.95f, 0.95f, 0.95f);
        edgeProg.setUniform("Material.Ka", 0.1f, 0.1f, 0.1f);
        edgeProg.setUniform("Material.Shininess", 100.0f);
        model = mat4(1.0f);
        model = glm::rotate(model, glm::radians(-90.0f), vec3(1.0f, 0.0f, 0.0f));
        setMatrices();
        teapot.render();
        edgeProg.setUniform("Light.Position", vec4(0.0f, 0.0f, 0.0f, 1.0f));
        edgeProg.setUniform("Material.Kd", 0.9f, 0.5f, 0.2f);
        edgeProg.setUniform("Material.Ks", 0.95f, 0.95f, 0.95f);
        edgeProg.setUniform("Material.Ka", 0.1f, 0.1f, 0.1f);
        edgeProg.setUniform("Material.Shininess", 100.0f);
        model = mat4(1.0f);
        model = glm::translate(model, vec3(1.0f, 1.0f, 3.0f));
        model = glm::rotate(model, glm::radians(90.0f), vec3(1.0f, 0.0f, 0.0f));
        setMatrices();
        torus.render();*/


    }



}

void SceneBasic_Uniform::pass2()
{

    if (shaderType == 1)
    {
        prog.use();
        prog.setUniform("Pass", 2);
        glBindFramebuffer(GL_FRAMEBUFFER, intermediateFBO);
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, renderTex);
        glDisable(GL_DEPTH_TEST);
        glClear(GL_COLOR_BUFFER_BIT);
        model = mat4(1.0f);
        view = mat4(1.0f);
        projection = mat4(1.0f);
        setMatrices();
        // Render the full-screen quad
        glBindVertexArray(fsQuad);
        glDrawArrays(GL_TRIANGLES, 0, 6);

    }
    else
    {
        edgeProg.use();
        edgeProg.setUniform("Pass", 2);
        glBindFramebuffer(GL_FRAMEBUFFER, 0);
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, renderTex);
        glDisable(GL_DEPTH_TEST);
        glClear(GL_COLOR_BUFFER_BIT);
        model = mat4(1.0f);
        view = mat4(1.0f);
        projection = mat4(1.0f);
        setMatrices();
        // Render the full-screen quad
        glBindVertexArray(fsQuad);
        glDrawArrays(GL_TRIANGLES, 0, 6);
        glBindVertexArray(0);
    }
}

void SceneBasic_Uniform::pass3()
{
    prog.setUniform("Pass", 3);
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, intermediateTex);
    glClear(GL_COLOR_BUFFER_BIT);
    model = mat4(1.0f);
    view = mat4(1.0f);
    projection = mat4(1.0f);
    setMatrices();
    // Render the full-screen quad
    glBindVertexArray(fsQuad);
    glDrawArrays(GL_TRIANGLES, 0, 6);
}