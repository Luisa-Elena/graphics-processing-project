//
//  main.cpp
//  OpenGL Advances Lighting
//
//  Created by CGIS on 28/11/16.
//  Copyright � 2016 CGIS. All rights reserved.
//

#if defined (__APPLE__)
    #define GLFW_INCLUDE_GLCOREARB
    #define GL_SILENCE_DEPRECATION
#else
    #define GLEW_STATIC
    #include <GL/glew.h>
#endif

#include <GLFW/glfw3.h>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/matrix_inverse.hpp>
#include <glm/gtc/type_ptr.hpp>

#include "Shader.hpp"
#include "Model3D.hpp"
#include "Camera.hpp"
#include "SkyBox.hpp"

#include <iostream>

int glWindowWidth = 800;
int glWindowHeight = 600;
int retina_width, retina_height;
GLFWwindow* glWindow = NULL;

const unsigned int SHADOW_WIDTH = 4096;
const unsigned int SHADOW_HEIGHT = 4096;

glm::mat4 model;
GLuint modelLoc;
glm::mat4 view;
GLuint viewLoc;
glm::mat4 projection;
GLuint projectionLoc;
glm::mat3 normalMatrix;
GLuint normalMatrixLoc;
glm::mat4 lightRotation;

glm::vec3 lightDir;
GLuint lightDirLoc;
glm::vec3 lightColor;
GLuint lightColorLoc;


glm::vec3 pointLightPos;
GLuint pointLightPosLoc;
glm::vec3 pointLightColor;
GLuint pointLightColorLoc;
glm::vec3 pointLightPos1;
GLuint pointLightPosLoc1;
glm::vec3 pointLightColor1;
GLuint pointLightColorLoc1;

glm::vec3 spotLightPos;
GLuint spotLightPosLoc;
glm::vec3 spotLightColor;
GLuint spotLightColorLoc;
glm::vec3 spotLightDir;
GLuint spotLightDirLoc;

bool pointLightEnabled = false;
bool spotLightEnabled = false;
bool moveCameraEnabled = false;
bool generateObj = false;
bool rainEnabled = false;

gps::SkyBox mySkyBox;
gps::Shader skyboxShader;

gps::Camera myCamera(
				glm::vec3(0.0f, 2.2f, 7.5f), 
				glm::vec3(0.0f, 0.0f, 0.0f),
				glm::vec3(0.0f, 1.0f, 0.0f));

float cameraSpeed = 0.05f;

bool pressedKeys[1024];
float angleY = 0.0f;
GLfloat lightAngle;

gps::Model3D cat;
gps::Model3D ground;
gps::Model3D lightCube;
gps::Model3D screenQuad;
gps::Model3D house;
gps::Model3D tree;
gps::Model3D polytree;
gps::Model3D beagle;
gps::Model3D dog;
gps::Model3D duck;
gps::Model3D fantana;
gps::Model3D maner;
gps::Model3D flying;
gps::Model3D lamp;
gps::Model3D human;
gps::Model3D flashlight;
gps::Model3D flower;

gps::Shader myCustomShader;
gps::Shader lightShader;
gps::Shader screenQuadShader;
gps::Shader depthMapShader;

gps::Shader rainShader;

GLuint shadowMapFBO;
GLuint depthMapTexture;

glm::vec3 duckPosition(2.6f, -0.299f, -1.8f); // duck's initial position
glm::mat4 duckModelMatrix; // model matrix for the duck

struct AABB {
	glm::vec3 min;
	glm::vec3 max;
};

AABB wellAABB = { glm::vec3(2.9f, 0.0f, -2.4f), glm::vec3(3.7f, 0.0f, -1.6f) };


bool showDepthMap;

GLenum glCheckError_(const char *file, int line) {
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

void windowResizeCallback(GLFWwindow* window, int width, int height) {
	fprintf(stdout, "window resized to width: %d , and height: %d\n", width, height);
	//TODO	
}

void keyboardCallback(GLFWwindow* window, int key, int scancode, int action, int mode) {
	if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS)
		glfwSetWindowShouldClose(window, GL_TRUE);

	if (key == GLFW_KEY_M && action == GLFW_PRESS)
		showDepthMap = !showDepthMap;

	if (key == GLFW_KEY_P && action == GLFW_PRESS)
		pointLightEnabled = !pointLightEnabled;
	if (key == GLFW_KEY_O && action == GLFW_PRESS)
		spotLightEnabled = !spotLightEnabled;
	if (key == GLFW_KEY_Z && action == GLFW_PRESS)
		moveCameraEnabled = !moveCameraEnabled;
	if (key == GLFW_KEY_G && action == GLFW_PRESS)
		generateObj = true;
	if (key == GLFW_KEY_R && action == GLFW_PRESS)
		rainEnabled = !rainEnabled;

	if (key >= 0 && key < 1024)
	{
		if (action == GLFW_PRESS)
			pressedKeys[key] = true;
		else if (action == GLFW_RELEASE)
			pressedKeys[key] = false;
	}
}

float yaw = -90.0f;
float pitch = 0.0f;
float lastX = glWindowWidth / 2.0f;
float lastY = glWindowHeight / 2.0f;
bool firstMouse = true;
void mouseCallback(GLFWwindow* window, double xpos, double ypos)
{
	if (firstMouse) {
		lastX = xpos;
		lastY = ypos;
		firstMouse = false;
	}

	float xoffset = xpos - lastX;
	float yoffset = lastY - ypos;
	lastX = xpos;
	lastY = ypos;

	float sensitivity = 0.1f;
	xoffset *= sensitivity;
	yoffset *= sensitivity;

	yaw += xoffset;
	pitch += yoffset;

	if (pitch > 89.0f)
		pitch = 89.0f;
	if (pitch < -89.0f)
		pitch = -89.0f;

	myCamera.rotate(pitch, yaw);
}

AABB computeDuckAABB(glm::mat4 model) {
	glm::vec3 duckMin(-0.1f, 0.0f, -0.1f);
	glm::vec3 duckMax(0.1f, 0.0f, 0.1f);

	glm::vec3 transformedMin = glm::vec3(model * glm::vec4(duckMin, 1.0f));
	glm::vec3 transformedMax = glm::vec3(model * glm::vec4(duckMax, 1.0f));

	return { transformedMin, transformedMax };
}

