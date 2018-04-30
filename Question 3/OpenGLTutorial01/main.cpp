#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <SOIL2/SOIL2.h>

#include <iostream>
#include <string>
#include <vector>
#include <map>
#include <stdlib.h>     /* srand, rand */
#include <time.h>       /* time */

#include "cShaderProgram.h"
#include "cCamera.h"
#include "cMesh.h"
#include "cModel.h"
#include "cSkybox.h"
#include "cSkinnedMesh.h"
#include "cSkinnedGameObject.h"
#include "cAnimationState.h"
#include "cScreenQuad.h"
#include "cPlaneObject.h"
#include "cFrameBuffer.h"
#include "cCubeObject.h"
#include "cMazeMaker.h"
#include "cTexture.h"
#include "cTardis.h"
#include "cRandThreaded.h"

//Setting up a camera GLOBAL
cCamera Camera(glm::vec3(20.0f, 35.0f, 40.0f),		//Camera Position
			glm::vec3(0.0f, 1.0f, 0.0f),		//World Up vector
			-75.0f,								//Pitch
			-90.0f);							//Yaw

float cameraRotAngle = 0.0f;
glm::vec3 forwardCamera;
float deltaTime = 0.0f;
float lastFrame = 0.0f;
bool firstMouse = true;
float lastX = 400, lastY = 300;

unsigned int SCR_WIDTH = 800;
unsigned int SCR_HEIGHT = 600;

int drawType = 1;

std::map<std::string, cModel*> mapModelsToNames;
std::map<std::string, cSkinnedMesh*> mapSkinnedMeshToNames;
std::map<std::string, cShaderProgram*> mapShaderToName;

void framebuffer_size_callback(GLFWwindow* window, int width, int height);
void processInput(GLFWwindow* window);
void mouse_callback(GLFWwindow* window, double xpos, double ypos);
void scroll_callback(GLFWwindow* window, double xoffset, double yoffset);
unsigned int loadCubeMap(std::string directory, std::vector<std::string> faces);

