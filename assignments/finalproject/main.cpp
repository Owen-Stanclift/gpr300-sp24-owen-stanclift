#include <stdio.h>
#include <math.h>

#include <iostream>
#include <ew/external/glad.h>
#include <GLFW/glfw3.h>
#include <imgui.h>
#include <imgui_impl_glfw.h>
#include <imgui_impl_opengl3.h>
#include <ew/shader.h>
#include <ew/model.h>
#include <ew/camera.h>
#include <ew/cameraController.h>
#include <ew/transform.h>
#include <ew/texture.h>
#include <ew/procGen.h>

//#define GLM_ENABLE_EXPERIMENTAL
#include "glm/gtx/transform.hpp"


GLenum glCheckError_(const char* file, int line)
{
	GLenum errorCode;
	while ((errorCode = glGetError()) != GL_NO_ERROR)
	{
		std::string error;
		switch (errorCode)
		{
		case GL_INVALID_ENUM:                  error = "INVALID_ENUM"; break;
		case GL_INVALID_VALUE:                 error = "INVALID_VALUE"; break;
		case GL_INVALID_OPERATION:             error = "INVALID_OPERATION"; break;
		case GL_OUT_OF_MEMORY:                 error = "OUT_OF_MEMORY"; break;
		case GL_INVALID_FRAMEBUFFER_OPERATION: error = "INVALID_FRAMEBUFFER_OPERATION"; break;
		}
		std::cout << error << " | " << file << " (" << line << ")" << std::endl;
	}
	return errorCode;
}
#define glCheckError() glCheckError_(__FILE__, __LINE__) 

using namespace std;

void framebufferSizeCallback(GLFWwindow* window, int width, int height);
GLFWwindow* initWindow(const char* title, int width, int height);
ew::Camera camera;
ew::CameraController cameraController;


// imgui uniforms
struct
{
	float land_scale = 10.0f;
	float water_height = 2.0f;
	float wave_speed = 2.0f;
	float refreaction_power = 5.0f;
} debug;

void drawUI();

struct WaterFBO
{
	GLuint fbo;
	GLuint color0;
	GLuint depth;
	void waterInit()
	{
		glGenFramebuffers(1, &fbo);
		glBindFramebuffer(GL_FRAMEBUFFER, fbo);

		glGenTextures(1, &color0);
		glBindTexture(GL_TEXTURE_2D, color0);
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, 800, 600, 0, GL_RGB, GL_UNSIGNED_BYTE, nullptr);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, color0, 0);

		glGenTextures(1, &depth);
		glBindTexture(GL_TEXTURE_2D, depth);
		glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT, 800, 600, 0, GL_DEPTH_COMPONENT, GL_UNSIGNED_BYTE, nullptr);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, depth, 0);

		if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
		{
			printf("glorious failure\n");
		}
		glBindFramebuffer(GL_FRAMEBUFFER, 0);
	};
} waterBuffer;

//Global state
int screenWidth = 1080;
int screenHeight = 720;
float prevFrameTime;
float deltaTime;

