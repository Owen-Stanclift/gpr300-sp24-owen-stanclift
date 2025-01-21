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
glm::vec3 shadow = glm::vec3(1.0f);
glm::vec3 highlight = glm::vec3(1.0f, 0.0f, 0.0f);
void drawUI();

//Global state
int screenWidth = 1080;
int screenHeight = 720;
float prevFrameTime;
float deltaTime;


int main() {
	GLFWwindow* window = initWindow("Assignment 0", screenWidth, screenHeight);
	ew::Shader toonShader = ew::Shader("assets/toonLit.vert", "assets/toonLit.frag");
	GLuint 	skullTexture = ew::loadTexture("assets/Txo_dokuo.png");
	GLuint 	toonTexture = ew::loadTexture("assets/ZAtoon.png");
	ew::Model skullModel = ew::Model("assets/skull.obj");
	ew::Transform skullTransform;

	camera.position = glm::vec3(0.0f, 0.0f, 50.0f);
	camera.target = glm::vec3(0.0f, 0.0f, 0.0f);
	camera.aspectRatio = (float)screenWidth / screenHeight;
	camera.fov = 60.0f;
	glfwSetFramebufferSizeCallback(window, framebufferSizeCallback);

	while (!glfwWindowShouldClose(window)) {
		glfwPollEvents();

		float time = (float)glfwGetTime();
		deltaTime = time - prevFrameTime;
		prevFrameTime = time;

		cameraController.move(window, &camera, deltaTime*2);

		//RENDER
		glClearColor(0.6f, 0.8f, 0.92f, 1.0f);
		glEnable(GL_CULL_FACE);
		glCullFace(GL_BACK);
		glEnable(GL_DEPTH_TEST);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		glBindTextureUnit(0, skullTexture);
		glBindTextureUnit(1, toonTexture);
		skullTransform.rotation = glm::rotate(skullTransform.rotation, deltaTime, glm::vec3(0.0, 1.0, 0.0));



		toonShader.use();
		toonShader.setInt("_Albedo", 0);
		toonShader.setInt("_ZATOON", 1);
		toonShader.setVec3("shadow", shadow);
		toonShader.setVec3("highlight", highlight);
		toonShader.setVec3("_EyePos", camera.position);
		toonShader.setMat4("_Model", skullTransform.modelMatrix());
		toonShader.setMat4("_ViewProjection", camera.projectionMatrix() * camera.viewMatrix());
		skullModel.draw();


		drawUI();

		glfwSwapBuffers(window);
	}
	printf("Shutting down...");
}
void resetCamera(ew::Camera* camera, ew::CameraController* controller)
{
	camera->position = glm::vec3(0, 0, 20.0f);
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
	if (ImGui::CollapsingHeader("Shadow"))
	{
		ImGui::SliderFloat("sR", &shadow.x, 0.0f, 1.0f);
		ImGui::SliderFloat("sG", &shadow.y, 0.0f, 1.0f);
		ImGui::SliderFloat("sB", &shadow.z, 0.0f, 1.0f);
	}
	if (ImGui::CollapsingHeader("Highlight"))
	{
		ImGui::SliderFloat("hR", &highlight.x, 0.0f, 1.0f);
		ImGui::SliderFloat("hG", &highlight.y, 0.0f, 1.0f);
		ImGui::SliderFloat("hB", &highlight.z, 0.0f, 1.0f);
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