bool checkCollision(const AABB& a, const AABB& b) {
	return (a.max.x >= b.min.x && a.min.x <= b.max.x) &&
		(a.max.z >= b.min.z && a.min.z <= b.max.z);
}

void updateDuckPosition(glm::vec3& position, const glm::vec3& movement) {
	glm::vec3 newPosition = position + movement;

	glm::mat4 tempModelMatrix = glm::translate(glm::mat4(1.0f), newPosition);

	AABB duckAABB = computeDuckAABB(tempModelMatrix);

	if (!checkCollision(duckAABB, wellAABB)) {
		position = newPosition;
	}
}

void processMovement()
{
	if (pressedKeys[GLFW_KEY_Q]) {
		angleY -= 1.0f;
	}

	if (pressedKeys[GLFW_KEY_E]) {
		angleY += 1.0f;
	}

	if (pressedKeys[GLFW_KEY_J]) {
		lightAngle -= 1.0f;		
	}

	if (pressedKeys[GLFW_KEY_L]) {
		lightAngle += 1.0f;
	}

	if (pressedKeys[GLFW_KEY_W]) {
		myCamera.move(gps::MOVE_FORWARD, cameraSpeed);		
	}

	if (pressedKeys[GLFW_KEY_S]) {
		myCamera.move(gps::MOVE_BACKWARD, cameraSpeed);		
	}

	if (pressedKeys[GLFW_KEY_A]) {
		myCamera.move(gps::MOVE_LEFT, cameraSpeed);		
	}

	if (pressedKeys[GLFW_KEY_D]) {
		myCamera.move(gps::MOVE_RIGHT, cameraSpeed);		
	}

	if (pressedKeys[GLFW_KEY_T]) {
		glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
	}

	if (pressedKeys[GLFW_KEY_Y]) {
		glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
	}


	glm::vec3 movement(0.0f);
	float speed = 0.01f; // Adjust movement speed
	if (pressedKeys[GLFW_KEY_UP]) {
		movement.z -= speed; // Move forward
	}
	if (pressedKeys[GLFW_KEY_DOWN]) {
		movement.z += speed; // Move backward
	}
	if (pressedKeys[GLFW_KEY_LEFT]) {
		movement.x -= speed; // Move left
	}
	if (pressedKeys[GLFW_KEY_RIGHT]) {
		movement.x += speed; // Move right
	}

	updateDuckPosition(duckPosition, movement);
}

bool initOpenGLWindow()
{
	if (!glfwInit()) {
		fprintf(stderr, "ERROR: could not start GLFW3\n");
		return false;
	}

	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 1);
	glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    
    
    //window scaling for HiDPI displays
    glfwWindowHint(GLFW_SCALE_TO_MONITOR, GLFW_TRUE);

    //for sRBG framebuffer
    glfwWindowHint(GLFW_SRGB_CAPABLE, GLFW_TRUE);

    //for antialising
    glfwWindowHint(GLFW_SAMPLES, 4);

	glWindow = glfwCreateWindow(glWindowWidth, glWindowHeight, "OpenGL Shader Example", NULL, NULL);
	if (!glWindow) {
		fprintf(stderr, "ERROR: could not open window with GLFW3\n");
		glfwTerminate();
		return false;
	}

	glfwSetWindowSizeCallback(glWindow, windowResizeCallback);
	glfwSetKeyCallback(glWindow, keyboardCallback);
	glfwSetCursorPosCallback(glWindow, mouseCallback);
	//glfwSetInputMode(glWindow, GLFW_CURSOR, GLFW_CURSOR_DISABLED);

	glfwMakeContextCurrent(glWindow);

	glfwSwapInterval(1);

#if not defined (__APPLE__)
    // start GLEW extension handler
    glewExperimental = GL_TRUE;
    glewInit();
#endif

	// get version info
	const GLubyte* renderer = glGetString(GL_RENDERER); // get renderer string
	const GLubyte* version = glGetString(GL_VERSION); // version as a string
	printf("Renderer: %s\n", renderer);
	printf("OpenGL version supported %s\n", version);

	//for RETINA display
	glfwGetFramebufferSize(glWindow, &retina_width, &retina_height);

	return true;
}

void initOpenGLState()
{
	glClearColor(0.3f, 0.3f, 0.3f, 1.0f);
	glViewport(0, 0, retina_width, retina_height);

	glEnable(GL_DEPTH_TEST); // enable depth-testing
	glDepthFunc(GL_LESS); // depth-testing interprets a smaller value as "closer"
	glEnable(GL_CULL_FACE); // cull face
	glCullFace(GL_BACK); // cull back face
	glFrontFace(GL_CCW); // GL_CCW for counter clock-wise

	glEnable(GL_FRAMEBUFFER_SRGB);
}

void initObjects() {
	cat.LoadModel("objects/cat/12221_Cat_v1_l3.obj");
	ground.LoadModel("objects/ground/10450_Rectangular_Grass_Patch_v1_iterations-2.obj");
	lightCube.LoadModel("objects/cube/cube.obj");
	screenQuad.LoadModel("objects/quad/quad.obj");
	house.LoadModel("objects/house/house.obj");
	tree.LoadModel("objects/tree/Tree.obj");
	polytree.LoadModel("objects/polytree/lowpoyltree.obj");
	beagle.LoadModel("objects/beagle/13041_Beagle_v1_L1.obj");
	dog.LoadModel("objects/dog/13463_Australian_Cattle_Dog_v3.obj");
	duck.LoadModel("objects/duck/12248_Bird_v1_L2.obj");
	fantana.LoadModel("objects/fantana/fantana.obj");
	maner.LoadModel("objects/fantana/maner_merged_v2.obj");
	flying.LoadModel("objects/flying/Bat_Quad.obj");
	lamp.LoadModel("objects/lamp/Street_Lamp_7.obj");
	human.LoadModel("objects/human/Humano_01Business_01_100K.obj");
	flashlight.LoadModel("objects/flashlight/Flashlight.obj");
	flower.LoadModel("objects/flower/12973_anemone_flower_v1_l2.obj");
}