int main()
{
	glfwInit();

	srand(time(NULL));

	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

	GLFWwindow* window = glfwCreateWindow(SCR_WIDTH, SCR_HEIGHT, "LearnOpenGL", NULL, NULL);
	if (window == NULL)
	{
		std::cout << "Failed to create GLFW window" << std::endl;
		glfwTerminate();
		return -1;
	}

	glfwMakeContextCurrent(window);

	if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
	{
		std::cout << "Failed to initialzed GLAD" << std::endl;
		return -1;
	}

	glViewport(0, 0, SCR_WIDTH, SCR_HEIGHT);

	//Setting up global openGL state
	glEnable(GL_DEPTH_TEST);
	glDepthFunc(GL_LESS);

	glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);
	glfwSetCursorPosCallback(window, mouse_callback);
	glfwSetScrollCallback(window, scroll_callback);

	//Set up all our programs
	cShaderProgram* myProgram = new cShaderProgram();
	myProgram->compileProgram("assets/shaders/", "vertShader.glsl", "fragShader.glsl");
	mapShaderToName["mainProgram"] = myProgram;

	myProgram = new cShaderProgram();
	myProgram->compileProgram("assets/shaders/", "animVert.glsl", "animFrag.glsl");
	mapShaderToName["skinProgram"] = myProgram;

	myProgram = new cShaderProgram();
	myProgram->compileProgram("assets/shaders/", "skyBoxVert.glsl", "skyBoxFrag.glsl");
	mapShaderToName["skyboxProgram"] = myProgram;

	myProgram = new cShaderProgram();
	myProgram->compileProgram("assets/shaders/", "modelVert.glsl", "modelFrag.glsl");
	mapShaderToName["simpleProgram"] = myProgram;

	myProgram = new cShaderProgram();
	myProgram->compileProgram("assets/shaders/", "quadVert.glsl", "quadFrag.glsl");
	mapShaderToName["quadProgram"] = myProgram;

	//Assemble all our models (These models don't exist anymore...)
	std::string path = "assets/models/tardis/tardis.obj";
	mapModelsToNames["Tardis"] = new cModel(path);

	cTexture wallTexture("assets/textures/wall.jpg");
	cTexture tardisTexture("assets/textures/wood-texture-pilm-l.jpg");

	//Using Maze Maker
	const int dimension = 301;
	cMazeMaker* theMM = new cMazeMaker();
	//theMM.GenerateMaze(20, 20);
	//Mazes need to have odd numbered dimensions
	theMM->GenerateMaze(dimension, dimension);

	int startX, startZ, endX, endZ;
	startX = startZ = endX = endZ = -1;

	//Basically, whichever one I find first is the start, and then the next one becomes the exit
	//Top row check
	for (int i = 0; i < dimension; i++)
	{
		if (theMM->maze[0][i][0] == false)
		{
			if (startX == -1)
			{
				startZ = 0;
				startX = i;
			}
			else if (endX == -1)
			{
				endZ = 0;
				endX = i;
				break;
			}
		}
	}

	//Left side check
	if (endX == -1)
	{
		for (int i = 0; i < dimension; i++)
		{
			if (theMM->maze[i][0][0] == false)
			{
				if (startX == -1)
				{
					startZ = i;
					startX = 0;
				}
				else if (endX == -1)
				{
					endZ = i;
					endX = 0;
					break;
				}
			}
		}
	}

	//Right side check
	if (endX == -1)
	{
		for (int i = 0; i < dimension; i++)
		{
			if (theMM->maze[i][dimension - 1][0] == false)
			{
				if (startX == -1)
				{
					startZ = i;
					startX = dimension - 1;
				}
				else if (endX == -1)
				{
					endZ = i;
					endX = dimension - 1;
					break;
				}
			}
		}
	}

	//Bottom check
	if (endX == -1)
	{
		for (int i = 0; i < dimension; i++)
		{
			if (theMM->maze[dimension - 1][i][0] == false)
			{
				if (startX == -1)
				{
					startZ = dimension - 1;
					startX = i;
				}
				else if (endX == -1)
				{
					endZ = dimension - 1;
					endX = i;
					break;
				}
			}
		}
	}

	//Make our Tardis object, now that we've found the start and exit
	cRandThreaded* g_pThreadedRandom = NULL;
	cTardis* theTardis = new cTardis(glm::vec3((float)startX + 0.5f, 0.0f, (float)startZ + 0.5f),
							glm::vec3((float)endX + 0.5f, 0.0f, (float)endZ + 0.5f),
							theMM, 
							g_pThreadedRandom);

	Camera.position.x = (float)startX;
	Camera.position.z = (float)startZ;

	//Making 3 frame buffers, one for each camera type, and one to draw at the end
	cFrameBuffer mainFrameBuffer(SCR_HEIGHT, SCR_WIDTH);

	//Some simple shapes
	cScreenQuad screenQuad;
	cPlaneObject planeObject;
	cCubeObject cubeObject;

	//Positions for some of the point light
	glm::vec3 pointLightPositions[] = {
		glm::vec3(0.7f,  0.2f,  2.0f),
		glm::vec3(2.3f, -3.3f, -4.0f),
		glm::vec3(-4.0f,  2.0f, -12.0f),
		glm::vec3(0.0f,  0.0f, -8.0f)
	};

	//Load the skyboxes
	cSkybox daybox("assets/textures/skybox/");
	cSkybox skybox("assets/textures/spacebox/");

	glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);

	mapShaderToName["skyboxProgram"]->useProgram();
	mapShaderToName["skyboxProgram"]->setInt("skybox", 0);

	mapShaderToName["quadProgram"]->useProgram();
	mapShaderToName["quadProgram"]->setInt("screenTexture", 0);

	mapShaderToName["simpleProgram"]->useProgram();
	mapShaderToName["simpleProgram"]->setInt("texture_diffuse1", 0);
	mapShaderToName["simpleProgram"]->setInt("texture_static", 1);

	mapShaderToName["mainProgram"]->useProgram();
	mapShaderToName["mainProgram"]->setInt("skybox", 0);
	mapShaderToName["mainProgram"]->setInt("reflectRefract", 0);
	//Light settings go in here until I've made a class for them
	{
		//http://devernay.free.fr/cours/opengl/materials.html
		mapShaderToName["mainProgram"]->setVec3("dirLight.direction", -0.2f, -1.0f, -0.3f);
		mapShaderToName["mainProgram"]->setVec3("dirLight.ambient", 0.15f, 0.15f, 0.15f);
		mapShaderToName["mainProgram"]->setVec3("dirLight.diffuse", 0.4f, 0.4f, 0.4f);
		mapShaderToName["mainProgram"]->setVec3("dirLight.specular", 0.5f, 0.5f, 0.5f);

		mapShaderToName["mainProgram"]->setVec3("pointLights[0].position", pointLightPositions[0]);
		mapShaderToName["mainProgram"]->setFloat("pointLights[0].constant", 1.0f);
		mapShaderToName["mainProgram"]->setFloat("pointLights[0].linear", 0.09f);
		mapShaderToName["mainProgram"]->setFloat("pointLights[0].quadratic", 0.032f);
		mapShaderToName["mainProgram"]->setVec3("pointLights[0].ambient", 0.05f, 0.05f, 0.05f);
		mapShaderToName["mainProgram"]->setVec3("pointLights[0].diffuse", 0.8f, 0.8f, 0.8f);
		mapShaderToName["mainProgram"]->setVec3("pointLights[0].specular", 1.0f, 1.0f, 1.0f);

		mapShaderToName["mainProgram"]->setVec3("pointLights[1].position", pointLightPositions[1]);
		mapShaderToName["mainProgram"]->setFloat("pointLights[1].constant", 1.0f);
		mapShaderToName["mainProgram"]->setFloat("pointLights[1].linear", 0.09f);
		mapShaderToName["mainProgram"]->setFloat("pointLights[1].quadratic", 0.032f);
		mapShaderToName["mainProgram"]->setVec3("pointLights[1].ambient", 0.05f, 0.05f, 0.05f);
		mapShaderToName["mainProgram"]->setVec3("pointLights[1].diffuse", 0.8f, 0.8f, 0.8f);
		mapShaderToName["mainProgram"]->setVec3("pointLights[1].specular", 1.0f, 1.0f, 1.0f);

		mapShaderToName["mainProgram"]->setVec3("pointLights[2].position", pointLightPositions[2]);
		mapShaderToName["mainProgram"]->setFloat("pointLights[2].constant", 1.0f);
		mapShaderToName["mainProgram"]->setFloat("pointLights[2].linear", 0.09f);
		mapShaderToName["mainProgram"]->setFloat("pointLights[2].quadratic", 0.032f);
		mapShaderToName["mainProgram"]->setVec3("pointLights[2].ambient", 0.05f, 0.05f, 0.05f);
		mapShaderToName["mainProgram"]->setVec3("pointLights[2].diffuse", 0.8f, 0.8f, 0.8f);
		mapShaderToName["mainProgram"]->setVec3("pointLights[2].specular", 1.0f, 1.0f, 1.0f);

		mapShaderToName["mainProgram"]->setVec3("pointLights[3].position", pointLightPositions[3]);
		mapShaderToName["mainProgram"]->setFloat("pointLights[3].constant", 1.0f);
		mapShaderToName["mainProgram"]->setFloat("pointLights[3].linear", 0.09f);
		mapShaderToName["mainProgram"]->setFloat("pointLights[3].quadratic", 0.032f);
		mapShaderToName["mainProgram"]->setVec3("pointLights[3].ambient", 0.05f, 0.05f, 0.05f);
		mapShaderToName["mainProgram"]->setVec3("pointLights[3].diffuse", 0.8f, 0.8f, 0.8f);
		mapShaderToName["mainProgram"]->setVec3("pointLights[3].specular", 1.0f, 1.0f, 1.0f);

		mapShaderToName["mainProgram"]->setVec3("spotLight.position", glm::vec3(0.0f, 6.0f, 0.0f));
		mapShaderToName["mainProgram"]->setVec3("spotLight.direction", glm::vec3(0.0f, -1.0f, 0.0f));
		mapShaderToName["mainProgram"]->setFloat("spotLight.cutOff", glm::cos(glm::radians(12.5f)));
		mapShaderToName["mainProgram"]->setFloat("spotLight.outerCutOff", glm::cos(glm::radians(15.0f)));
		mapShaderToName["mainProgram"]->setFloat("spotLight.constant", 1.0f);
		mapShaderToName["mainProgram"]->setFloat("spotLight.linear", 0.09f);
		mapShaderToName["mainProgram"]->setFloat("spotLight.quadratic", 0.032f);
		mapShaderToName["mainProgram"]->setVec3("spotLight.ambient", 0.0f, 0.0f, 0.0f);
		mapShaderToName["mainProgram"]->setVec3("spotLight.diffuse", 1.0f, 1.0f, 1.0f);
		mapShaderToName["mainProgram"]->setVec3("spotLight.specular", 1.0f, 1.0f, 1.0f);
		//http://wiki.ogre3d.org/tiki-index.php?page=-Point+Light+Attenuation
	}

	theTardis->beginThread();

	while (!glfwWindowShouldClose(window))
	{
		processInput(window);

		float currentFrame = glfwGetTime();
		deltaTime = currentFrame - lastFrame;
		lastFrame = currentFrame;

		//Begin writing to the main frame buffer
		glBindFramebuffer(GL_FRAMEBUFFER, mainFrameBuffer.FBO);

		glEnable(GL_DEPTH_TEST);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		//Have the camera move toward the doctor
		glm::vec3 docPos(0.0f);
		glm::vec3 levelCamera = glm::vec3(Camera.position.x, 0.0f, Camera.position.z);
		float backUpX = Camera.position.x;
		float backUpZ = Camera.position.z;
		while (theTardis->isDataLocked())
		{

		}
		theTardis->getTardisPosition(docPos);
		glm::vec3 distVec = glm::normalize(docPos - levelCamera);
		Camera.position.x += distVec.x * 5.0f * deltaTime;
		Camera.position.z += distVec.z * 5.0f * deltaTime;

		//This is how I was preserving the camera during those times I couldn't access the tardis' position
		//A much smarter idea is to initialize it to a 0 vector. But this will be a memento
		if (Camera.position.x != Camera.position.x)
		{
			Camera.position.x = backUpX;
			Camera.position.z = backUpZ;
		}

		glm::mat4 projection = glm::perspective(glm::radians(Camera.zoom), (float)SCR_WIDTH / (float)SCR_HEIGHT, 0.1f, 350.0f);
		glm::mat4 view = Camera.getViewMatrix();
		glm::mat4 skyboxView = glm::mat4(glm::mat3(Camera.getViewMatrix()));

		mapShaderToName["simpleProgram"]->useProgram();
		glm::mat4 model = glm::mat4(1.0f);
		//model = glm::translate(model, glm::vec3(0.5f, 0.0f, 0.0f));
		model = glm::rotate(model, glm::radians(90.0f), glm::vec3(1.0f, 0.0f, 0.0f));
		model = glm::scale(model, glm::vec3((float)dimension));
		mapShaderToName["simpleProgram"]->setMat4("projection", projection);
		mapShaderToName["simpleProgram"]->setMat4("view", view);
		mapShaderToName["simpleProgram"]->setMat4("model", model);
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, wallTexture.ID);
		glBindVertexArray(planeObject.VAO);
		glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);

		//Rendering our maze, one cube at a time
		//QUESTION 2
		//So we want to draw 20 cubes in all cardinal directions, plus all the cubes in between
		float viewRadius = sqrt(pow(20, 2) + pow(20, 2));
		//Camera is really high up, so we put it down on the ground while we do this calculation
		//Also Z is 20 units further than the circle's centre so we can look down from an angle. Adjust that too
		glm::vec3 adjustedCamera = glm::vec3(Camera.position.x, 0.0f, Camera.position.z);

		glBindVertexArray(cubeObject.VAO);
		for (int i = 0; i < theMM->maze.size(); i++)
		{
			for (int j = 0; j < theMM->maze[i].size(); j++)
			{
				if (theMM->maze[i][j][0] == true)
				{	
					glm::vec3 cubePosition = glm::vec3(1.0f * j, 0.0f, 1.0f * i);
					if (glm::distance(cubePosition, adjustedCamera) <= viewRadius)
					{
						model = glm::mat4(1.0f);
						model = glm::translate(model, cubePosition);
						mapShaderToName["simpleProgram"]->setMat4("model", model);
						glDrawArrays(GL_TRIANGLES, 0, 36);
						glDepthFunc(GL_LESS);
					}
				}
			}
		}
		model = glm::mat4(1.0f);
		model = glm::translate(model, docPos);
		model = glm::scale(model, glm::vec3(0.002f));
		mapShaderToName["simpleProgram"]->setMat4("model", model);
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, tardisTexture.ID);
		mapModelsToNames["Tardis"]->Draw(*mapShaderToName["simpleProgram"]);

		mapShaderToName["skyboxProgram"]->useProgram();

		glDepthFunc(GL_LEQUAL);  // change depth function so depth test passes when values are equal to depth buffer's content
		mapShaderToName["skyboxProgram"]->setMat4("projection", projection);
		mapShaderToName["skyboxProgram"]->setMat4("view", skyboxView);

		glBindVertexArray(skybox.VAO);
		glBindTexture(GL_TEXTURE_CUBE_MAP, daybox.textureID);
		glDrawArrays(GL_TRIANGLES, 0, 36);
		glDepthFunc(GL_LESS);

		//Final pass: Render all of the above on one quad
		glBindFramebuffer(GL_FRAMEBUFFER, 0);
		glDisable(GL_DEPTH_TEST);
		glClearColor(1.0f, 1.0f, 1.0f, 1.0f);
		glClear(GL_COLOR_BUFFER_BIT);

		//Paste the entire scene onto a quad as a single texture
		mapShaderToName["quadProgram"]->useProgram();
		mapShaderToName["quadProgram"]->setInt("drawType", drawType);
		glBindVertexArray(screenQuad.VAO);
		glBindTexture(GL_TEXTURE_2D, mainFrameBuffer.textureID);
		glDrawArrays(GL_TRIANGLES, 0, 6);

		glfwSwapBuffers(window);
		glfwPollEvents();
	}

	glfwTerminate();

	return 0;
}