int main() {
	GLFWwindow* window = initWindow("Assignment 0", screenWidth, screenHeight);

	ew::Shader land_shader = ew::Shader("assets/landmass.vert", "assets/landmass.frag");
	ew::Shader water_shader = ew::Shader("assets/water.vert", "assets/water.frag");
	GLuint 	heightmap = ew::loadTexture("assets/heightmap.png");
	GLuint 	waterTexture = ew::loadTexture("assets/wave_tex.png");
	GLuint 	waterSpec = ew::loadTexture("assets/wave_spec.png");
	GLuint 	waterWarp = ew::loadTexture("assets/wave_warp.png");
	GLuint 	dudv = ew::loadTexture("assets/DuDvMap.png");

	ew::Mesh islandPlane;
	ew::Mesh waterPlane;

	waterBuffer.waterInit();

	islandPlane.load(ew::createPlane(50.0f, 50.0f, 100));
	waterPlane.load(ew::createPlane(50.0f, 50.0f, 1));

	camera.position = glm::vec3(0.0f, 5.0f, 5.0f);
	camera.target = glm::vec3(0.0f, 0.0f, 0.0f);
	camera.aspectRatio = (float)screenWidth / screenHeight;
	camera.fov = 60.0f;

	glfwSetFramebufferSizeCallback(window, framebufferSizeCallback);
	while (!glfwWindowShouldClose(window)) {
		glfwPollEvents();

		// always first
		float time = (float)glfwGetTime();
		deltaTime = time - prevFrameTime;
		prevFrameTime = time;
		cameraController.move(window, &camera, deltaTime);

		const auto view_proj = camera.projectionMatrix() * camera.viewMatrix();

		glClearColor(0.2f, 0.3f, 0.3f, 1.0f);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		//WaterFBO
		{
			glBindFramebuffer(GL_FRAMEBUFFER, waterBuffer.fbo);

			//Set back
			glBindFramebuffer(GL_FRAMEBUFFER, 0);

		}


		//Landmass
		{	// render landmass:
			glEnable(GL_CULL_FACE);
			glCullFace(GL_BACK);
			glEnable(GL_DEPTH_TEST);

			glActiveTexture(GL_TEXTURE0);
			glBindTexture(GL_TEXTURE_2D, heightmap);
			glActiveTexture(GL_TEXTURE1);
			glBindTexture(GL_TEXTURE_2D, waterWarp);
			glActiveTexture(GL_TEXTURE2);
			glBindTexture(GL_TEXTURE_2D, waterSpec);
			glActiveTexture(GL_TEXTURE3);
			glBindTexture(GL_TEXTURE_2D, dudv);
			glEnable(GL_CLIP_DISTANCE0);

			land_shader.use();
			land_shader.setInt("heightmap", 0);
			land_shader.setInt("plane", debug.water_height);
			land_shader.setMat4("model", glm::mat4(1.0f));
			land_shader.setMat4("view_proj", view_proj);
			land_shader.setFloat("landmass.scale", debug.land_scale);

			islandPlane.draw();
		}
		//Water
		{	// render water:
			glEnable(GL_BLEND);
			glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

			water_shader.use();
			water_shader.setMat4("model", glm::translate(glm::vec3(0.0f, debug.water_height, 0.0f)));
			water_shader.setMat4("view_proj", view_proj);
			water_shader.setVec3("camera_position", camera.position);
			water_shader.setFloat("tiling", 6.0f);
			water_shader.setFloat("waveSpeed", debug.wave_speed * glfwGetTime());
			water_shader.setFloat("power", debug.refreaction_power);
			water_shader.setInt("reflectTexture", 1);
			water_shader.setInt("refractTexture", 2);
			water_shader.setInt("dudvMap", 3);
			waterPlane.draw();
		}

		// always last.
		drawUI();
		glfwSwapBuffers(window);
	}
	printf("Shutting down...");
}
void resetCamera(ew::Camera* camera, ew::CameraController* controller)
{
	camera->position = glm::vec3(0, 0, 5.0f);
	camera->target = glm::vec3(0);
	controller->yaw = controller->pitch = 0;
}

void drawUI() {
	ImGui_ImplGlfw_NewFrame();
	ImGui_ImplOpenGL3_NewFrame();
	ImGui::NewFrame();

	ImGui::Begin("Settings");
	ImGui::Separator();

	ImGui::SliderFloat("Land Height", &debug.land_scale, 0.0f, 10.0f);
	ImGui::SliderFloat("Water Height", &debug.water_height, 0.0f, 10.0f);
	ImGui::SliderFloat("Water Speed", &debug.wave_speed, 0.0f, 10.0f);
	ImGui::SliderFloat("Refractions", &debug.refreaction_power, 0.0f, 10.0f);

	ImVec2 size = ImGui::GetWindowSize();
	ImGui::Image((ImTextureID)waterBuffer.fbo, size, ImVec2(0, 1), ImVec2(1, 0));
	ImGui::End();


	ImGui::Render();
	ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
}

void framebufferSizeCallback(GLFWwindow* window, int width, int height)
{
	glViewport(0, 0, width, height);
	screenWidth = width;
	screenHeight = height;
}

/// <summary>
/// Initializes GLFW, GLAD, and IMGUI
/// </summary>
/// <param name="title">Window title</param>
/// <param name="width">Window width</param>
/// <param name="height">Window height</param>
/// <returns>Returns window handle on success or null on fail</returns>
GLFWwindow* initWindow(const char* title, int width, int height) {
	printf("Initializing...");
	if (!glfwInit()) {
		printf("GLFW failed to init!");
		return nullptr;
	}


	GLFWwindow* window = glfwCreateWindow(width, height, title, NULL, NULL);
	if (window == NULL) {
		printf("GLFW failed to create window");
		return nullptr;
	}
	glfwMakeContextCurrent(window);

	if (!gladLoadGL(glfwGetProcAddress)) {
		printf("GLAD Failed to load GL headers");
		return nullptr;
	}

	//Initialize ImGUI
	IMGUI_CHECKVERSION();
	ImGui::CreateContext();
	ImGui_ImplGlfw_InitForOpenGL(window, true);
	ImGui_ImplOpenGL3_Init();

	return window;
}

