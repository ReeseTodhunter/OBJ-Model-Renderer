#include <glad/glad.h>
#include <glfw/glfw3.h>
#include <fstream>
#include <iostream>

#include <glm/glm.hpp>
#include <glm/ext.hpp>

#include "Utilities.h"

static double s_prevTime = 0;
static float s_totalTime = 0;
static float s_deltaTime = 0;

void Utility::resetTimer()
{
	s_prevTime = glfwGetTime();
	s_totalTime = 0;
	s_deltaTime = 0;
}

float Utility::tickTimer()
{
	double currentTime = glfwGetTime();
	s_deltaTime = (float)(currentTime - s_prevTime);
	s_totalTime += s_deltaTime;
	s_prevTime = currentTime;
	return s_deltaTime;
}

float Utility::getDeltaTime()
{
	return s_deltaTime;
}

float Utility::getTotalTime()
{
	return s_totalTime;
}

char* Utility::fileToBuffer(const char* a_sPath)
{
	//get an fstream to read in the file data
	std::fstream file;
	file.open(a_sPath, std::ios_base::in | std::ios_base::binary);
	//Test to see if the file has opened correctly
	if (file.is_open())
	{
		//Success file has been opened, verify contents of file -- ie check that the file is not zero length
		file.ignore(std::numeric_limits<std::streamsize>::max());	//Attempt to read the highest number of bytes from the file
		std::streamsize fileSize = file.gcount();		//gCount will have reached EOF marker, letting us know the number of bytes
		file.clear();				//Clear the EOF marker from being read
		file.seekg(0, std::ios_base::beg);		//Seek back to the beginning of the file
		if (fileSize == 0) //If the file has no data close the file and return early
		{
			file.close();
			return nullptr;
		}
		//Create a char buffer large enough to hold the entire file
		char* dataBuffer = new char[fileSize + 1];
		memset(dataBuffer, 0, fileSize + 1); //Ensure the contents of the buffer are cleared
		//File  the buffer with the contents of the file
		file.read(dataBuffer, fileSize);
		//Close the file
		file.close();
		return dataBuffer;
	}
	return nullptr;
}

void Utility::freeMovement(glm::mat4& a_transform, float a_deltaTime, float a_speed,
	const glm::vec3& a_up /* = glm::vec3(0,1,0) */)
{
	//get the current window context
	GLFWwindow* window = glfwGetCurrentContext();

	//Get the camera's forward, right, up and location vectors
	glm::vec4 vForward = a_transform[2];
	glm::vec4 vRight = a_transform[0];
	glm::vec4 vUp = a_transform[1];
	glm::vec4 vTranslation = a_transform[3];
	//test to see if the left shift key is pressed
	//we will use left shift to double the speed of camera movement
	float frameSpeed = glfwGetKey(window, GLFW_KEY_LEFT_SHIFT) == GLFW_PRESS ? a_deltaTime * a_speed * 2 : a_deltaTime * a_speed;

	//Translate camera
	//W & S - Forward and Backward (Horizontal displacement)
	//A & D - Left and Right (Horizontal displacement)
	//Q & E - Up and Down (Vertical displacement)
	if (glfwGetKey(window, 'W') == GLFW_PRESS)
	{
		vTranslation -= vForward * frameSpeed;
	}
	if (glfwGetKey(window, 'S') == GLFW_PRESS)
	{
		vTranslation += vForward * frameSpeed;
	}
	if (glfwGetKey(window, 'A') == GLFW_PRESS)
	{
		vTranslation -= vRight * frameSpeed;
	}
	if (glfwGetKey(window, 'D') == GLFW_PRESS)
	{
		vTranslation += vRight * frameSpeed;
	}
	if (glfwGetKey(window, 'Q') == GLFW_PRESS)
	{
		vTranslation -= vUp * frameSpeed;
	}
	if (glfwGetKey(window, 'E') == GLFW_PRESS)
	{
		vTranslation += vUp * frameSpeed;
	}
	//Set the translation to the camera matrix that has been passed in
	a_transform[3] = vTranslation;

	//Check for camera rotation
	//test for mouse button being held/pressed for rotation (button 2)
	static bool sbMouseButtonDown = false;
	if (glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_2) == GLFW_PRESS)
	{
		static double siPrevMouseX = 0;
		static double siPrevMouseY = 0;

		if (sbMouseButtonDown == false)
		{
			sbMouseButtonDown = true;
			glfwGetCursorPos(window, &siPrevMouseX, &siPrevMouseY);
		}

		double mouseX = 0, mouseY = 0;
		glfwGetCursorPos(window, &mouseX, &mouseY);

		double iDeltaX = mouseX - siPrevMouseX;
		double iDeltaY = mouseY - siPrevMouseY;

		siPrevMouseX = mouseX;
		siPrevMouseY = mouseY;

		glm::mat4 mMat;

		//pitch
		if (iDeltaY != 0)
		{
			mMat = glm::axisAngleMatrix(vRight.xyz(), (float)-iDeltaY / 150.0f);
			vRight = mMat * vRight;
			vUp = mMat * vUp;
			vForward = mMat * vForward;
		}
		//yaw
		if (iDeltaY != 0)
		{
			mMat = glm::axisAngleMatrix(a_up, (float)-iDeltaX / 150.0f);
			vRight = mMat * vRight;
			vUp = mMat * vUp;
			vForward = mMat * vForward;
		}

		a_transform[0] = vRight;
		a_transform[1] = vUp;
		a_transform[2] = vForward;
	}
	else
	{
		sbMouseButtonDown = false;
	}
}