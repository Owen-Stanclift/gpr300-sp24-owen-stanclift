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

static int effect_index = 0;
static std::vector<std::string> post_processing_effects = {
	"None",
	"Grayscale",
	"Kernel Blur",
	"Inverse",
	"Chromatic Aberration",
	"VIGNETTE",
	"HDR",
	"Gaussian",
	"Sharpen",
	"Ridge",
	"Edge",
	"Gamma",
	"GammaDisplay",
	"Fog",
};

void framebufferSizeCallback(GLFWwindow* window, int width, int height);
void render(ew::Shader, ew::Model);
void post_process(ew::Shader);
GLFWwindow* initWindow(const char* title, int width, int height);
ew::Camera camera;
ew::CameraController cameraController;
ew::Transform monkeyTransform;
int textureIndex = 0;
struct Material
{
	glm::vec3 Ka = glm::vec3(1.00f, 1.00, 1.00f);
	glm::vec3 Kd = glm::vec3(0.5f, 0.5f, 0.5f);
	glm::vec3 Ks = glm::vec3(0.5f, 0.5f, 0.5f);
	float alpha = 1.0f;
	float Shininess = 128.0f;
}material;
struct Framebuffer
{
	GLuint fbo;
	GLuint color0;
	GLuint color1;
	GLuint depth;
}framebuffer;

struct FullscreenQuad
{
	GLuint vao;
	GLuint vbo;
}fullscreen_quad;

enum
{
	EFFECT_NONE =0,
	EFFECT_GRAYSCALE = 1,
	EFFECT_BLUR = 2,
	EFFECT_INVERSE = 3,
	EFFECT_CHROMATIC = 4,
	EFFECT_VIGNETTE = 5,
	EFFECT_HDR = 6,
	EFFECT_GAUSSIAN = 7,
	EFFECT_SHAPREN = 8,
	EFFECT_RIDGE = 9,
	EFFECT_EDGE = 10,
	EFFECT_GAMMA = 11,
	EFFECT_GAMMADISPLAY = 12,
	EFFECT_FOG = 13,
};
GLuint 	brickTexture;
void drawUI();

//Global state
int screenWidth = 1080;
int screenHeight = 720;
float prevFrameTime;
float deltaTime;


int main() {
	GLFWwindow* window = initWindow("Assignment 0", screenWidth, screenHeight);
	ew::Shader shader = ew::Shader("assets/lit.vert", "assets/lit.frag");

	std::vector<ew::Shader> effects = {
		ew::Shader("assets/fullscreen.vert", "assets/fullscreen.frag"),
		ew::Shader("assets/fullscreen.vert", "assets/grayscale.frag"),
		ew::Shader("assets/fullscreen.vert", "assets/blur.frag"),
		ew::Shader("assets/fullscreen.vert", "assets/inverse.frag"),
		ew::Shader("assets/fullscreen.vert", "assets/chromatic.frag"),
		ew::Shader("assets/fullscreen.vert", "assets/vignette.frag"),
		ew::Shader("assets/fullscreen.vert", "assets/hdr.frag"),
		ew::Shader("assets/fullscreen.vert", "assets/gaussian.frag"),
		ew::Shader("assets/fullscreen.vert", "assets/sharpen.frag"),
		ew::Shader("assets/fullscreen.vert", "assets/ridge.frag"),
		ew::Shader("assets/fullscreen.vert", "assets/edge.frag"),
		ew::Shader("assets/fullscreen.vert", "assets/gamma.frag"),
		ew::Shader("assets/fullscreen.vert", "assets/gammaDisplay.frag"),
		ew::Shader("assets/fullscreen.vert", "assets/fog.frag"),
		ew::Shader("assets/fullscreen.vert", "assets/bloom.frag"),
		ew::Shader("assets/fullscreen.vert", "assets/blend.frag"),
	};

	ew::Model monkeyModel = ew::Model("assets/Suzanne.fbx");
	brickTexture = ew::loadTexture("assets/brick_color.jpg");
	camera.position = glm::vec3(0.0f, 0.0f, 5.0f);
	camera.target = glm::vec3(0.0f, 0.0f, 0.0f);
	camera.aspectRatio = (float)screenWidth / screenHeight;
	camera.fov = 60.0f;
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


	// frb
	glGenFramebuffers(1, &framebuffer.fbo);
	glBindFramebuffer(GL_FRAMEBUFFER, framebuffer.fbo);
	{
		glGenTextures(1, &framebuffer.color0);
		glBindTexture(GL_TEXTURE_2D, framebuffer.color0);
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, 800, 600, 0, GL_RGB, GL_UNSIGNED_BYTE, nullptr);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, framebuffer.color0, 0);

		glGenTextures(1, &framebuffer.color1);
		glBindTexture(GL_TEXTURE_2D, framebuffer.color1);
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, 800, 600, 0, GL_RGB, GL_UNSIGNED_BYTE, nullptr);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT1, GL_TEXTURE_2D, framebuffer.color1, 0);

		GLuint attachments[2] = { GL_COLOR_ATTACHMENT0,GL_COLOR_ATTACHMENT1 };
		glDrawBuffers(2, attachments);

		glGenTextures(1, &framebuffer.depth);
		glBindTexture(GL_TEXTURE_2D, framebuffer.depth);
		glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT, 800, 600, 0, GL_DEPTH_COMPONENT, GL_UNSIGNED_BYTE, nullptr);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, framebuffer.depth, 0);

		unsigned int pingpongFBO[2];
		unsigned int pingpongBuffer[2];
		glGenFramebuffers(2, pingpongFBO);
		glGenTextures(2, pingpongBuffer);
		for (unsigned int i = 0; i < 2; i++)
		{
			glBindFramebuffer(GL_FRAMEBUFFER, pingpongFBO[i]);
			glBindTexture(GL_TEXTURE_2D, pingpongBuffer[i]);
			glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA16F, 800, 600, 0, GL_RGBA, GL_FLOAT, NULL);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
			glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, pingpongBuffer[i], 0);
		}
	}
	glBindFramebuffer(GL_FRAMEBUFFER, 0);
	glfwSetFramebufferSizeCallback(window, framebufferSizeCallback);

	while (!glfwWindowShouldClose(window)) {
		glfwPollEvents();

		float time = (float)glfwGetTime();
		deltaTime = time - prevFrameTime;
		prevFrameTime = time;

		cameraController.move(window, &camera, deltaTime);

		//RENDER
		render(shader, monkeyModel);

		post_process(effects[effect_index]);

		// pipelione


		// pass



		// bindings
		// draw



		drawUI();

		glfwSwapBuffers(window);
	}
	printf("Shutting down...");
}

