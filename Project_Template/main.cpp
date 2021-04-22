#include "helper/scene.h"
#include "helper/scenerunner.h"
#include "scenebasic_uniform.h"
#include <imgui.h>
#include <imgui-1.79/examples/imgui_impl_glfw.h>
#include <imgui-1.79/examples/imgui_impl_opengl3.h>
#include "GLFW/glfw3.h"
#include "GLFW/glfw3.h"
#include <glm/glm.hpp> //includes GLM
#include "glm/fwd.hpp"
#include <glm/gtc/type_ptr.hpp> // GLM: access to the value_ptr


int main(int argc, char* argv[])
{
	SceneRunner runner("Shader_Basics");

	std::unique_ptr<Scene> scene;

	scene = std::unique_ptr<Scene>(new SceneBasic_Uniform(runner.window));


	return runner.run(*scene);
}