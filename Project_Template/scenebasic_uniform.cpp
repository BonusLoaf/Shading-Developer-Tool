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

SceneBasic_Uniform::SceneBasic_Uniform(GLFWwindow* sceneRunnerWindow) : angle(0.0f), tPrev(0.0f), rotSpeed(-0.3f), sky(100.0f)
{
    pyramid = ObjMesh::load("media/pyr.obj", false, true);
    staff = ObjMesh::load("media/staff.obj", true);

    window = sceneRunnerWindow;
   
}



void SceneBasic_Uniform::initScene()
{

    compile();
    
    if (shaderType == 1)
    {
        prog.use();
    }
    else
    {
        edgeProg.use();
    }

        glfwInit();

        glfwMakeContextCurrent(window);

        glEnable(GL_DEPTH_TEST);

        angle = glm::radians(90.0f);

        view = glm::lookAt(vec3(0.5f, 0.75f, 0.75f), vec3(0.0f, 0.0f, 0.0f), vec3(0.0f, 1.0f, 0.0f));
        projection = mat4(1.0f);


        IMGUI_CHECKVERSION();
        ImGui::CreateContext();
        ImGuiIO& io = ImGui::GetIO(); (void)io;


        ImGui::StyleColorsDark();

        ImGui_ImplGlfw_InitForOpenGL(window, true);
        ImGui_ImplOpenGL3_Init();

        glClearColor(0.08f, 0.015f, 0.0f, 1.0f);

        //ADD SPOTLIGHT
        prog.setUniform("Spot.L", vec3(1.0f, 0.0f, 0.3f));
        prog.setUniform("Spot.La", vec3(1.0f));
        prog.setUniform("Spot.Exponent", 50.0f);
        prog.setUniform("Spot.Cutoff", glm::radians(2.0f));

        //Add point light
        prog.setUniform("Light.Position", (view * glm::vec4(x, 5.0f, 0.0, 0.0f)));

        prog.setUniform("Light.L", vec3(0.9f));

        prog.setUniform("Light.La", vec3(0.5f, 0.2f, 0.1f));

        //Locate all textures
        GLuint skybox = Texture::loadCubeMap("media/skybox/sand", ".png");

        GLuint pyBricks = Texture::loadTexture("media/texture/bricks.jpg");

        GLuint staff = Texture::loadTexture("media/texture/red.png");

        GLuint normalMap = Texture::loadTexture("media/texture/normalMap.png");

        GLuint dirt = Texture::loadTexture("media/texture/dirt.png");

        // Load brick texture file into channel 0
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, skybox);

        // Load texture file into channel 1
        glActiveTexture(GL_TEXTURE1);
        glBindTexture(GL_TEXTURE_2D, pyBricks);


        // Load texture file into channel 2
        glActiveTexture(GL_TEXTURE2);
        glBindTexture(GL_TEXTURE_2D, staff);

        // Load texture file into channel 3
        glActiveTexture(GL_TEXTURE3);
        glBindTexture(GL_TEXTURE_2D, normalMap);

        // Load texture file into channel 4
        glActiveTexture(GL_TEXTURE4);
        glBindTexture(GL_TEXTURE_2D, dirt);

        
        //glClearColor(1.0f, 0.0f, 0.0f, 1.0f);
        glEnable(GL_DEPTH_TEST);
        projection = mat4(1.0f);
        angle = glm::pi<float>() / 4.0f;
        setupFBO();
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
		prog.compileShader("shader/basic_uniform.vert");
		prog.compileShader("shader/basic_uniform.frag");
		prog.link();
		prog.use();


        edgeProg.compileShader("shader/edge_uniform.vert");
        edgeProg.compileShader("shader/edge_uniform.frag");
        edgeProg.link();
        

        
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


void SceneBasic_Uniform::renderGUI()
{

	ImGui_ImplOpenGL3_NewFrame();
	ImGui_ImplGlfw_NewFrame();
	ImGui::NewFrame();



	glm::vec3 camZoom = glm::vec3(0, 0, 2);

	static float backgroundValue = 0.0f;
	static int counter = 0;

	ImGui::Begin("Customisation Menu");

	ImGui::Text("---------Controls(WASD)---------");

    if (ImGui::Button("<"))
    {
        rotSpeed = 0.3;
    }
    ImGui::SameLine();
    if (ImGui::Button(">"))
    {
        rotSpeed = -0.3;
    }
    if (ImGui::Button("Stop"))
    {
        rotSpeed = 0;
    }
    ImGui::SameLine();


	ImGui::End();


	




	ImGui::Render();
	int display_w, display_h;
	ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

}

void SceneBasic_Uniform::render()
{
    
    glfwMakeContextCurrent(window);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    pass1();

    if (shaderType == 1)
    {
        prog.use();
    }
    else
    {
        edgeProg.use();
        glFlush();
        pass2();
    }

    controls();
        //prog.use();
        renderGUI();
        glfwPollEvents();
    
       

    


}

void SceneBasic_Uniform::setMatrices()
{
    if (shaderType == 1)
    {
        mat4 mv;
        mv = view * model;

        prog.setUniform("ModelViewMatrix", mv);
        prog.setUniform("NormalMatrix", glm::mat3(vec3(mv[0]), vec3(mv[1]), vec3(mv[2])));
        prog.setUniform("MVP", projection * mv);
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


void SceneBasic_Uniform::setupFBO()
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


void SceneBasic_Uniform::pass1()
{
    if (shaderType == 1)
    {
        prog.use();

        //Set parameters for spotlight
        vec4 lightPos = vec4(15.0f, 15.0f, 0.0f, 1.0f);
        prog.setUniform("Spot.Position", vec3(view * lightPos));
        mat3 normalMatrix = mat3(vec3(view[0]), vec3(view[1]), vec3(view[2]));
        prog.setUniform("Spot.Direction", normalMatrix * vec3(-lightPos));



        //Sets parameters for model and renders them
        prog.setUniform("texID", 1);
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


        //Sets parameters for model and renders them
        //Also Changes texID to choose a different texture
        prog.setUniform("texID", 2);
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




        //Sets parameters for skybox and renders it
        prog.setUniform("texID", 0);
        vec3 cameraPos = vec3(7.0f * cos(angle), 2.0f, 7.0f * sin(angle));
        view = glm::lookAt(cameraPos, vec3(0.0f, 0.0f, 0.0f), vec3(0.0f, 1.0f,
            0.0f));

        model = mat4(1.0f);
        setMatrices();
        sky.render();
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