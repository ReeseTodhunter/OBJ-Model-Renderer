#pragma once
#include <string>
#include <vector>
#include <glm/fwd.hpp>

//Forward declaring the cubemap
class CubeMap;

class Skybox
{
public:
	//constructor + Destructor
	Skybox();
	~Skybox();

	//Functions
	void SetupSkybox();
	void RenderSkybox(glm::mat4 viewMatrix, glm::mat4 projectionMatrix);

private:
	//Skybox variables
	unsigned int m_SkyboxVBO;
	unsigned int m_SkyboxVAO;
	unsigned int m_SkyboxShader;

	//Cubemap variables
	CubeMap* m_SkyboxTexture;
};