void initShaders() {
	myCustomShader.loadShader("shaders/shaderStart.vert", "shaders/shaderStart.frag");
	myCustomShader.useShaderProgram();
	lightShader.loadShader("shaders/lightCube.vert", "shaders/lightCube.frag");
	lightShader.useShaderProgram();
	screenQuadShader.loadShader("shaders/screenQuad.vert", "shaders/screenQuad.frag");
	screenQuadShader.useShaderProgram();

	depthMapShader.loadShader("shaders/depthMap.vert", "shaders/depthMap.frag");
	depthMapShader.useShaderProgram();

	skyboxShader.loadShader("shaders/skyboxShader.vert", "shaders/skyboxShader.frag");
	skyboxShader.useShaderProgram();
}

void initUniforms() {
	myCustomShader.useShaderProgram();

	model = glm::mat4(1.0f);
	modelLoc = glGetUniformLocation(myCustomShader.shaderProgram, "model");
	glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));

	view = myCamera.getViewMatrix();
	viewLoc = glGetUniformLocation(myCustomShader.shaderProgram, "view");
	glUniformMatrix4fv(viewLoc, 1, GL_FALSE, glm::value_ptr(view));
	
	normalMatrix = glm::mat3(glm::inverseTranspose(view*model));
	normalMatrixLoc = glGetUniformLocation(myCustomShader.shaderProgram, "normalMatrix");
	glUniformMatrix3fv(normalMatrixLoc, 1, GL_FALSE, glm::value_ptr(normalMatrix));
	
	projection = glm::perspective(glm::radians(45.0f), (float)retina_width / (float)retina_height, 0.1f, 1000.0f);
	projectionLoc = glGetUniformLocation(myCustomShader.shaderProgram, "projection");
	glUniformMatrix4fv(projectionLoc, 1, GL_FALSE, glm::value_ptr(projection));

	//set the light direction (direction towards the light)
	lightDir = glm::vec3(0.0f, 2.7f, 1.0f);
	lightRotation = glm::rotate(glm::mat4(1.0f), glm::radians(lightAngle), glm::vec3(0.0f, 1.0f, 0.0f));
	lightDirLoc = glGetUniformLocation(myCustomShader.shaderProgram, "lightDir");	
	glUniform3fv(lightDirLoc, 1, glm::value_ptr(glm::inverseTranspose(glm::mat3(view * lightRotation)) * lightDir));

	//set light color
	lightColor = glm::vec3(1.0f, 1.0f, 1.0f); //white light
	lightColorLoc = glGetUniformLocation(myCustomShader.shaderProgram, "lightColor");
	glUniform3fv(lightColorLoc, 1, glm::value_ptr(lightColor));

	//point lights 
	pointLightPos = glm::vec3(0.0f, 1.5f, -20.0f);
	pointLightPosLoc = glGetUniformLocation(myCustomShader.shaderProgram, "pointLightPos");
	glUniform3fv(pointLightPosLoc, 1, glm::value_ptr(glm::mat3(view) * pointLightPos));

	pointLightColor = glm::vec3(1.0, 0.85, 0.6);
	pointLightColorLoc = glGetUniformLocation(myCustomShader.shaderProgram, "pointLightColor");
	glUniform3fv(pointLightColorLoc, 1, glm::value_ptr(pointLightColor));

	pointLightPos1 = glm::vec3(5.0f, 1.5f, -20.0f);
	pointLightPosLoc1 = glGetUniformLocation(myCustomShader.shaderProgram, "pointLightPos1");
	glUniform3fv(pointLightPosLoc1, 1, glm::value_ptr(glm::mat3(view) * pointLightPos1));

	pointLightColor1 = glm::vec3(1.0, 0.85, 0.6);
	pointLightColorLoc1 = glGetUniformLocation(myCustomShader.shaderProgram, "pointLightColor1");
	glUniform3fv(pointLightColorLoc1, 1, glm::value_ptr(pointLightColor1));

	//spotlight
	spotLightPos = glm::vec3(-3.0f, 2.0f, -8.0f);
	spotLightPosLoc = glGetUniformLocation(myCustomShader.shaderProgram, "spotLightPos");
	glUniform3fv(spotLightPosLoc, 1, glm::value_ptr(glm::mat3(view) * spotLightPos));

	spotLightColor = glm::vec3(1.0f, 1.0f, 1.0f);
	spotLightColorLoc = glGetUniformLocation(myCustomShader.shaderProgram, "spotLightColor");
	glUniform3fv(spotLightColorLoc, 1, glm::value_ptr(spotLightColor));

	spotLightDir = glm::vec3(0.0f, -1.0f, 0.0f);
	spotLightDirLoc = glGetUniformLocation(myCustomShader.shaderProgram, "spotLightDir");
	glUniform3fv(spotLightDirLoc, 1, glm::value_ptr(glm::mat3(view) * spotLightDir));

	float spotLightCutOff = glm::cos(glm::radians(12.5f));
	GLint spotLightCutOffLoc = glGetUniformLocation(myCustomShader.shaderProgram, "spotLightCutOff");
	glUniform1f(spotLightCutOffLoc, spotLightCutOff);

	float spotLightOuterCutOff = glm::cos(glm::radians(17.5f));
	GLint spotLightOuterCutOffLoc = glGetUniformLocation(myCustomShader.shaderProgram, "spotLightOuterCutOff");
	glUniform1f(spotLightOuterCutOffLoc, spotLightOuterCutOff);


	lightShader.useShaderProgram();
	glUniformMatrix4fv(glGetUniformLocation(lightShader.shaderProgram, "projection"), 1, GL_FALSE, glm::value_ptr(projection));
}

