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

int imageProcessingType = 1;

#include "helper/glutils.h"

std::unique_ptr<ObjMesh> tree;
std::unique_ptr<ObjMesh> sphinx;
std::unique_ptr<ObjMesh> pond;
std::unique_ptr<ObjMesh> cactus;
std::unique_ptr<ObjMesh> camel;

using glm::vec3;

ImVec4 backgroundColour = ImVec4(0.06f, 0.015f, 0.0f, 1.0f);

float x, z;

GLFWwindow* window;

int structureType = 1;
int floraType = 1;


SceneBasic_Uniform::SceneBasic_Uniform(GLFWwindow* sceneRunnerWindow) : angle(0.0f), tPrev(0.0f), rotSpeed(-0.0f), plane(500.0f,
    500.0f, 1, 1), time(0), aniPlane(70.0f, 20.0f, 20, 2)
{
    tree = ObjMesh::load("media/tree.obj", true);
    sphinx = ObjMesh::load("media/sphinx.obj", true);
    pond = ObjMesh::load("media/pond.obj", true);
    cactus = ObjMesh::load("media/cactus.obj", true);
    camel = ObjMesh::load("media/camel.obj", true);

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

        angle = glm::pi<float>() / 4.0f;

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


        prog.setUniform("Light.Position", (view * glm::vec4(3.0f, 5.0f, 0.0, 0.0f)));
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


        aniProg.use();
        aniProg.setUniform("lights.Position", (view * glm::vec4(0.0f, 5.0f, 0.0, 0.0f)));

        aniProg.setUniform("lights.L", vec3(0.627,0.843f,0.952f));

        aniProg.setUniform("lights.La", vec3(0.6f));

        angle = glm::half_pi<float>();
        
        prog.use();

        IMGUI_CHECKVERSION();
        ImGui::CreateContext();
        ImGuiIO& io = ImGui::GetIO(); (void)io;


        ImGui::StyleColorsDark();

        ImGui_ImplGlfw_InitForOpenGL(window, true);
        ImGui_ImplOpenGL3_Init();



        //Locate all textures

        GLuint treeTexture = Texture::loadTexture("media/texture/treeTex.png");

        GLuint sphinxTexture = Texture::loadTexture("media/texture/sphinxBricks.jpg");

        GLuint pondTexture = Texture::loadTexture("media/texture/pondTex.png");

        GLuint sandTexture = Texture::loadTexture("media/texture/sand.png");


        // Load texture file into channel 1
        glActiveTexture(GL_TEXTURE1);
        glBindTexture(GL_TEXTURE_2D, treeTexture);


        // Load texture file into channel 2
        glActiveTexture(GL_TEXTURE2);
        glBindTexture(GL_TEXTURE_2D, sphinxTexture);

        // Load texture file into channel 3
        glActiveTexture(GL_TEXTURE3);
        glBindTexture(GL_TEXTURE_2D, pondTexture);


        // Load texture file into channel 4
        glActiveTexture(GL_TEXTURE4);
        glBindTexture(GL_TEXTURE_2D, sandTexture);


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

        imageProcessingType = 1;
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
        imageProcessingType = 2;


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
        imageProcessingType = 0;
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

        aniProg.compileShader("shader/animation_uniform.vert");
        aniProg.compileShader("shader/animation_uniform.frag");
        aniProg.link();

        
	} catch (GLSLProgramException &e) {
		cerr << e.what() << endl;
		exit(EXIT_FAILURE);
	}
}

