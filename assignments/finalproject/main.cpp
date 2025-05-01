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


//Git giving a hard time
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
ew::Mesh lightSphere;


// imgui uniforms
struct
{
	float land_scale = 10.0f;
	float water_height = 2.0f;
	float wave_speed = 0.1f;
	float refreaction_power = 5.0f;
	float wave_length = 10.0f;
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
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, 800, 600, 0, GL_RGBA, GL_UNSIGNED_BYTE, nullptr);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, color0, 0);

		glGenTextures(1, &depth);
		glBindTexture(GL_TEXTURE_2D, depth);
		glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT, 800, 600, 0, GL_DEPTH_COMPONENT, GL_UNSIGNED_BYTE, nullptr);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
		glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, depth, 0);

		if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
		{
			printf("glorious failure\n");
		}
		glBindFramebuffer(GL_FRAMEBUFFER, 0);
	};
};

float skyboxVerticies[] =
{
	//Cords
	-1.0f,  1.0f, -1.0f,
	-1.0f, -1.0f, -1.0f,
	 1.0f, -1.0f, -1.0f,
	 1.0f, -1.0f, -1.0f,
	 1.0f,  1.0f, -1.0f,
	-1.0f,  1.0f, -1.0f,

	-1.0f, -1.0f,  1.0f,
	-1.0f, -1.0f, -1.0f,
	-1.0f,  1.0f, -1.0f,
	-1.0f,  1.0f, -1.0f,
	-1.0f,  1.0f,  1.0f,
	-1.0f, -1.0f,  1.0f,

	 1.0f, -1.0f, -1.0f,
	 1.0f, -1.0f,  1.0f,
	 1.0f,  1.0f,  1.0f,
	 1.0f,  1.0f,  1.0f,
	 1.0f,  1.0f, -1.0f,
	 1.0f, -1.0f, -1.0f,

	-1.0f, -1.0f,  1.0f,
	-1.0f,  1.0f,  1.0f,
	 1.0f,  1.0f,  1.0f,
	 1.0f,  1.0f,  1.0f,
	 1.0f, -1.0f,  1.0f,
	-1.0f, -1.0f,  1.0f,

	-1.0f,  1.0f, -1.0f,
	 1.0f,  1.0f, -1.0f,
	 1.0f,  1.0f,  1.0f,
	 1.0f,  1.0f,  1.0f,
	-1.0f,  1.0f,  1.0f,
	-1.0f,  1.0f, -1.0f,

	-1.0f, -1.0f, -1.0f,
	-1.0f, -1.0f,  1.0f,
	 1.0f, -1.0f, -1.0f,
	 1.0f, -1.0f, -1.0f,
	-1.0f, -1.0f,  1.0f,
	 1.0f, -1.0f,  1.0f,

};

unsigned int skyboxIndicies[] =
{
	//Right
	1,2,6,
	6,5,1,

	//Left
	0,4,7,
	7,3,0,

	//Top
	4,5,6,
	6,7,4,

	//Bottom
	0,3,2,
	2,1,0,

	//Back
	0,1,5,
	5,4,0,

	//Front
	3,7,6,
	6,2,3
};
struct SkyBuffer
{
	GLuint skyboxVAO, skyboxVBO;
	GLuint cubemap;
	void init()
	{
		glGenVertexArrays(1, &skyboxVAO);
		glGenBuffers(1, &skyboxVBO);

		glBindVertexArray(skyboxVAO);
		glBindBuffer(GL_ARRAY_BUFFER, skyboxVBO);
		glBufferData(GL_ARRAY_BUFFER, sizeof(skyboxVerticies), &skyboxVerticies, GL_STATIC_DRAW);
		glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
		glEnableVertexAttribArray(0);
		glBindVertexArray(0);

		vector<std::string> faces =
		{
			"assets/right.jpg",
			"assets/left.jpg",
			"assets/bottom.jpg",
			"assets/top.jpg",
			"assets/front.jpg",
			"assets/back.jpg"
		};

		cubemap = ew::cubeMapTexture(faces, GL_CLAMP_TO_EDGE, GL_LINEAR, GL_LINEAR);
	}
} sky;

struct DepthBuffer {

	GLuint fbo;
	GLuint depth;

	void init()
	{
		glCreateFramebuffers(1, &fbo);
		glBindFramebuffer(GL_FRAMEBUFFER, fbo);

		glGenTextures(1, &depth);
		glBindTexture(GL_TEXTURE_2D, depth);
		glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT, 2048, 2048, 0, GL_DEPTH_COMPONENT, GL_FLOAT, nullptr);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
		float borderColor[4] = { 1.0f,1.0f,1.0f,1.0f };
		glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, depth, 0);

		glDrawBuffer(GL_NONE);
		glReadBuffer(GL_NONE);

		glBindFramebuffer(GL_FRAMEBUFFER, 0);

	}
} depthbuffer;

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
Framebuffer lightBuffer;
Framebuffer landShadowBuffer;