void initFBO() {
	//TODO - Create the FBO, the depth texture and attach the depth texture to the FBO
	glGenFramebuffers(1, &shadowMapFBO);
	glGenTextures(1, &depthMapTexture);
	glBindTexture(GL_TEXTURE_2D, depthMapTexture);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT, SHADOW_WIDTH, SHADOW_HEIGHT, 0, GL_DEPTH_COMPONENT, GL_FLOAT, NULL);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	float borderColor[] = { 1.0f, 1.0f, 1.0f, 1.0f };
	glTexParameterfv(GL_TEXTURE_2D, GL_TEXTURE_BORDER_COLOR, borderColor);

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);

	glBindFramebuffer(GL_FRAMEBUFFER, shadowMapFBO);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, depthMapTexture, 0);

	glDrawBuffer(GL_NONE);
	glReadBuffer(GL_NONE);

	glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

glm::mat4 computeLightSpaceTrMatrix() {
	//TODO - Return the light-space transformation matrix
	lightRotation = glm::rotate(glm::mat4(1.0f), glm::radians(lightAngle), glm::vec3(0.0f, 1.0f, 0.0f));

	glm::mat4 lightView = glm::lookAt(glm::vec3(lightRotation * glm::vec4(lightDir, 1.0f)), glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f, 1.0f, 0.0f));
	const GLfloat near_plane = 0.1f, far_plane = 20.0f;
	//glm::mat4 lightProjection = glm::ortho(-1.0f, 1.0f, -1.0f, 1.0f, near_plane, far_plane);
	glm::mat4 lightProjection = glm::ortho(-5.0f, 5.0f, -5.0f, 5.0f, near_plane, far_plane);
	glm::mat4 lightSpaceTrMatrix = lightProjection * lightView;

	return lightSpaceTrMatrix;
}

void renderDuck(gps::Shader shader, bool depthPass) {
	// Update the duck's model matrix
	duckModelMatrix = glm::translate(glm::mat4(1.0f), duckPosition);
	duckModelMatrix = glm::scale(duckModelMatrix, glm::vec3(0.005f));
	duckModelMatrix = glm::rotate(duckModelMatrix, glm::radians(angleY), glm::vec3(0.0f, 1.0f, 0.0f));
	duckModelMatrix = glm::rotate(duckModelMatrix, glm::radians(270.0f), glm::vec3(1.0f, 0.0f, 0.0f));

	// Pass the model matrix to the shader
	glUniformMatrix4fv(glGetUniformLocation(shader.shaderProgram, "model"), 1, GL_FALSE, glm::value_ptr(duckModelMatrix));

	if (!depthPass) {
		glm::mat3 normalMatrix = glm::mat3(glm::inverseTranspose(view * duckModelMatrix));
		glUniformMatrix3fv(normalMatrixLoc, 1, GL_FALSE, glm::value_ptr(normalMatrix));
	}

	duck.Draw(shader);
}


