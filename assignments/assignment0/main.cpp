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

int textureIndex = 0;
struct Material
{
	glm::vec3 Ka = glm::vec3(0.02f, 0.17f,0.02f);
	glm::vec3 Kd = glm::vec3(0.07f, 0.61f, 0.07f);
	glm::vec3 Ks = glm::vec3(0.63f, 0.72f, 0.63f);
	float alpha = 0.55f;
	float Shininess = 76.8f;
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
	GLuint 	emeraldTexture = ew::loadTexture("assets/emerald.jpg");
	GLuint 	brassTexture = ew::loadTexture("assets/brass.jpg");
	GLuint 	bronzeTexture = ew::loadTexture("assets/bronze.jpg");
	GLuint 	copperTexture = ew::loadTexture("assets/copper.jpg");
	GLuint 	silverTexture = ew::loadTexture("assets/silver.jpg");
	GLuint 	goldTexture = ew::loadTexture("assets/gold.jpg");
	GLuint 	rubyTexture = ew::loadTexture("assets/ruby.jpg");
	GLuint 	pearlTexture = ew::loadTexture("assets/pearl.jpg");
	GLuint 	turquoiseTexture = ew::loadTexture("assets/turquoise.jpg");
	GLuint 	brickTexture = ew::loadTexture("assets/brick_color.jpg");
	ew::Model monkeyModel = ew::Model("assets/suzanne.fbx");
	ew::Transform monkeyTransform;

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
		glClearColor(0.6f, 0.8f, 0.92f, 1.0f);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		glEnable(GL_CULL_FACE);
		glEnable(GL_BLEND);
		glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
		glCullFace(GL_BACK);
		glEnable(GL_DEPTH_TEST);
		glBindTextureUnit(0, emeraldTexture);
		glBindTextureUnit(1, brassTexture);
		glBindTextureUnit(2, bronzeTexture);
		glBindTextureUnit(3, copperTexture);
		glBindTextureUnit(4, silverTexture);
		glBindTextureUnit(5, goldTexture);
		glBindTextureUnit(6, rubyTexture);
		glBindTextureUnit(7, pearlTexture);
		glBindTextureUnit(8, turquoiseTexture);
		glBindTextureUnit(9, brickTexture);

		monkeyTransform.rotation = glm::rotate(monkeyTransform.rotation, deltaTime, glm::vec3(0.0, 1.0, 0.0));

		shader.use();
		shader.setInt("_MainTex", textureIndex);
		shader.setVec3("_EyePos", camera.position);
		shader.setMat4("_Model", monkeyTransform.modelMatrix());
		shader.setMat4("_ViewProjection", camera.projectionMatrix() * camera.viewMatrix());
		shader.setVec3("_Material.Ka", material.Ka);
		shader.setVec3("_Material.Kd", material.Kd);
		shader.setVec3("_Material.Ks", material.Ks);
		shader.setFloat("_Material.alpha", material.alpha);
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
		if (ImGui::Button("Emerald"))
		{
			textureIndex = 0;
			material.Ka = glm::vec3(0.02, 0.17, 0.02);
			material.Kd = glm::vec3(0.08, 0.61, 0.08);
			material.Ks = glm::vec3(0.63, 0.73, 0.63);
			material.alpha = 0.55f;
			material.Shininess = 76.8f;
		}
		if (ImGui::Button("Brass"))
		{
			textureIndex = 1;
			material.Ka = glm::vec3(0.33, 0.22, 0.03);
			material.Kd = glm::vec3(0.78, 0.57, 0.11);
			material.Ks = glm::vec3(0.99, 0.94, 0.81);
			material.alpha = 1.0f;
			material.Shininess = 27.9f;
		}
		if (ImGui::Button("Bronze"))
		{
			textureIndex = 2;
			material.Ka = glm::vec3(0.21, 0.13, 0.05);
			material.Kd = glm::vec3(0.71, 0.43, 0.18);
			material.Ks = glm::vec3(0.39, 0.27, 0.17);
			material.alpha = 1.0f;
			material.Shininess = 25.6f;
			
		}
		if (ImGui::Button("Copper"))
		{
			textureIndex = 3;
			material.Ka = glm::vec3(0.19, 0.07, 0.02);
			material.Kd = glm::vec3(0.70, 0.27, 0.08);
			material.Ks = glm::vec3(0.26, 0.14, 0.09);
			material.alpha = 1.0f;
			material.Shininess = 12.8f;
			
		}
		if (ImGui::Button("Silver"))
		{
			textureIndex = 4;
			material.Ka = glm::vec3(0.19, 0.19, 0.19);
			material.Kd = glm::vec3(0.51, 0.51, 0.51);
			material.Ks = glm::vec3(0.51, 0.51, 0.51);
			material.alpha = 1.0f;
			material.Shininess = 51.2f;
			
		}
		if (ImGui::Button("Gold"))
		{
			textureIndex = 5;
			material.Ka = glm::vec3(0.25, 0.20, 0.07);
			material.Kd = glm::vec3(0.75, 0.61, 0.23);
			material.Ks = glm::vec3(0.63, 0.56, 0.37);
			material.alpha = 1.0f;
			material.Shininess = 51.2f;
		}
		if (ImGui::Button("Ruby"))
		{
			textureIndex = 6;
			material.Ka = glm::vec3(0.17, 0.01, 0.01);
			material.Kd = glm::vec3(0.61, 0.04, 0.04);
			material.Ks = glm::vec3(0.73, 0.63, 0.63);
			material.alpha = 0.55f;
			material.Shininess = 76.8f;
		
		}
		if (ImGui::Button("Pearl"))
		{
			textureIndex = 7;
			material.Ka = glm::vec3(0.25, 0.21, 0.21);
			material.Kd = glm::vec3(1.0, 0.83, 0.83);
			material.Ks = glm::vec3(0.3, 0.3, 0.3);
			material.alpha = 0.92f;
			material.Shininess = 11.3f;
			
		}
		if (ImGui::Button("Turquoise"))
		{
			textureIndex = 8;
			material.Ka = glm::vec3(0.1, 0.19, 0.17);
			material.Kd = glm::vec3(0.4, 0.74, 0.69);
			material.Ks = glm::vec3(0.30, 0.31, 0.31);
			material.alpha = 0.8f;
			material.Shininess = 12.8f;
		}
		if (ImGui::Button("Brick"))
		{
			textureIndex = 9;
			material.Ka = glm::vec3(1.0, 1.0, 1.0);
			material.Kd = glm::vec3(0.5, 0.5, 0.5);
			material.Ks = glm::vec3(0.5, 0.5, 0.5);
			material.alpha = 1.0f;
			material.Shininess = 128.0f;
		}
	}
		if (ImGui::CollapsingHeader("Custom"))
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
			ImGui::SliderFloat("Alpha", &material.alpha, 0.0f, 1.0f);
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




