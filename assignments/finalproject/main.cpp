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
	float wave_speed = 0.1f;
	float refreaction_power = 5.0f;
	float wave_length = 0.02;
	int tiles = 5;
} debug;

float moveFactor;
void drawUI();

struct Framebuffer
{
	GLuint fbo;
	GLuint color0;
	GLuint depth;
	void init()
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
};

struct WaterMaterial {
	unsigned int spec;
	unsigned int dudv;
	unsigned int normal;
	unsigned int warp;
} water_material;

enum {
	WATER_REFLECTION = 0,
	WATER_REFRACTION = 1,
	WATER_COUNT,
};

Framebuffer waterBuffers[WATER_COUNT];

struct Light {
	glm::vec3 lightPosition = glm::vec3(0);
	glm::vec3 lightColor = glm::vec3(1);
}light;

//Global state
int screenWidth = 1080;
int screenHeight = 720;
float prevFrameTime;
float deltaTime;

// render terrain:
void render_terrain(GLuint heightmap, const ew::Shader& shader, const ew::Mesh& mesh, const glm::vec4 clipping_plane)
{
	const auto view_proj = camera.projectionMatrix() * camera.viewMatrix();
	glEnable(GL_CULL_FACE);
	glCullFace(GL_BACK);
	glEnable(GL_DEPTH_TEST);
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, heightmap);

	shader.use();
	shader.setInt("heightmap", 0);
	shader.setMat4("model", glm::mat4(1.0f));
	shader.setMat4("view_proj", view_proj);
	shader.setFloat("landmass.scale", debug.land_scale);
	shader.setVec4("clipping_plane", clipping_plane);

	mesh.draw();
}

// render water:
void render_water(const ew::Shader& shader, const ew::Mesh& mesh)
{
	const auto view_proj = camera.projectionMatrix() * camera.viewMatrix();
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, waterBuffers[WATER_REFLECTION].color0);
	glActiveTexture(GL_TEXTURE1);
	glBindTexture(GL_TEXTURE_2D, waterBuffers[WATER_REFRACTION].color0);
	glActiveTexture(GL_TEXTURE2);
	glBindTexture(GL_TEXTURE_2D, water_material.dudv);
	glActiveTexture(GL_TEXTURE3);
	glBindTexture(GL_TEXTURE_2D, water_material.normal);

	shader.use();
	shader.setMat4("model", glm::translate(glm::vec3(0.0f, debug.water_height, 0.0f)));
	shader.setFloat("tiling", debug.tiles);
	shader.setVec3("camera_position", camera.position);
	shader.setVec3("lightPosition", light.lightPosition);
	shader.setVec3("lightColor", light.lightColor);
	shader.setMat4("view_proj", view_proj);
	shader.setInt("reflectTexture", 0);
	shader.setInt("refractTexture", 1);
	shader.setInt("dudvMap", 2);
	shader.setInt("normalMap", 3);
	shader.setFloat("refractStrength", debug.refreaction_power);
	shader.setFloat("moveFactor", moveFactor);
	mesh.draw();
}