void drawObjects(gps::Shader shader, bool depthPass) {
		
	shader.useShaderProgram();
	
	model = glm::mat4(1.0f);
	model = glm::translate(model, glm::vec3(-0.5f, -0.57f, -2.2f));
	model = glm::rotate(model, glm::radians(330.0f), glm::vec3(0.0f, 1.0f, 0.0f));
	model = glm::rotate(model, glm::radians(270.0f), glm::vec3(1.0f, 0.0f, 0.0f));
	model = glm::scale(model, glm::vec3(0.01f));
	glUniformMatrix4fv(glGetUniformLocation(shader.shaderProgram, "model"), 1, GL_FALSE, glm::value_ptr(model));
	
	// do not send the normal matrix if we are rendering in the depth map
	if (!depthPass) {
		normalMatrix = glm::mat3(glm::inverseTranspose(view * model));
		glUniformMatrix3fv(normalMatrixLoc, 1, GL_FALSE, glm::value_ptr(normalMatrix));
	}

	cat.Draw(shader);

	model = glm::translate(glm::mat4(1.0f), glm::vec3(0.0f, -1.3f, 0.0f));
	model = glm::rotate(model, glm::radians(270.0f), glm::vec3(1.0f, 0.0f, 0.0f));
	model = glm::scale(model, glm::vec3(0.1f));
	glUniformMatrix4fv(glGetUniformLocation(shader.shaderProgram, "model"), 1, GL_FALSE, glm::value_ptr(model));

	// Apply the normal matrix for rendering (if needed)
	if (!depthPass) {
		normalMatrix = glm::mat3(glm::inverseTranspose(view * model));
		glUniformMatrix3fv(normalMatrixLoc, 1, GL_FALSE, glm::value_ptr(normalMatrix));
	}

	ground.Draw(shader);  // Draw the ground with the applied transformations

	model = glm::translate(glm::mat4(1.0f), glm::vec3(-2.3f, -0.6f, -3.5f));
	model = glm::rotate(model, glm::radians(320.0f), glm::vec3(0.0f, 1.0f, 0.0f));
	model = glm::scale(model, glm::vec3(0.2f));
	glUniformMatrix4fv(glGetUniformLocation(shader.shaderProgram, "model"), 1, GL_FALSE, glm::value_ptr(model));

	if (!depthPass) {
		normalMatrix = glm::mat3(glm::inverseTranspose(view * model));
		glUniformMatrix3fv(normalMatrixLoc, 1, GL_FALSE, glm::value_ptr(normalMatrix));
	}

	house.Draw(shader);

	model = glm::translate(glm::mat4(1.0f), glm::vec3(-4.0f, -1.5f, -2.0f));
	model = glm::scale(model, glm::vec3(0.7f));
	glUniformMatrix4fv(glGetUniformLocation(shader.shaderProgram, "model"), 1, GL_FALSE, glm::value_ptr(model));

	if (!depthPass) {
		normalMatrix = glm::mat3(glm::inverseTranspose(view * model));
		glUniformMatrix3fv(normalMatrixLoc, 1, GL_FALSE, glm::value_ptr(normalMatrix));
	}

	polytree.Draw(shader);

	model = glm::translate(glm::mat4(1.0f), glm::vec3(0.7f, -0.5f, -3.5f));
	model = glm::scale(model, glm::vec3(0.3f));

	// Add wind effect: slight rotation
	float currentTime = glfwGetTime();
	float windStrength = 2.0f; // Maximum sway angle in degrees
	float windFrequency = 0.5f; // Oscillation frequency
	float randomPhase = 2.0f;   // Per-tree randomization factor

	float swayAngle = windStrength * sin(windFrequency * currentTime + randomPhase);
	model = glm::rotate(model, glm::radians(swayAngle), glm::vec3(0.0f, 0.0f, 1.0f)); // Rotate around Z-axis

	glUniformMatrix4fv(glGetUniformLocation(shader.shaderProgram, "model"), 1, GL_FALSE, glm::value_ptr(model));

	if (!depthPass) {
		normalMatrix = glm::mat3(glm::inverseTranspose(view * model));
		glUniformMatrix3fv(normalMatrixLoc, 1, GL_FALSE, glm::value_ptr(normalMatrix));
	}

	tree.Draw(shader);
	
	model = glm::translate(glm::mat4(1.0f), glm::vec3(1.5f, -0.5f, -3.5f));
	model = glm::scale(model, glm::vec3(0.2f));

	// Add wind effect: slight rotation
	currentTime = glfwGetTime();
	windStrength = 3.0f;
	windFrequency = 0.5f;
	randomPhase = 2.0f;

	swayAngle = windStrength * sin(windFrequency * currentTime + randomPhase);
	model = glm::rotate(model, glm::radians(swayAngle), glm::vec3(0.0f, 0.0f, 1.0f)); // Rotate around Z-axis

	glUniformMatrix4fv(glGetUniformLocation(shader.shaderProgram, "model"), 1, GL_FALSE, glm::value_ptr(model));

	if (!depthPass) {
		normalMatrix = glm::mat3(glm::inverseTranspose(view * model));
		glUniformMatrix3fv(normalMatrixLoc, 1, GL_FALSE, glm::value_ptr(normalMatrix));
	}

	tree.Draw(shader);
	
	model = glm::translate(glm::mat4(1.0f), glm::vec3(2.0f, -0.5f, -3.5f));
	model = glm::scale(model, glm::vec3(0.3f));

	// Add wind effect: slight rotation
	currentTime = glfwGetTime();
	windStrength = 5.0f;
	windFrequency = 0.5f;
	randomPhase = 2.0f;

	swayAngle = windStrength * sin(windFrequency * currentTime + randomPhase);
	model = glm::rotate(model, glm::radians(swayAngle), glm::vec3(0.0f, 0.0f, 1.0f)); // Rotate around Z-axis

	glUniformMatrix4fv(glGetUniformLocation(shader.shaderProgram, "model"), 1, GL_FALSE, glm::value_ptr(model));

	if (!depthPass) {
		normalMatrix = glm::mat3(glm::inverseTranspose(view * model));
		glUniformMatrix3fv(normalMatrixLoc, 1, GL_FALSE, glm::value_ptr(normalMatrix));
	}

	tree.Draw(shader);


	model = glm::translate(glm::mat4(1.0f), glm::vec3(2.8f, -0.5f, -3.5f));
	model = glm::scale(model, glm::vec3(0.2f));

	// Add wind effect: slight rotation
	currentTime = glfwGetTime();
	windStrength = 3.0f; // Maximum sway angle in degrees
	windFrequency = 0.5f; // Oscillation frequency
	randomPhase = 2.0f;   // Per-tree randomization factor

	swayAngle = windStrength * sin(windFrequency * currentTime + randomPhase);
	model = glm::rotate(model, glm::radians(swayAngle), glm::vec3(0.0f, 0.0f, 1.0f)); // Rotate around Z-axis

	glUniformMatrix4fv(glGetUniformLocation(shader.shaderProgram, "model"), 1, GL_FALSE, glm::value_ptr(model));

	if (!depthPass) {
		normalMatrix = glm::mat3(glm::inverseTranspose(view * model));
		glUniformMatrix3fv(normalMatrixLoc, 1, GL_FALSE, glm::value_ptr(normalMatrix));
	}

	tree.Draw(shader);

	model = glm::translate(glm::mat4(1.0f), glm::vec3(-3.1f, -0.6f, -1.7f));
	model = glm::scale(model, glm::vec3(0.008f));
	model = glm::rotate(model, glm::radians(270.0f), glm::vec3(1.0f, 0.0f, 0.0f));
	model = glm::rotate(model, glm::radians(140.0f), glm::vec3(0.0f, 0.0f, 1.0f));
	glUniformMatrix4fv(glGetUniformLocation(shader.shaderProgram, "model"), 1, GL_FALSE, glm::value_ptr(model));

	if (!depthPass) {
		normalMatrix = glm::mat3(glm::inverseTranspose(view * model));
		glUniformMatrix3fv(normalMatrixLoc, 1, GL_FALSE, glm::value_ptr(normalMatrix));
	}

	beagle.Draw(shader);

	model = glm::translate(glm::mat4(1.0f), glm::vec3(1.2f, -0.4f, -3.0f));
	model = glm::scale(model, glm::vec3(0.028f));
	model = glm::rotate(model, glm::radians(270.0f), glm::vec3(1.0f, 0.0f, 0.0f));
	model = glm::rotate(model, glm::radians(330.0f), glm::vec3(0.0f, 0.0f, 1.0f));
	glUniformMatrix4fv(glGetUniformLocation(shader.shaderProgram, "model"), 1, GL_FALSE, glm::value_ptr(model));

	if (!depthPass) {
		normalMatrix = glm::mat3(glm::inverseTranspose(view * model));
		glUniformMatrix3fv(normalMatrixLoc, 1, GL_FALSE, glm::value_ptr(normalMatrix));
	}

	dog.Draw(shader);

	model = glm::translate(glm::mat4(1.0f), glm::vec3(3.2f, -0.38f, -2.0f));
	model = glm::scale(model, glm::vec3(0.06f));
	model = glm::rotate(model, glm::radians(-60.0f), glm::vec3(0.0f, 1.0f, 0.0f));
	glUniformMatrix4fv(glGetUniformLocation(shader.shaderProgram, "model"), 1, GL_FALSE, glm::value_ptr(model));

	if (!depthPass) {
		normalMatrix = glm::mat3(glm::inverseTranspose(view * model));
		glUniformMatrix3fv(normalMatrixLoc, 1, GL_FALSE, glm::value_ptr(normalMatrix));
	}

	fantana.Draw(shader);

	
	model = glm::translate(glm::mat4(1.0f), glm::vec3(3.4f, 0.27f, -1.68f));
	model = glm::scale(model, glm::vec3(0.06f));
	model = glm::rotate(model, glm::radians(-60.0f), glm::vec3(0.0f, 1.0f, 0.0f));
	model = glm::rotate(model, glm::radians(angleY), glm::vec3(1.0f, 0.0f, 0.0f));
	glUniformMatrix4fv(glGetUniformLocation(shader.shaderProgram, "model"), 1, GL_FALSE, glm::value_ptr(model));

	if (!depthPass) {
		normalMatrix = glm::mat3(glm::inverseTranspose(view * model));
		glUniformMatrix3fv(normalMatrixLoc, 1, GL_FALSE, glm::value_ptr(normalMatrix));
	}

	maner.Draw(shader);

	renderDuck(shader, depthPass);

	model = glm::translate(glm::mat4(1.0f), glm::vec3(0.0f, -0.7f, -4.0f));
	glUniformMatrix4fv(glGetUniformLocation(shader.shaderProgram, "model"), 1, GL_FALSE, glm::value_ptr(model));

	if (!depthPass) {
		normalMatrix = glm::mat3(glm::inverseTranspose(view * model));
		glUniformMatrix3fv(normalMatrixLoc, 1, GL_FALSE, glm::value_ptr(normalMatrix));
	}

	lamp.Draw(shader);

	model = glm::translate(glm::mat4(1.0f), glm::vec3(3.0f, -0.7f, -4.0f));
	glUniformMatrix4fv(glGetUniformLocation(shader.shaderProgram, "model"), 1, GL_FALSE, glm::value_ptr(model));

	if (!depthPass) {
		normalMatrix = glm::mat3(glm::inverseTranspose(view * model));
		glUniformMatrix3fv(normalMatrixLoc, 1, GL_FALSE, glm::value_ptr(normalMatrix));
	}

	lamp.Draw(shader);

	float time = glfwGetTime();
	float scaleY = 0.0001f * sin(time);
	model = glm::translate(glm::mat4(1.0f), glm::vec3(-1.0f, -0.55f, -1.0f));
	model = glm::scale(model, glm::vec3(0.005f, 0.005f + scaleY, 0.005f));
	glUniformMatrix4fv(glGetUniformLocation(shader.shaderProgram, "model"), 1, GL_FALSE, glm::value_ptr(model));

	if (!depthPass) {
		normalMatrix = glm::mat3(glm::inverseTranspose(view * model));
		glUniformMatrix3fv(normalMatrixLoc, 1, GL_FALSE, glm::value_ptr(normalMatrix));
	}

	human.Draw(shader);

	
	model = glm::translate(glm::mat4(1.0f), glm::vec3(-2.1f, -0.19f, -2.95f));
	model = glm::scale(model, glm::vec3(0.01f));
	model = glm::rotate(model, glm::radians(67.0f), glm::vec3(0.0f, 1.0f, 0.0f));
	glUniformMatrix4fv(glGetUniformLocation(shader.shaderProgram, "model"), 1, GL_FALSE, glm::value_ptr(model));

	if (!depthPass) {
		normalMatrix = glm::mat3(glm::inverseTranspose(view * model));
		glUniformMatrix3fv(normalMatrixLoc, 1, GL_FALSE, glm::value_ptr(normalMatrix));
	}

	flashlight.Draw(shader);

}