struct Light {
	glm::vec3 lightPosition = glm::vec3(0,15,0);
	glm::vec3 lightColor = glm::vec3(1);
} light;

//Global state
int screenWidth = 1080;
int screenHeight = 720;
float prevFrameTime;
float deltaTime;

//Camera adjustments
void recalculateCamera()
{
	float yawRad = glm::radians(cameraController.yaw);

	float pitchRad = glm::radians(cameraController.pitch);

	glm::vec3 forwardVec;

	forwardVec.x = cosf(pitchRad) * sinf(yawRad);
	forwardVec.y = sinf(pitchRad);
	forwardVec.z = cosf(pitchRad) * -cosf(yawRad);

	forwardVec = glm::normalize(forwardVec);

	glm::vec3 rightVec = glm::normalize(glm::cross(forwardVec, glm::vec3(0, 1, 0)));
	glm::vec3 upVec = glm::normalize(glm::cross(rightVec, forwardVec));
	
	camera.target = camera.position + forwardVec;
}

void render_light(const ew::Shader& shader, const ew::Mesh& sphere, const glm::vec4 clipping_plane)
{
	const auto view_proj = camera.projectionMatrix() * camera.viewMatrix();
	shader.use();
	shader.setMat4("_CameraViewProjection", view_proj);
	shader.setMat4("_Model", glm::translate(light.lightPosition));
	shader.setVec3("color", light.lightColor);
	shader.setVec4("clipping_plane", clipping_plane);
	sphere.draw();

}
// render terrain:
void render_terrain(GLuint heightmap, GLuint normalmap, const ew::Shader& shader, const ew::Mesh& mesh, const glm::vec4 clipping_plane)
{
	const auto view_proj = camera.projectionMatrix() * camera.viewMatrix();
	const auto light_proj = glm::ortho(-50.0f, 50.0f, -50.0f, 50.0f, 0.1f, 100.0f);
	const auto light_view = glm::lookAt(light.lightPosition, glm::vec3(0.0f), glm::vec3(0.0f, 1.0f, 0.0f));
	const auto light_view_proj = light_proj * light_view;

	glEnable(GL_CULL_FACE);
	glCullFace(GL_BACK);
	glEnable(GL_DEPTH_TEST);
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, heightmap);
	glActiveTexture(GL_TEXTURE1);
	glBindTexture(GL_TEXTURE_2D, depthbuffer.depth);

	shader.use();
	shader.setInt("heightmap", 0);
	shader.setInt("shadowmap", 1);
	shader.setMat4("model", glm::mat4(1.0f));
	shader.setMat4("view_proj", view_proj);
	shader.setMat4("light_view_proj", light_view_proj);
	shader.setFloat("landmass.scale", debug.land_scale);
	shader.setVec4("clipping_plane", clipping_plane);

	mesh.draw();
}
void render_sky(const ew::Shader& shader)
{
	const auto view_proj = camera.projectionMatrix() * glm::mat4(glm::mat3(camera.viewMatrix()));

	glBindVertexArray(sky.skyboxVAO);
	{
		glEnable(GL_TEXTURE_CUBE_MAP_SEAMLESS);
		//glCullFace(GL_FRONT);
		glDepthFunc(GL_LEQUAL);
		glClearColor(0.1f, 0.1f, 0.1f, 0.1f);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		glEnableVertexAttribArray(0);

		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_CUBE_MAP, sky.cubemap);
		glDepthMask(GL_FALSE);
		shader.use();
		shader.setMat4("view_proj", view_proj);
		shader.setInt("skybox", 0);
		glDrawArrays(GL_TRIANGLES, 0, 36);

		glDepthFunc(GL_LESS);
		glDepthMask(GL_TRUE);
	}
	glBindVertexArray(0);
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
	glActiveTexture(GL_TEXTURE4);
	glBindTexture(GL_TEXTURE_CUBE_MAP, sky.cubemap);

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
	shader.setInt("skybox", 4);
	shader.setFloat("refractStrength", debug.refreaction_power);
	shader.setFloat("moveFactor", moveFactor);
	shader.setFloat("waveLength", debug.wave_length);
	mesh.draw();
}