int main() {
	GLFWwindow* window = initWindow("Assignment 0", screenWidth, screenHeight);

	ew::Shader land_shader = ew::Shader("assets/landmass.vert", "assets/landmass.frag");
	ew::Shader water_shader = ew::Shader("assets/water.vert", "assets/water.frag");
	GLuint 	heightmap = ew::loadTexture("assets/heightmap.png");

	water_material.dudv = ew::loadTexture("assets/DuDvMap.png");
	water_material.normal = ew::loadTexture("assets/water_normal.png");
	water_material.spec = ew::loadTexture("assets/wave_spec.png");
	water_material.warp = ew::loadTexture("assets/wave_warp.png"); 

	ew::Mesh islandPlane;
	ew::Mesh waterPlane;

	waterBuffers[WATER_REFLECTION].init();
	waterBuffers[WATER_REFRACTION].init();

	islandPlane.load(ew::createPlane(50.0f, 50.0f, 100));
	waterPlane.load(ew::createPlane(50.0f, 50.0f, 100));

	camera.position = glm::vec3(0.0f, 5.0f, 5.0f);
	camera.target = glm::vec3(0.0f, 0.0f, 0.0f);
	camera.aspectRatio = (float)screenWidth / screenHeight;
	camera.fov = 60.0f; 

	glfwSetFramebufferSizeCallback(window, framebufferSizeCallback);
	while (!glfwWindowShouldClose(window)) {
		glfwPollEvents();
		// always first
		float time = (float)glfwGetTime();
		moveFactor = debug.wave_speed * time;
		deltaTime = time - prevFrameTime;
		prevFrameTime = time;
		cameraController.move(window, &camera, deltaTime);

		glViewport(0, 0, 800, 600);

		// WATER_REFLECTION:
		glBindFramebuffer(GL_FRAMEBUFFER, waterBuffers[WATER_REFLECTION].fbo);
		{
			glEnable(GL_CLIP_DISTANCE0);
			glClearColor(0.2f, 0.2f, 1.0f, 0.0f);
			glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

			float posXOriginal = camera.position.x;
			float posYOriginal = camera.position.y;
			float posZOriginal = camera.position.z;
			float pitchOriginal = cameraController.pitch;
			float yawOriginal = cameraController.yaw;
			float distX = 2.0f * (camera.position.x);
			float distY = 2.0f * (camera.position.y - debug.water_height);
			float distZ = 2.0f * (camera.position.z);
			//camera.position.x -= distX;
			camera.position.y -= distY;
			camera.position.z -= distZ;
			cameraController.pitch *= -1.0f;
			//cameraController.yaw *= -1.0f;

			// TODO: MAYBE SET A NEW CAMERA ANGLE;
			render_terrain(heightmap, land_shader, islandPlane, glm::vec4(0.0, 1.0, 0.0, -debug.water_height));

			camera.position.x = posXOriginal;
			camera.position.y = posYOriginal;
			camera.position.z = posZOriginal;
			cameraController.pitch = pitchOriginal;
			cameraController.yaw = yawOriginal;
		}
		glBindFramebuffer(GL_FRAMEBUFFER, 0);

		// WATER_REFRACTION:
		glBindFramebuffer(GL_FRAMEBUFFER, waterBuffers[WATER_REFRACTION].fbo);
		{
			glEnable(GL_CLIP_DISTANCE0);
			glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
			glViewport(0, 0, 800, 600);
			glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
			// TODO: MAYBE SET A NEW CAMERA ANGLE;
			render_terrain(heightmap, land_shader, islandPlane, glm::vec4(0.0, -1.0, 0.0, debug.water_height));
		}
		glBindFramebuffer(GL_FRAMEBUFFER, 0);


		// SKY:
		glDisable(GL_CLIP_DISTANCE0);
		glClearColor(0.2f, 0.3f, 0.3f, 1.0f);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		glViewport(0, 0, 800, 600);

		render_terrain(heightmap, land_shader, islandPlane, glm::vec4(0.0f));
		render_water(water_shader, waterPlane);

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
	ImGui::SliderFloat("Water Speed", &debug.wave_speed, 0.0f, 1.0f);
	ImGui::SliderFloat("Water Length", &debug.wave_length, 0.0f, 100.0f);
	ImGui::SliderFloat("Refractions", &debug.refreaction_power, 0.0f, 10.0f);
	ImGui::SliderInt("Tiles", &debug.tiles, 0.0f, 10.0f);

	ImGui::Text("(%.2f, %.2f, %.2f)", camera.position.x, camera.position.y, camera.position.z);

	ImVec2 size = { 400.0f, 300.0f };
	ImGui::Text("Refraction (fbo.color0)");
	ImGui::Image((ImTextureID)waterBuffers[WATER_REFRACTION].color0, size, ImVec2(0, 1), ImVec2(1, 0));
	ImGui::Text("Reflection (fbo.color0)");
	ImGui::Image((ImTextureID)waterBuffers[WATER_REFLECTION].color0, size);
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