struct Flower {
	double startTime;
	float randomX;
	float randomZ;
};
std::vector<Flower> activeFlowers;



void drawRandomObj(gps::Shader shader, bool depthPass) {
	if (generateObj) {
		Flower newFlower;
		newFlower.startTime = glfwGetTime();
		newFlower.randomX = -1.0f + static_cast<float>(rand()) / (RAND_MAX / 2.0f);
		newFlower.randomZ = 0.0f + static_cast<float>(rand()) / (RAND_MAX / 1.0f);

		activeFlowers.push_back(newFlower);
		generateObj = false;
	}

	for (size_t i = 0; i < activeFlowers.size(); ) {
		double currentTime = glfwGetTime();
		double elapsedTime = currentTime - activeFlowers[i].startTime;

		if (elapsedTime < 3.0) {
			shader.useShaderProgram();
			glm::mat4 model = glm::mat4(1.0f);

			model = glm::translate(model, glm::vec3(activeFlowers[i].randomX, -0.58f, activeFlowers[i].randomZ));
			model = glm::scale(model, glm::vec3(0.005f));
			model = glm::rotate(model, glm::radians(-90.0f), glm::vec3(1.0f, 0.0f, 0.0f));

			glUniformMatrix4fv(glGetUniformLocation(shader.shaderProgram, "model"), 1, GL_FALSE, glm::value_ptr(model));

			if (!depthPass) {
				normalMatrix = glm::mat3(glm::inverseTranspose(view * model));
				glUniformMatrix3fv(normalMatrixLoc, 1, GL_FALSE, glm::value_ptr(normalMatrix));
			}

			flower.Draw(shader);

			++i;
		}
		else {
			activeFlowers.erase(activeFlowers.begin() + i);
		}
	}
}





float angle = 0.0f; // Angle in degrees, starts at 0
float radius = 1.5f; // Radius of the circular motion