int main() {
	GLFWwindow* window = initWindow("Assignment 0", screenWidth, screenHeight);

	ew::Shader land_shader = ew::Shader("assets/landmass.vert", "assets/landmass.frag");
	ew::Shader water_shader = ew::Shader("assets/water.vert", "assets/water.frag");
	ew::Shader sky_shader = ew::Shader("assets/skybox.vert", "assets/skybox.frag");
	ew::Shader light_shader = ew::Shader("assets/lightSphere.vert", "assets/lightSphere.frag");
	ew::Shader depth_shader = ew::Shader("assets/depth.vert", "assets/depth.frag");
	GLuint 	heightmap = ew::loadTexture("assets/heightmap.png");
	GLuint 	normalmap = ew::loadTexture("assets/NormalMap.png");

	sky.init();
	depthbuffer.init();

	water_material.dudv = ew::loadTexture("assets/DuDvMap.png");
	water_material.normal = ew::loadTexture("assets/water_normal.png");
	water_material.spec = ew::loadTexture("assets/wave_spec.png");
	water_material.warp = ew::loadTexture("assets/wave_warp.png"); 

	//sky_material.cubemap = ew::loadTexture("assets/SkyBox.png");

	ew::Mesh islandPlane;
	ew::Mesh waterPlane;

	waterBuffers[WATER_REFLECTION].init();
	waterBuffers[WATER_REFRACTION].init();
	lightBuffer.init();

	islandPlane.load(ew::createPlane(50.0f, 50.0f, 100));
	waterPlane.load(ew::createPlane(50.0f, 50.0f, 1));
	lightSphere.load(ew::createSphere(1.0f, 12));

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

		// SHADOW_MAP
		glBindFramebuffer(GL_FRAMEBUFFER, depthbuffer.fbo);
		{
			const auto light_proj = glm::ortho(-50.0f, 50.0f, -50.0f, 50.0f, 0.1f, 100.0f);
			const auto light_view = glm::lookAt(light.lightPosition, glm::vec3(0.0f), glm::vec3(0.0f, 1.0f, 0.0f));
			const auto light_view_proj = light_proj * light_view;

			glEnable(GL_DEPTH_TEST);
			glViewport(0, 0, 2048, 2048);
			glClear(GL_DEPTH_BUFFER_BIT);
			glEnable(GL_CULL_FACE);
			glCullFace(GL_FRONT);

			glActiveTexture(GL_TEXTURE0);
			glBindTexture(GL_TEXTURE_2D, heightmap);

			depth_shader.use();
			depth_shader.setInt("heightmap", 0);
			depth_shader.setFloat("landmass.scale", debug.land_scale);
			depth_shader.setMat4("model", glm::mat4(1.0f));
			depth_shader.setMat4("light_view_proj", light_view_proj);

			islandPlane.draw();
			glCullFace(GL_BACK);
		}
		glBindFramebuffer(GL_FRAMEBUFFER, 0);

		glViewport(0, 0, 800, 600);

		// WATER_REFLECTION:
		glBindFramebuffer(GL_FRAMEBUFFER, waterBuffers[WATER_REFLECTION].fbo);
		{
			glEnable(GL_CLIP_DISTANCE0);
			glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
			glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

			float distY = 2.0f * (camera.position.y - debug.water_height);
			camera.position.y -= distY;
			cameraController.pitch *= -1.0f;
			recalculateCamera();

			render_sky(sky_shader);
			render_light(light_shader, lightSphere, glm::vec4(0.0, 1.0, 0.0, -debug.water_height));
			render_terrain(heightmap,normalmap, land_shader, islandPlane, glm::vec4(0.0, 1.0, 0.0, -debug.water_height));

			camera.position.y += distY;
			cameraController.pitch *= -1.0f;
			recalculateCamera();
		}

		// WATER_REFRACTION:
		glBindFramebuffer(GL_FRAMEBUFFER, waterBuffers[WATER_REFRACTION].fbo);
		{
			glEnable(GL_CLIP_DISTANCE0);
			glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
			glViewport(0, 0, 800, 600);
			glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
			render_terrain(heightmap,normalmap, land_shader, islandPlane, glm::vec4(0.0, -1.0, 0.0, debug.water_height));
		}
		glBindFramebuffer(GL_FRAMEBUFFER, 0);

		glViewport(0, 0, screenWidth, screenHeight);
		render_sky(sky_shader);
		render_light(light_shader, lightSphere, glm::vec4(0.0f));
		render_terrain(heightmap,normalmap, land_shader, islandPlane, glm::vec4(0.0f));
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
	ImGui::SliderFloat("Water Length", &debug.wave_length, 0.0f, 500.0f);
	ImGui::SliderFloat("Refractions", &debug.refreaction_power, 0.0f, 10.0f);
	ImGui::SliderInt("Tiles", &debug.tiles, 0.0f, 10.0f);
	ImGui::Text("Light");
	ImGui::SliderFloat3("LightPosition", &light.lightPosition[0], -20, 20);
	ImGui::ColorEdit3("LightColor", &light.lightColor[0]);

	ImGui::Text("(%.2f, %.2f, %.2f)", camera.position.x, camera.position.y, camera.position.z);

	ImVec2 size = { 400.0f, 300.0f };
	ImGui::Text("Shadow Map (depthbuffer.depth)");
	ImGui::Image((ImTextureID)depthbuffer.depth, size);
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