void framebuffer_size_callback(GLFWwindow* window, int width, int height)
{
	SCR_HEIGHT = height;
	SCR_WIDTH = width;
	glViewport(0, 0, SCR_WIDTH, SCR_HEIGHT);
	return;
}

void processInput(GLFWwindow* window)
{
	if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
		glfwSetWindowShouldClose(window, true);

	if (glfwGetKey(window, GLFW_KEY_1) == GLFW_PRESS)
		drawType = 1;
	if (glfwGetKey(window, GLFW_KEY_2) == GLFW_PRESS)
		drawType = 2;
	if (glfwGetKey(window, GLFW_KEY_3) == GLFW_PRESS)
		drawType = 3;
	if (glfwGetKey(window, GLFW_KEY_4) == GLFW_PRESS)
		drawType = 4;
	if (glfwGetKey(window, GLFW_KEY_5) == GLFW_PRESS)
		drawType = 5;

	//if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS)
	//	Camera.processKeyboard(Camera_Movement::FORWARD, deltaTime);
	//if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS)
	//	Camera.processKeyboard(Camera_Movement::BACKWARD, deltaTime);
	//if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS)
	//	Camera.processKeyboard(Camera_Movement::LEFT, deltaTime);
	//if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)
	//	Camera.processKeyboard(Camera_Movement::RIGHT, deltaTime);

	//if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS)
	//{
	//	Camera.position.z -= 10.0f * deltaTime;
	//	if (Camera.position.z <= 40.0f)
	//	{
	//		Camera.position.z = 40.0f;
	//	}
	//}
	//if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS)
	//{
	//	Camera.position.z += 10.0f * deltaTime;
	//	if (Camera.position.z >= 300.0f)
	//	{
	//		Camera.position.z = 300.0f;
	//	}
	//}
	//if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS)
	//{
	//	Camera.position.x -= 10.0f * deltaTime;
	//	if (Camera.position.x <= 20.0f)
	//	{
	//		Camera.position.x = 20.0f;
	//	}
	//}
	//if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)
	//{
	//	Camera.position.x += 10.0f * deltaTime;
	//	if (Camera.position.x >= 280.0f)
	//	{
	//		Camera.position.x = 280.0f;
	//	}
	//}
}