void drawAnimation(gps::Shader shader, bool depthPass) {
	shader.useShaderProgram();

	angle += 0.2f;
	if (angle >= 360.0f) {
		angle -= 360.0f;
	}

	// Calculate the circular position
	float x = radius * cos(glm::radians(angle));
	float z = radius * sin(glm::radians(angle));

	// Calculate the forward direction (tangent to the circle)
	float nextAngle = angle + 1.0f; // Slightly ahead angle for tangent calculation
	float nextX = radius * cos(glm::radians(nextAngle));
	float nextZ = radius * sin(glm::radians(nextAngle));

	glm::vec3 currentPosition = glm::vec3(x, 2.0f, z);
	glm::vec3 nextPosition = glm::vec3(nextX, 2.0f, nextZ);
	glm::vec3 forwardDirection = glm::normalize(nextPosition - currentPosition);

	// Calculate the rotation to align the object with the forward direction
	glm::vec3 upVector = glm::vec3(0.0f, 1.0f, 0.0f); // Up vector
	glm::vec3 rightDirection = glm::normalize(glm::cross(upVector, forwardDirection));
	glm::vec3 adjustedUp = glm::cross(forwardDirection, rightDirection);

	glm::mat4 rotationMatrix = glm::mat4(1.0f);
	rotationMatrix[0] = glm::vec4(rightDirection, 0.0f);   // X axis
	rotationMatrix[1] = glm::vec4(adjustedUp, 0.0f);         // Y axis
	rotationMatrix[2] = glm::vec4(forwardDirection, 0.0f); // Z axis

	// Transformation matrix
	glm::mat4 model = glm::mat4(1.0f);
	model = glm::translate(model, currentPosition); // Position the object
	model = model * rotationMatrix;                 // Align to the direction of motion

	// Add fixed incline rotation
	model = glm::rotate(model, glm::radians(40.0f), glm::vec3(0.0f, 0.0f, 1.0f)); // Tilt around Z-axis
	model = glm::rotate(model, glm::radians(320.0f), glm::vec3(0.0f, 1.0f, 0.0f)); // Tilt around Y-axis

	glUniformMatrix4fv(glGetUniformLocation(shader.shaderProgram, "model"), 1, GL_FALSE, glm::value_ptr(model));

	if (!depthPass) {
		normalMatrix = glm::mat3(glm::inverseTranspose(view * model));
		glUniformMatrix3fv(normalMatrixLoc, 1, GL_FALSE, glm::value_ptr(normalMatrix));
	}

	flying.Draw(shader);
}

double startTimeStamp = 0.0f;
void animateCamera() {
	
	float speed = 0.03f;

	if (!moveCameraEnabled) {
		return;
	}

	if (startTimeStamp == 0.0f) {
		myCamera.cameraPosition = glm::vec3(0.0f, 2.2f, 7.5f);
		startTimeStamp = glfwGetTime();
	}

	double currentTimeStamp = glfwGetTime();
	double elapsedTime = currentTimeStamp - startTimeStamp;

	if (elapsedTime < 0.5) {
		myCamera.move(gps::MOVE_FORWARD, speed);
	}
	else if (elapsedTime < 2.0) {
		myCamera.move(gps::MOVE_LEFT, speed);
	}
	if (elapsedTime < 2.5) {
		myCamera.move(gps::MOVE_FORWARD, speed);
	}
	else if (elapsedTime < 5.5) {
		myCamera.move(gps::MOVE_RIGHT, speed);
	}
	else if (elapsedTime < 7.5) {
		myCamera.move(gps::MOVE_BACKWARD, speed);
	}
	else if (elapsedTime < 9.25) {
		myCamera.move(gps::MOVE_LEFT, speed);
	}
	else {
		moveCameraEnabled = false;
		startTimeStamp = 0.0f;
	}
}

GLuint rainVBO, rainVAO, rainEBO;

struct RainDrop {
	glm::vec3 position;
	glm::vec3 endPosition;
	float speed;
	double startTime;
	float size;
};

int NUM_RAINDROPS = 15;
std::vector<RainDrop> activeRainDrops;

void initializeRainBuffers() {
	
	glGenVertexArrays(1, &rainVAO);
	glGenBuffers(1, &rainVBO);
	glGenBuffers(1, &rainEBO);

	glBindVertexArray(rainVAO);

	glBindBuffer(GL_ARRAY_BUFFER, rainVBO);
	glBufferData(GL_ARRAY_BUFFER, sizeof(glm::vec3) * NUM_RAINDROPS * 2, nullptr, GL_DYNAMIC_DRAW);

	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, rainEBO);
	std::vector<GLuint> indices(NUM_RAINDROPS * 2);
	for (size_t i = 0; i < NUM_RAINDROPS; ++i) {
		indices[i * 2] = i * 2;         // First vertex of line i
		indices[i * 2 + 1] = i * 2 + 1; // Second vertex of line i
	}
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices.size() * sizeof(GLuint), indices.data(), GL_STATIC_DRAW);

	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(glm::vec3), (void*)0);
	glEnableVertexAttribArray(0);

	glBindVertexArray(0);
	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
}


void generateRainDrops(int count) {
	for (int i = 0; i < count; ++i) {
		RainDrop newRainDrop;
		newRainDrop.position = glm::vec3(
			-1.0f + static_cast<float>(rand()) / (RAND_MAX / 2.0f), // Random X position
			1.0f,                                                   // Start high
			-1.0f + static_cast<float>(rand()) / (RAND_MAX / 2.0f)  // Random Z position
		);
		newRainDrop.speed = 0.5f + static_cast<float>(rand()) / (RAND_MAX / 2.0f); // Random speed
		newRainDrop.startTime = glfwGetTime();
		newRainDrop.size = 0.01f;

		newRainDrop.endPosition = newRainDrop.position;
		newRainDrop.endPosition.y -= 0.05f;

		activeRainDrops.push_back(newRainDrop);
	}
}

void drawRain() {
	double currentTime = glfwGetTime();
	std::vector<glm::vec3> rainDropVertices;

	for (size_t i = 0; i < activeRainDrops.size();) {
		RainDrop& rainDrop = activeRainDrops[i];

		double elapsedTime = currentTime - rainDrop.startTime;
		rainDrop.position.y -= rainDrop.speed * static_cast<float>(elapsedTime);
		rainDrop.endPosition.y -= rainDrop.speed * static_cast<float>(elapsedTime);

		if (rainDrop.position.y < -0.35f) {
			activeRainDrops.erase(activeRainDrops.begin() + i);
		}
		else {
			rainDropVertices.push_back(rainDrop.position);
			rainDropVertices.push_back(rainDrop.endPosition);
			++i;
		}
	}

	// Upload updated vertex data to the GPU
	glBindBuffer(GL_ARRAY_BUFFER, rainVBO);
	glBufferSubData(GL_ARRAY_BUFFER, 0, rainDropVertices.size() * sizeof(glm::vec3), rainDropVertices.data());
	glBindBuffer(GL_ARRAY_BUFFER, 0);

	// Render the raindrops
	rainShader.useShaderProgram();
	glBindVertexArray(rainVAO);

	glDrawElements(GL_LINES, rainDropVertices.size(), GL_UNSIGNED_INT, 0); // Draw lines using the indices

	glBindVertexArray(0);
}


