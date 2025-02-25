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

void framebufferSizeCallback(GLFWwindow* window, int width, int height);
GLFWwindow* initWindow(const char* title, int width, int height);
ew::Camera camera;
ew::CameraController cameraController;
ew::Mesh plane;
ew::Transform monkeyTransform;
ew::Transform planeTransform;

int textureIndex = 0;
float bias = 0.005f;
struct Material
{
	glm::vec3 Ka = glm::vec3{1.0f};
	glm::vec3 Kd = glm::vec3{ 0.5f };
	glm::vec3 Ks = glm::vec3{ 0.5f };
	float alpha = 1.0f;
	float Shininess = 76.8f;
}material;

struct Light
{
	glm::vec3 position = glm::vec3(10.0f, 5.0f, 0.0f);
	glm::vec3 color = glm::vec3(1.0f, 1.0f, 1.0f);
}light;

struct DepthBuffer {

	GLuint fbo;
	GLuint depth;

	void Init()
	{
		glCreateFramebuffers(1, &fbo);
		glBindFramebuffer(GL_FRAMEBUFFER, fbo);

		glGenTextures(1, &depth);
		glBindTexture(GL_TEXTURE_2D, depth);
		//glTexStorage2D(GL_TEXTURE_2D, 1, GL_DEPTH_COMPONENT16, 2048, 2048);
		glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH24_STENCIL8, 2048, 2048, 0, GL_DEPTH_STENCIL, GL_UNSIGNED_INT_24_8, nullptr);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
		float borderColor[4] = { 1.0f,1.0f,1.0f,1.0f };
		glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_TEXTURE_2D,depth,0);
		//glTexParameterfv(GL_TEXTURE_2D, GL_TEXTURE_BORDER_COLOR, borderColor);

		glDrawBuffer(GL_NONE);
		glReadBuffer(GL_NONE);

		glBindFramebuffer(GL_FRAMEBUFFER, 0);

	}
} depthbuffer;
void drawUI();
void drawScene(ew::Model model, ew::Shader shader, glm::mat4 lightViewProjection);
//Global state
int screenWidth = 1080;
int screenHeight = 720;
float prevFrameTime;
float deltaTime;


int main() {
	GLFWwindow* window = initWindow("Assignment 0", screenWidth, screenHeight);
	ew::Shader blinphong = ew::Shader("assets/b_light.vert", "assets/b_light.frag");
	ew::Shader shadow = ew::Shader("assets/shadow.vert", "assets/shadow.frag");
	plane.load(ew::createPlane(50.0f, 50.0f, 100));
	ew::Model monkeyModel = ew::Model("assets/suzanne.fbx");

	monkeyTransform.position = glm::vec3(0.0f, 0.0f, 0.0f);
	planeTransform.position = glm::vec3(0.0f, -2.5f, 0.0f);
	camera.position = glm::vec3(0.0f, 0.0f, 5.0f);
	camera.target = glm::vec3(0.0f, 0.0f, 0.0f);
	camera.aspectRatio = (float)screenWidth / screenHeight;
	camera.fov = 60.0f;
	glfwSetFramebufferSizeCallback(window, framebufferSizeCallback);

	depthbuffer.Init();


	while (!glfwWindowShouldClose(window)) {
		glfwPollEvents();

		float time = (float)glfwGetTime();
		deltaTime = time - prevFrameTime;
		prevFrameTime = time;

		cameraController.move(window, &camera, deltaTime);

		const auto light_proj = glm::ortho(-10.0f, 10.0f, -10.0f, 10.0f, 0.1f, 100.0f);
		const auto light_view = glm::lookAt(light.position, glm::vec3(0.0f), glm::vec3(0.0f, 1.0f, 0.0f));
		const auto light_view_proj = light_proj * light_view;

		const auto camera_view_proj = camera.projectionMatrix() * camera.viewMatrix();

		//RENDER
		monkeyTransform.rotation = glm::rotate(monkeyTransform.rotation, deltaTime, glm::vec3(0.0, 1.0, 0.0));

		glBindFramebuffer(GL_FRAMEBUFFER, depthbuffer.fbo);
		{
			glEnable(GL_DEPTH_TEST);
			glViewport(0, 0, 2048, 2048);
			glClear(GL_DEPTH_BUFFER_BIT);
			glCullFace(GL_FRONT);

			shadow.use();
			shadow.setMat4("_Model", monkeyTransform.modelMatrix());
			shadow.setMat4("_LightViewProjection", light_view_proj);

			monkeyModel.draw();
		}
		glBindFramebuffer(GL_FRAMEBUFFER, 0);
		
		// draw scene
		{
			glEnable(GL_CULL_FACE);
			glCullFace(GL_BACK);
			glEnable(GL_DEPTH_TEST);

			glViewport(0, 0, screenWidth, screenHeight);

			glClearColor(0.2f, 0.3f, 0.3f, 1.0f);
			glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

			glActiveTexture(GL_TEXTURE0);
			glBindTexture(GL_TEXTURE_2D, depthbuffer.depth);

			blinphong.use();
			blinphong.setInt("shadow_map", 0);
			blinphong.setMat4("_CameraViewProjection", camera_view_proj);
			blinphong.setMat4("_LightViewProjection", light_view_proj);
			blinphong.setMat4("_Model", monkeyTransform.modelMatrix());
			blinphong.setVec3("_Material.Ka", material.Ka);
			blinphong.setVec3("_Material.Kd", material.Kd);
			blinphong.setVec3("_Material.Ks", material.Ks);
			blinphong.setFloat("_Material.alpha", material.alpha);
			blinphong.setFloat("_Material.Shininess", material.Shininess);
			blinphong.setVec3("_LightColor", light.color);
			blinphong.setVec3("light_pos", light.position);
			blinphong.setVec3("camera_pos", camera.position);
			blinphong.setFloat("bias", bias);


			monkeyModel.draw();

			blinphong.setMat4("_Model", planeTransform.modelMatrix());
			plane.draw();
		}

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
void drawScene(ew::Model model, ew::Shader shader, glm::mat4 lightViewProjection)
{

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
	ImGui::BeginChild("Shadow Map");
	ImVec2 size = ImGui::GetWindowSize();
	ImGui::Image((ImTextureID)depthbuffer.depth, size, ImVec2(0,1),ImVec2(1,0));
	ImGui::EndChild();
	ImGui::SliderFloat("Bias", &bias, 0.000001f, 0.05f);
	ImGui::SliderFloat3("LightPos", &light.position.x, -5.0f, 5.0f);
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