void SceneBasic_Uniform::update( float t )
{
        
    time = t;
    
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
    imageProcessingType = 0;


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

    if (ImGui::Button("<<"))
    {
        rotSpeed = 0.6;
    }
    ImGui::SameLine();
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
    ImGui::SameLine();
    if (ImGui::Button(">>"))
    {
        rotSpeed = -0.6;
    }
    
    ImGui::Text("---------Flora---------");

    if (ImGui::Button("Tree"))
    {
        floraType = 1;
    }
    ImGui::SameLine();
    if (ImGui::Button("Cactus"))
    {
        floraType = 2;
    }


    ImGui::Text("---------Structures---------");

    if (ImGui::Button("Sphinx"))
    {
        structureType = 1;
    }
    ImGui::SameLine();
    if (ImGui::Button("Camel"))
    {
        structureType = 2;
    }



    ImGui::Text("---------Image Processing---------");

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

    pass1();
    pass2();

    if (imageProcessingType == 1)
    {
        pass3();
    }
    else
    {
        glFlush();
    }

    

    view = glm::lookAt(vec3(10.0f * cos(angle), 4.0f, 10.0f * sin(angle)), vec3(0.0f), vec3(0.0f, 1.0f, 0.0f));

    projection = glm::perspective(glm::radians(60.0f), (float)width / height, 0.3f, 100.0f);



    controls();
    renderGUI();
    glfwPollEvents();

}

void SceneBasic_Uniform::setMatrices(GLSLProgram& program)
{
    
    
        mat4 mv;
        mv = view * model;

        program.setUniform("ModelMatrix", model);
        program.setUniform("ModelViewMatrix", mv);
        program.setUniform("NormalMatrix", glm::mat3(vec3(mv[0]), vec3(mv[1]), vec3(mv[2])));
        program.setUniform("MVP", projection * mv);
    
    

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
    GLenum drawBuffers[] = { GL_COLOR_ATTACHMENT0 };
    glDrawBuffers(1, drawBuffers);
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
    glDrawBuffers(1, drawBuffers);
    // Unbind the framebuffer, and revert to default framebuffer
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
}