void updateRain() {
	static double lastSpawnTime = 0.0;
	double currentTime = glfwGetTime();

	// Generate new raindrops every 0.1 seconds
	if (currentTime - lastSpawnTime > 0.1) {
		generateRainDrops(NUM_RAINDROPS - 5);
		lastSpawnTime = currentTime;
	}
}

void renderScene() {

	// depth maps creation pass
	//TODO - Send the light-space transformation matrix to the depth map creation shader and
	//		 render the scene in the depth map
	depthMapShader.useShaderProgram();
	glUniformMatrix4fv(glGetUniformLocation(depthMapShader.shaderProgram, "lightSpaceTrMatrix"), 1, GL_FALSE, glm::value_ptr(computeLightSpaceTrMatrix()));
	glViewport(0, 0, SHADOW_WIDTH, SHADOW_HEIGHT);
	glBindFramebuffer(GL_FRAMEBUFFER, shadowMapFBO);
	glClear(GL_DEPTH_BUFFER_BIT);
	drawObjects(depthMapShader, true);
	drawAnimation(depthMapShader, true);
	drawRandomObj(depthMapShader, true);
	glBindFramebuffer(GL_FRAMEBUFFER, 0);

	// render depth map on screen - toggled with the M key

	if (showDepthMap) {
		glViewport(0, 0, retina_width, retina_height);

		glClear(GL_COLOR_BUFFER_BIT);

		screenQuadShader.useShaderProgram();

		//bind the depth map
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, depthMapTexture);
		glUniform1i(glGetUniformLocation(screenQuadShader.shaderProgram, "depthMap"), 0);

		glDisable(GL_DEPTH_TEST);
		screenQuad.Draw(screenQuadShader);
		glEnable(GL_DEPTH_TEST);
	}
	else {

		// final scene rendering pass (with shadows)

		glViewport(0, 0, retina_width, retina_height);

		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		myCustomShader.useShaderProgram();

		view = myCamera.getViewMatrix();
		glUniformMatrix4fv(viewLoc, 1, GL_FALSE, glm::value_ptr(view));

		lightRotation = glm::rotate(glm::mat4(1.0f), glm::radians(lightAngle), glm::vec3(0.0f, 1.0f, 0.0f));
		glUniform3fv(lightDirLoc, 1, glm::value_ptr(glm::inverseTranspose(glm::mat3(view * lightRotation)) * lightDir));

		// Update point light position and avoid lights following the camera
		glUniform3fv(pointLightPosLoc, 1, glm::value_ptr(glm::mat3(view) * pointLightPos));
		glUniform3fv(pointLightPosLoc1, 1, glm::value_ptr(glm::mat3(view) * pointLightPos1));
		glUniform1i(glGetUniformLocation(myCustomShader.shaderProgram, "pointLightEnabled"), pointLightEnabled);
		glUniform1i(glGetUniformLocation(myCustomShader.shaderProgram, "spotLightEnabled"), spotLightEnabled);
		glUniform3fv(spotLightPosLoc, 1, glm::value_ptr(glm::mat3(view) * spotLightPos));
		glUniform3fv(spotLightDirLoc, 1, glm::value_ptr(glm::mat3(view) * spotLightDir));

		//bind the shadow map
		glActiveTexture(GL_TEXTURE3);
		glBindTexture(GL_TEXTURE_2D, depthMapTexture);
		glUniform1i(glGetUniformLocation(myCustomShader.shaderProgram, "shadowMap"), 3);

		glUniformMatrix4fv(glGetUniformLocation(myCustomShader.shaderProgram, "lightSpaceTrMatrix"),
			1,
			GL_FALSE,
			glm::value_ptr(computeLightSpaceTrMatrix()));

		drawObjects(myCustomShader, false);
		drawAnimation(myCustomShader, false);
		drawRandomObj(myCustomShader, false);

		lightShader.useShaderProgram();

		glUniformMatrix4fv(glGetUniformLocation(lightShader.shaderProgram, "view"), 1, GL_FALSE, glm::value_ptr(view));

	}

	if(rainEnabled){
		updateRain();
		drawRain();
	}
	animateCamera();
	mySkyBox.Draw(skyboxShader, view, projection);
}

void cleanup() {
	glDeleteBuffers(1, &rainVBO);
	glDeleteBuffers(1, &rainEBO);
	glDeleteVertexArrays(1, &rainVAO);

	glDeleteTextures(1,& depthMapTexture);
	glBindFramebuffer(GL_FRAMEBUFFER, 0);
	glDeleteFramebuffers(1, &shadowMapFBO);
	glfwDestroyWindow(glWindow);
	//close GL context and any other GLFW resources
	glfwTerminate();
}

void initSkybox() {
	std::vector<const GLchar*> faces;
	faces.push_back("FishPond/negx.jpg");
	faces.push_back("FishPond/posx.jpg");
	faces.push_back("FishPond/posy.jpg");
	faces.push_back("FishPond/negy.jpg");
	faces.push_back("FishPond/negz.jpg");
	faces.push_back("FishPond/posz.jpg");

	mySkyBox.Load(faces);
}


int main(int argc, const char* argv[]) {

	if (!initOpenGLWindow()) {
		glfwTerminate();
		return 1;
	}

	initOpenGLState();
	initObjects();
	initShaders();
	initUniforms();
	initializeRainBuffers();
	initSkybox();
	initFBO();

	glCheckError();

	while (!glfwWindowShouldClose(glWindow)) {
		processMovement();
		renderScene();

		glfwPollEvents();
		glfwSwapBuffers(glWindow);
	}

	cleanup();

	return 0;
}