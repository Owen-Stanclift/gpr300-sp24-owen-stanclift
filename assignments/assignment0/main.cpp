#include <stdio.h>
#include <math.h>

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

void framebufferSizeCallback(GLFWwindow* window, int width, int height);
GLFWwindow* initWindow(const char* title, int width, int height);
ew::Camera camera;
ew::CameraController cameraController;

struct Material
{
	glm::vec3 Ka = glm::vec3(0.02f, 0.17f,0.02f);
	glm::vec3 Kd = glm::vec3(0.07f, 0.6f, 0.07f);
	glm::vec3 Ks = glm::vec3(0.63f, 0.72f, 0.63f);
	float Shininess = 0.6f;
}material;

void drawUI();

//Global state
int screenWidth = 1080;
int screenHeight = 720;
float prevFrameTime;
float deltaTime;


int main() {
	GLFWwindow* window = initWindow("Assignment 0", screenWidth, screenHeight);
	ew::Shader shader = ew::Shader("assets/lit.vert", "assets/lit.frag");
	GLuint brickTexture = ew::loadTexture("assets/emerald.jpg");
	ew::Model monkeyModel = ew::Model("assets/suzanne.fbx");
	ew::Transform monkeyTransform;
	glEnable(GL_CULL_FACE);
	glCullFace(GL_BACK);
	glEnable(GL_DEPTH_TEST);
	glBindTextureUnit(0, brickTexture);

	camera.position = glm::vec3(0.0f, 0.0f, 5.0f);
	camera.target = glm::vec3(0.0f, 0.0f, 0.0f);
	camera.aspectRatio = (float)screenWidth / screenHeight;
	camera.fov = 60.0f;
	glfwSetFramebufferSizeCallback(window, framebufferSizeCallback);

	while (!glfwWindowShouldClose(window)) {
		glfwPollEvents();

		float time = (float)glfwGetTime();
		deltaTime = time - prevFrameTime;
		prevFrameTime = time;

		cameraController.move(window, &camera, deltaTime);

		//RENDER
		glClearColor(0.6f,0.8f,0.92f,1.0f);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		monkeyTransform.rotation = glm::rotate(monkeyTransform.rotation, deltaTime, glm::vec3(0.0, 1.0, 0.0));

		shader.use();
		shader.setInt("_MainTex", 0);
		shader.setVec3("_EyePos", camera.position);
		shader.setMat4("_Model", monkeyTransform.modelMatrix());
		shader.setMat4("_ViewProjection", camera.projectionMatrix() * camera.viewMatrix());
		shader.setVec3("_Material.Ka", material.Ka);
		shader.setVec3("_Material.Kd", material.Kd);
		shader.setVec3("_Material.Ks", material.Ks);
		shader.setFloat("_Material.Shininess", material.Shininess);
		monkeyModel.draw();


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
	if (ImGui::Button("Reset Camera"))
	{
		resetCamera(&camera, &cameraController);
	}
	if (ImGui::CollapsingHeader("Material"))
	{
		if (ImGui::CollapsingHeader("AmbientK"))
		{
			ImGui::SliderFloat("AmbientR", &material.Ka.x, 0.0f, 1.0f);
			ImGui::SliderFloat("AmbientG", &material.Ka.y, 0.0f, 1.0f);
			ImGui::SliderFloat("AmbientB", &material.Ka.z, 0.0f, 1.0f);
		}

		if (ImGui::CollapsingHeader("DiffuseK"))
		{
			ImGui::SliderFloat("DiffuseR", &material.Kd.x, 0.0f, 1.0f);
			ImGui::SliderFloat("DiffuseG", &material.Kd.y, 0.0f, 1.0f);
			ImGui::SliderFloat("DiffuseB", &material.Kd.z, 0.0f, 1.0f);
		}
		if (ImGui::CollapsingHeader("SpecularK"))
		{
			ImGui::SliderFloat("SpecularR", &material.Ks.x, 0.0f, 1.0f);
			ImGui::SliderFloat("SpecularG", &material.Ks.y, 0.0f, 1.0f);
			ImGui::SliderFloat("SpecularB", &material.Ks.z, 0.0f, 1.0f);
		}
		ImGui::SliderFloat("Shininess", &material.Shininess, 0.0f, 1024.0f);
	}
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




