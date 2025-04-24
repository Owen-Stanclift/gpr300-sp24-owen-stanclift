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
#include <stdlib.h>


void framebufferSizeCallback(GLFWwindow* window, int width, int height);
GLFWwindow* initWindow(const char* title, int width, int height);
ew::Camera camera;
ew::CameraController cameraController;
ew::Mesh plane;
ew::Mesh lightSphere;
ew::Transform monkeyTransform;
ew::Transform planeTransform;

int textureIndex = 0;
float bias = 0.005f;
struct Material
{
	glm::vec3 Ka = glm::vec3{ 1.0f };
	glm::vec3 Kd = glm::vec3{ 0.5f };
	glm::vec3 Ks = glm::vec3{ 0.5f };
	float alpha = 1.0f;
	float Shininess = 76.8f;
}material;

struct {
	float radius = 1.0f;
} debug;

constexpr int FrameWidth = 800;
constexpr int FrameHeight = 600;
struct FrameBuffer {

	GLuint fbo;
	GLuint color0;
	GLuint color1;
	GLuint color2;
	GLuint depth;

	void Init()
	{
		glGenFramebuffers(1, &fbo);
		glBindFramebuffer(GL_FRAMEBUFFER, fbo);

		glGenTextures(1, &color0);
		glBindTexture(GL_TEXTURE_2D, color0);
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, 800, 600, 0, GL_RGB, GL_UNSIGNED_BYTE, nullptr);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, color0, 0);

		glGenTextures(1, &color1);
		glBindTexture(GL_TEXTURE_2D, color1);
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, 800, 600, 0, GL_RGB, GL_UNSIGNED_BYTE, nullptr);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT1, GL_TEXTURE_2D, color1, 0);

		glGenTextures(1, &color2);
		glBindTexture(GL_TEXTURE_2D, color2);
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, 800, 600, 0, GL_RGB, GL_UNSIGNED_BYTE, nullptr);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT2, GL_TEXTURE_2D, color2, 0);

		GLuint attachments[3] = { GL_COLOR_ATTACHMENT0,GL_COLOR_ATTACHMENT1, GL_COLOR_ATTACHMENT2 };
		glDrawBuffers(3, attachments);

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
	}
}framebuffer;

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
		glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_TEXTURE_2D, depth, 0);
		//glTexParameterfv(GL_TEXTURE_2D, GL_TEXTURE_BORDER_COLOR, borderColor);

		glDrawBuffer(GL_NONE);
		glReadBuffer(GL_NONE);

		glBindFramebuffer(GL_FRAMEBUFFER, 0);

	}
} depthbuffer;

struct FullscreenQuad
{
	GLuint vao;
	GLuint vbo;

	void Init() {
		static float quad_vertices[] =
		{
			// pos (x, y) texcoord (u, v)
			-1.0f,  1.0f, 0.0f, 1.0f,
			-1.0f, -1.0f, 0.0f, 0.0f,
			1.0f, -1.0f, 1.0f, 0.0f,

			-1.0f,  1.0f, 0.0f, 1.0f,
			1.0f, -1.0f, 1.0f, 0.0f,
			1.0f,  1.0f, 1.0f, 1.0f,
		};

		glGenVertexArrays(1, &fullscreen_quad.vao);
		glBindVertexArray(fullscreen_quad.vao);
		{
			glGenBuffers(1, &fullscreen_quad.vbo);
			glBindBuffer(GL_ARRAY_BUFFER, fullscreen_quad.vbo);

			glBufferData(GL_ARRAY_BUFFER, sizeof(quad_vertices), &quad_vertices, GL_STATIC_DRAW);


			glEnableVertexAttribArray(0);
			glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)0);
			glEnableVertexAttribArray(1);
			glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)(sizeof(float) * 2));
		}
		glBindVertexArray(0);
	}
}fullscreen_quad;

void drawUI();
void drawScene(ew::Model model, ew::Shader shader, glm::mat4 lightViewProjection);

//Global state
int screenWidth = 1080;
int screenHeight = 720;
float prevFrameTime;
float deltaTime;