void mouse_callback(GLFWwindow* window, double xpos, double ypos)
{
	if (firstMouse) // this bool variable is initially set to true
	{
		lastX = xpos;
		lastY = ypos;
		firstMouse = false;
	}

	float xOffset = xpos - lastX;
	float yOffset = lastY - ypos;
	lastX = xpos;
	lastY = ypos;

	Camera.processMouseMovement(xOffset, yOffset, true);
}

void scroll_callback(GLFWwindow* window, double xoffset, double yoffset)
{
	Camera.processMouseScroll(yoffset);
}

unsigned int loadCubeMap(std::string directory, std::vector<std::string> faces)
{
	unsigned int textureID;
	glGenTextures(1, &textureID);
	glBindTexture(GL_TEXTURE_CUBE_MAP, textureID);

	int width, height, nrChannels;
	for (unsigned int i = 0; i < faces.size(); i++)
	{
		std::string fullPath = directory + faces[i];
		unsigned char *data = SOIL_load_image(fullPath.c_str(), &width, &height, &nrChannels, 0);
		if (data)
		{
			GLenum format;
			if (nrChannels == 1)
				format = GL_RED;
			else if (nrChannels == 3)
				format = GL_RGB;
			else if (nrChannels == 4)
				format = GL_RGBA;

			glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i,
				0, GL_RGB, width, height, 0, format, GL_UNSIGNED_BYTE, data
			);
			SOIL_free_image_data(data);
		}
		else
		{
			std::cout << "Cubemap texture failed to load at path: " << faces[i] << std::endl;
			SOIL_free_image_data(data);
		}
	}
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);

	return textureID;
}