void render(ew::Shader shader, ew::Model model)
{
	glBindFramebuffer(GL_FRAMEBUFFER, framebuffer.fbo);
	{
		// pipeline
		glEnable(GL_CULL_FACE);
		glEnable(GL_BLEND);
		glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
		glCullFace(GL_BACK);
		glEnable(GL_DEPTH_TEST);

		// pass
		glClearColor(0.6f, 0.8f, 0.92f, 1.0f);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		// bindings
		glBindTextureUnit(0, brickTexture);

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
		model.draw();
	}
	glBindFramebuffer(GL_FRAMEBUFFER,0);
}

void post_process(ew::Shader shader)
{
	glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	glDisable(GL_DEPTH_TEST);

	shader.use();
	shader.setInt("_MainTex", 0);

	//glDepthMask(GL_FALSE);
//glDepthFunc(GL_LESS);
	switch (effect_index)
{
case EFFECT_GRAYSCALE:
    break;
case EFFECT_BLUR:
	shader.setFloat("strength", 16.0f);
    break;
case EFFECT_INVERSE:
    break;
case EFFECT_CHROMATIC:
    break;
case EFFECT_VIGNETTE:
	shader.setVec2("resolution", glm::vec2(800, 600));
	shader.setFloat("radius", 0.5f);
	shader.setFloat("softness", 0.02f);
    break;
case EFFECT_HDR:
	shader.setFloat("exposure", 1.0f);
case EFFECT_FOG:

default:
    break;
}
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, framebuffer.color0);
	glBindVertexArray(fullscreen_quad.vao);
	{
		glDrawArrays(GL_TRIANGLES, 0, 6);
	}
	glBindVertexArray(0);
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

	if (ImGui::BeginCombo("Effect", post_processing_effects[effect_index].c_str()))
	{
		for (auto n = 0; n < post_processing_effects.size(); ++n)
		{
			auto is_selected = (post_processing_effects[effect_index] == post_processing_effects[n]);
			if (ImGui::Selectable(post_processing_effects[n].c_str(), is_selected))
			{
				effect_index = n;
			}
			if (is_selected)
			{
				ImGui::SetItemDefaultFocus();
			}
		}
		ImGui::EndCombo();
	}

	ImGui::Begin("Screen");
	ImGui::Image((ImTextureID)(intptr_t) framebuffer.color0, ImVec2(800, 600));
	ImGui::Image((ImTextureID)(intptr_t) framebuffer.color1, ImVec2(800, 600));
	
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




