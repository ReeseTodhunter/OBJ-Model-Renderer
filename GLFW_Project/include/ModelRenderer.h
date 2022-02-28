#pragma once
#include "Application.h"
#include "ApplicationEvent.h"
#include <glm/glm.hpp>
#include <glm/ext.hpp>
#include <imgui.h>
#include <string>

//Forward declare OBJ model and SkyBox
class OBJModel;
class Skybox;

class ModelRenderer : public Application
{
public:
	ModelRenderer();
	virtual ~ModelRenderer();

	void OnWindowResize(WindowResizeEvent* e);
protected:
	virtual bool OnCreate();
	virtual void Update(float deltaTime);
	virtual void Draw();
	virtual void Destroy();

private:
	//Structure for a simple vertex - interleaved (position, colour)
	typedef struct Vertex
	{
		glm::vec4 position;
		glm::vec4 colour;
	}Vertex;

	//Structure for a line
	typedef struct Line
	{
		Vertex v0;
		Vertex v1;
	}Line;

	glm::mat4 m_cameraMatrix;
	glm::mat4 m_projectionMatrix;

	//Shader programs
	unsigned int m_uiProgram;
	unsigned int m_objProgram;
	unsigned int m_lineVBO;
	unsigned int m_objModelBuffer[2];
	bool m_renderSkybox;

	//Model variables
	std::string m_currentFile;
	std::string m_previousFile;
	float m_scale;

	//Model
	OBJModel* m_objModel;
	Line* lines;

	//Skybox
	Skybox* m_skybox;

	//Panel Stuff
	glm::vec3 m_backgroundColour;
};