int main() {
	srand(time(NULL));
	GLFWwindow* window = initWindow("Assignment 0", screenWidth, screenHeight);
	ew::Shader geometry = ew::Shader("assets/experiment.vert", "assets/experiment.frag");
	ew::Shader exLight = ew::Shader("assets/experiment_light.vert", "assets/experiment_light.frag");
	ew::Shader lightShader = ew::Shader("assets/light.vert", "assets/light.frag");
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

	framebuffer.Init();
	depthbuffer.Init();
	fullscreen_quad.Init();

	lightSphere.load(ew::createSphere(2.0f, 4));
	glm::vec3 storedLights[64];
	glm::vec3 storedColor[64];
	int count = 0;
	for (int i = -3; i <= 4; i++)
	{
		for (int j = -3; j <= 4; j++)
		{
			float rColor = static_cast<float>(((rand() % 100) / 200.0f) + 0.5); // between 0.5 and 1.0
			float gColor = static_cast<float>(((rand() % 100) / 200.0f) + 0.5); // between 0.5 and 1.0
			float bColor = static_cast<float>(((rand() % 100) / 200.0f) + 0.5); // between 0.5 and 1.0
			storedLights[count] = glm::vec3(i * 2.75f, 2, j * 2.75f);
			storedColor[count] = glm::vec3(rColor, gColor, bColor);
			count++;
		}
	}

	while (!glfwWindowShouldClose(window)) {
		glfwPollEvents();
		//const auto light_proj = glm::ortho(-10.0f, 10.0f, -10.0f, 10.0f, 0.1f, 100.0f);
		//const auto light_view = glm::lookAt(light.position, glm::vec3(0.0f), glm::vec3(0.0f, 1.0f, 0.0f));
		//const auto light_view_proj = light_proj * light_view;
		float time = (float)glfwGetTime();
		deltaTime = time - prevFrameTime;
		prevFrameTime = time;

		cameraController.move(window, &camera, deltaTime);


		const auto camera_view_proj = camera.projectionMatrix() * camera.viewMatrix();

		// draw scene
		// here we want to render geometric data (position, normal, and out albedo)
		glBindFramebuffer(GL_FRAMEBUFFER, framebuffer.fbo);
		{
			glEnable(GL_CULL_FACE);
			glCullFace(GL_BACK);

			glEnable(GL_DEPTH_TEST);

			glViewport(0, 0, 800, 600);

			glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
			glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

			// NPTE: this is our geomtric-shader

			geometry.use();
			geometry.setMat4("_CameraViewProjection", camera_view_proj);
			geometry.setMat4("_Model", glm::translate(planeTransform.position));
			plane.draw();
			
			for (auto i = -3; i <= 4; i++)
			{
				for (auto j = -3; j <= 4; j++)
				{
					geometry.setMat4("_Model", glm::translate(glm::vec3(i*2.75f,0,j*2.75f)));
					monkeyModel.draw();
				}
			}

			// draw plane
		}
		glBindFramebuffer(GL_FRAMEBUFFER, 0);

		// render post process
		// this we will use blinnphong, with info from our g-buffers (position, normal, and out albedo)
		{
			lightShader.use();

				//Fragment
			// FIXME:
			lightShader.setInt("albedo", 0);
			lightShader.setInt("position", 1);
			lightShader.setInt("normal", 2);
			lightShader.setFloat("radius", debug.radius);
			lightShader.setVec3("cameraPos", camera.position);

			for (auto i = 0; i < 64; i++) {
				lightShader.setVec3("lights[" + std::to_string(i) + "].position", storedLights[i]);
				lightShader.setVec3("lights[" + std::to_string(i) + "].color", storedColor[i]);
				// printf("(%.2f, %.2f, %.2f)", storedLights[i].x, storedLights[i].y, storedLights[i].z);
			}

			glDisable(GL_DEPTH_TEST);

			glViewport(0, 0, screenWidth, screenHeight);

			glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
			glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

			glBindVertexArray(fullscreen_quad.vao);

			glActiveTexture(GL_TEXTURE0);
			glBindTexture(GL_TEXTURE_2D, framebuffer.color0);

			glActiveTexture(GL_TEXTURE1);
			glBindTexture(GL_TEXTURE_2D, framebuffer.color1);

			glActiveTexture(GL_TEXTURE2);
			glBindTexture(GL_TEXTURE_2D, framebuffer.color2);

			glDrawArrays(GL_TRIANGLES, 0, 6);

			glBindVertexArray(0);
		}

		// render all spheres using shader called "light_sphere"
		// consider this a debug step
		// ...
		//lightProcess(lightShader);
		{
			// https://learnopengl.com/Advanced-Lighting/Deferred-Shading
			glEnable(GL_DEPTH_TEST);
			glDepthMask(GL_TRUE);
			glDepthFunc(GL_LESS);


			glBindFramebuffer(GL_READ_FRAMEBUFFER, framebuffer.fbo);
			glBindFramebuffer(GL_DRAW_FRAMEBUFFER, 0);
			glBlitFramebuffer(
				0, 0, 800, 600,
				0, 0, screenWidth, screenHeight,
				GL_DEPTH_BUFFER_BIT, GL_NEAREST
			);
			glBindFramebuffer(GL_FRAMEBUFFER, 0);

			exLight.use();
			exLight.setMat4("_CameraViewProjection", camera_view_proj);
			for (int i = 0; i < 64; i++)
			{
				exLight.setMat4("_Model", glm::translate(storedLights[i]));
				exLight.setVec3("color", storedColor[i]);
				lightSphere.draw();
			}

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

void drawUI() {
	ImGui_ImplGlfw_NewFrame();
	ImGui_ImplOpenGL3_NewFrame();
	ImGui::NewFrame();

	ImGui::Begin("Settings");
	if (ImGui::Button("Reset Camera"))
	{
		resetCamera(&camera, &cameraController);
	}

	ImGui::SliderFloat("Light Radius", &debug.radius, 0.5f, 10.0f);

	ImVec2 size = ImGui::GetWindowSize();
	ImGui::Image((ImTextureID)framebuffer.color0, size, ImVec2(0, 1), ImVec2(1, 0));
	ImGui::Image((ImTextureID)framebuffer.color1, size, ImVec2(0, 1), ImVec2(1, 0));
	ImGui::Image((ImTextureID)framebuffer.color2, size, ImVec2(0, 1), ImVec2(1, 0));
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