void SceneBasic_Uniform::pass1()
{
    if (imageProcessingType == 1)
    {
        prog.use();

        prog.setUniform("Pass", 1);

        glBindFramebuffer(GL_FRAMEBUFFER, renderFBO);
        glEnable(GL_DEPTH_TEST);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        


        //Sets parameters for model and renders them
        prog.setUniform("texID", 1);
        prog.setUniform("Material.Kd", 0.8f, 0.8f, 0.8f);
        prog.setUniform("Material.Ks", 0.2f, 0.2f, 0.2f);
        prog.setUniform("Material.Ka", 0.2f, 0.2f, 0.2f);
        prog.setUniform("Material.Shininess", 5.0f);
        model = mat4(1.0f);
        
        if (floraType == 1)
        {
            model = glm::translate(model, vec3(-3.0f, -1.0f, -6.0f));
            model = glm::rotate(model, glm::radians(90.0f), vec3(0.0f, 1.0f, 0.0f));
            model = glm::scale(model, glm::vec3(1.0f));
            setMatrices(prog);
            tree->render();
        }
        else
        {
            model = glm::translate(model, vec3(-3.0f, 1.5f, -6.0f));
            model = glm::rotate(model, glm::radians(-90.0f), vec3(1.0f, 0.0f, 0.0f));
            model = glm::scale(model, glm::vec3(0.02f));
            setMatrices(prog);
            cactus->render();
        }


        
        prog.setUniform("texID", 2);
        prog.setUniform("Material.Kd", 0.4f, 0.4f, 0.4f);
        prog.setUniform("Material.Ks", 0.9f, 0.9f, 0.9f);
        prog.setUniform("Material.Ka", 0.5f, 0.5f, 0.5f);
        prog.setUniform("Material.Shininess", 180.0f);
        model = mat4(1.0f);
        

        if (structureType == 1)
        {
            model = glm::translate(model, vec3(9.0f, 1.0f, -8.0f));
            model = glm::rotate(model, glm::radians(-90.0f), vec3(1.0f, 0.0f, 0.0f));
            model = glm::rotate(model, glm::radians(-30.0f), vec3(0.0f, 0.0f, 1.0f));
            model = glm::scale(model, glm::vec3(0.002f));
            setMatrices(prog);
            sphinx->render();
        }
        else
        {
            model = glm::translate(model, vec3(7.5f, 0.8f, -6.0f));
            model = glm::rotate(model, glm::radians(-30.0f), vec3(0.0f, 1.0f, 0.0f));
            model = glm::scale(model, glm::vec3(0.3f));
            setMatrices(prog);
            camel->render();
        }
        





        prog.setUniform("texID", 3);
        prog.setUniform("Material.Kd", 0.4f, 0.4f, 0.4f);
        prog.setUniform("Material.Ks", 0.0f, 0.0f, 0.0f);
        prog.setUniform("Material.Ka", 0.1f, 0.1f, 0.1f);
        prog.setUniform("Material.Shininess", 1.0f);
        model = mat4(1.0f);
        model = glm::translate(model, vec3(0.0f, -0.75f, 0.0f));
        setMatrices(prog);
        plane.render();


        prog.setUniform("lights.Position", (view * glm::vec4(3.0f, 5.0f, 0.0, 0.0f)));

        prog.setUniform("lights.L", vec3(0.627, 0.843f, 0.952f));

        prog.setUniform("lights.La", vec3(0.6f));

        prog.setUniform("texID", 4);
        prog.setUniform("Material.Kd", 0.2f, 0.5f, 0.9f);
        prog.setUniform("Material.Ks", 0.2f, 0.5f, 0.9f);
        prog.setUniform("Material.Ka", 0.2f, 0.5f, 0.9f);
        prog.setUniform("Material.Shininess", 180.0f);
        model = mat4(1.0f);
        model = glm::translate(model, vec3(0.0f, -0.8f, -6.0f));
        setMatrices(prog);
        pond->render();
        

        prog.setUniform("Light.Position", (view * glm::vec4(3.0f, 5.0f, 0.0, 0.0f)));
        prog.setUniform("Light.L", vec3(1.0f));
        prog.setUniform("Light.La", vec3(0.6f));





        aniProg.use();
        aniProg.setUniform("Time", time);


        aniProg.setUniform("Material.Kd", 0.2f, 0.5f, 0.9f);
        aniProg.setUniform("Material.Ks", 0.2f, 0.5f, 0.9f);
        aniProg.setUniform("Material.Ka", 0.2f, 0.5f, 0.9f);
        aniProg.setUniform("Material.Shininess", 1.0f);
        model = mat4(1.0f);
        
        model = mat4(1.0f);
        model = glm::translate(model, vec3(0.0f, -0.7f, -6.0f));
        model = glm::scale(model, glm::vec3(0.999f, 0.03f, 0.999f));
        setMatrices(aniProg);
        pond->render();


    }
    else
    {
        edgeProg.use();

        edgeProg.setUniform("Pass", 1);
        glBindFramebuffer(GL_FRAMEBUFFER, fboHandle);
        glEnable(GL_DEPTH_TEST);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);


        
        edgeProg.setUniform("Light.Position", vec4(0.0f, 0.0f, 0.0f, 1.0f));


        //Sets parameters for model and renders them
        edgeProg.setUniform("texID", 1);
        edgeProg.setUniform("Material.Kd", 0.8f, 0.8f, 0.8f);
        edgeProg.setUniform("Material.Ks", 0.2f, 0.2f, 0.2f);
        edgeProg.setUniform("Material.Ka", 0.2f, 0.2f, 0.2f);
        edgeProg.setUniform("Material.Shininess", 5.0f);
        model = mat4(1.0f);

        if (floraType == 1)
        {
            model = glm::translate(model, vec3(-3.0f, -1.0f, -6.0f));
            model = glm::rotate(model, glm::radians(90.0f), vec3(0.0f, 1.0f, 0.0f));
            model = glm::scale(model, glm::vec3(1.0f));
            setMatrices(edgeProg);
            tree->render();
        }
        else
        {
            model = glm::translate(model, vec3(-3.0f, 1.5f, -6.0f));
            model = glm::rotate(model, glm::radians(-90.0f), vec3(1.0f, 0.0f, 0.0f));
            model = glm::scale(model, glm::vec3(0.02f));
            setMatrices(edgeProg);
            cactus->render();
        }



        edgeProg.setUniform("texID", 2);
        edgeProg.setUniform("Material.Kd", 0.4f, 0.4f, 0.4f);
        edgeProg.setUniform("Material.Ks", 0.9f, 0.9f, 0.9f);
        edgeProg.setUniform("Material.Ka", 0.5f, 0.5f, 0.5f);
        edgeProg.setUniform("Material.Shininess", 180.0f);
        model = mat4(1.0f);


        if (structureType == 1)
        {
            model = glm::translate(model, vec3(9.0f, 1.0f, -8.0f));
            model = glm::rotate(model, glm::radians(-90.0f), vec3(1.0f, 0.0f, 0.0f));
            model = glm::rotate(model, glm::radians(-30.0f), vec3(0.0f, 0.0f, 1.0f));
            model = glm::scale(model, glm::vec3(0.002f));
            setMatrices(edgeProg);
            sphinx->render();
        }
        else
        {
            model = glm::translate(model, vec3(7.5f, 0.8f, -6.0f));
            model = glm::rotate(model, glm::radians(-30.0f), vec3(0.0f, 1.0f, 0.0f));
            model = glm::scale(model, glm::vec3(0.3f));
            setMatrices(edgeProg);
            camel->render();
        }






        edgeProg.setUniform("texID", 3);
        edgeProg.setUniform("Material.Kd", 0.4f, 0.4f, 0.4f);
        edgeProg.setUniform("Material.Ks", 0.0f, 0.0f, 0.0f);
        edgeProg.setUniform("Material.Ka", 0.1f, 0.1f, 0.1f);
        edgeProg.setUniform("Material.Shininess", 1.0f);
        model = mat4(1.0f);
        model = glm::translate(model, vec3(0.0f, -0.75f, 0.0f));
        setMatrices(edgeProg);
        plane.render();


        edgeProg.setUniform("lights.Position", (view * glm::vec4(3.0f, 5.0f, 0.0, 0.0f)));

        edgeProg.setUniform("lights.L", vec3(0.627, 0.843f, 0.952f));

        edgeProg.setUniform("lights.La", vec3(0.6f));

        edgeProg.setUniform("texID", 4);
        edgeProg.setUniform("Material.Kd", 0.2f, 0.5f, 0.9f);
        edgeProg.setUniform("Material.Ks", 0.2f, 0.5f, 0.9f);
        edgeProg.setUniform("Material.Ka", 0.2f, 0.5f, 0.9f);
        edgeProg.setUniform("Material.Shininess", 180.0f);
        model = mat4(1.0f);
        model = glm::translate(model, vec3(0.0f, -0.8f, -6.0f));
        setMatrices(edgeProg);
        pond->render();


        edgeProg.setUniform("Light.Position", (view * glm::vec4(3.0f, 5.0f, 0.0, 0.0f)));
        edgeProg.setUniform("Light.L", vec3(1.0f));
        edgeProg.setUniform("Light.La", vec3(0.6f));





        aniProg.use();
        aniProg.setUniform("Time", time);


        aniProg.setUniform("Material.Kd", 0.2f, 0.5f, 0.9f);
        aniProg.setUniform("Material.Ks", 0.2f, 0.5f, 0.9f);
        aniProg.setUniform("Material.Ka", 0.2f, 0.5f, 0.9f);
        aniProg.setUniform("Material.Shininess", 1.0f);
        model = mat4(1.0f);

        model = mat4(1.0f);
        model = glm::translate(model, vec3(0.0f, -0.7f, -6.0f));
        model = glm::scale(model, glm::vec3(0.999f, 0.03f, 0.999f));
        setMatrices(aniProg);
        pond->render();


        edgeProg.use();


    }



}

void SceneBasic_Uniform::pass2()
{

    if (imageProcessingType == 1)
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
        setMatrices(prog);
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
        setMatrices(edgeProg);
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
    setMatrices(prog);
    // Render the full-screen quad
    glBindVertexArray(fsQuad);
    glDrawArrays(GL_TRIANGLES, 0, 6);
}