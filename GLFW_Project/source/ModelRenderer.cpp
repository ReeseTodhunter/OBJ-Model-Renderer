#include "ModelRenderer.h"
#include "ShaderUtil.h"
#include "Dispatcher.h"
#include "Observer.h"
#include "Utilities.h"
#include "Skybox.h"

#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include "TextureManager.h"
#include "OBJ_Loader.h"
#include <iostream>

//Including imgui header
#include <imgui.h>
#include <imgui_impl_opengl3.h>
#include <imgui_impl_glfw.h>

ModelRenderer::ModelRenderer()
{
}

ModelRenderer::~ModelRenderer()
{
}

bool ModelRenderer::OnCreate()
{
	m_currentFile = "resource/models/OfficeChair.obj";
	m_previousFile = m_currentFile;
	m_scale = 1.f;
	m_renderSkybox = true;

	Dispatcher* dp = Dispatcher::GetInstance();
	if (dp)
	{
		dp->Subscribe(this, &ModelRenderer::OnWindowResize);
	}
	//Get an instance of the texture manager
	TextureManager::CreateInstance();

	m_backgroundColour = glm::vec3(0.41f, 0.7f, 0.71f);
	//Set the clear colour and enable depth testing and backface culling
	glClearColor(m_backgroundColour.x, m_backgroundColour.y, m_backgroundColour.z, 1.f);
	glEnable(GL_DEPTH_TEST);
	glEnable(GL_CULL_FACE);

	//Create shader program
	unsigned int vertexShader = ShaderUtil::LoadShader("resource/shaders/vertex.glsl", GL_VERTEX_SHADER);
	unsigned int fragmentShader = ShaderUtil::LoadShader("resource/shaders/fragment.glsl", GL_FRAGMENT_SHADER);
	m_uiProgram = ShaderUtil::CreateProgram(vertexShader, fragmentShader);

	//Create a grid of lines to be drawn during our update
	//Create a 10 x 10 square grid
	Line* lines = new Line[42];

	for (int i = 0, j = 0; i < 21; i++, j += 2)
	{
		lines[j].v0.position = glm::vec4(-10 + i, 0.f, 10.f, 1.f);
		lines[j].v0.colour = (i == 10) ? glm::vec4(1.f, 1.f, 1.f, 1.f) : glm::vec4(0.f, 0.f, 0.f, 1.f);
		lines[j].v1.position = glm::vec4(-10 + i, 0.f, -10.f, 1.f);
		lines[j].v1.colour = (i == 10) ? glm::vec4(1.f, 1.f, 1.f, 1.f) : glm::vec4(0.f, 0.f, 0.f, 1.f);

		lines[j + 1].v0.position = glm::vec4(10, 0.f, -10.f + i, 1.f);
		lines[j + 1].v0.colour = (i == 10) ? glm::vec4(1.f, 1.f, 1.f, 1.f) : glm::vec4(0.f, 0.f, 0.f, 1.f);
		lines[j + 1].v1.position = glm::vec4(-10, 0.f, -10.f + i, 1.f);
		lines[j + 1].v1.colour = (i == 10) ? glm::vec4(1.f, 1.f, 1.f, 1.f) : glm::vec4(0.f, 0.f, 0.f, 1.f);
	}
	//Create a vertex buffer to hold our line data
	glGenBuffers(1, &m_lineVBO);
	glBindBuffer(GL_ARRAY_BUFFER, m_lineVBO);

	//fill vertex buffer with line data
	glBufferData(GL_ARRAY_BUFFER, 42 * sizeof(Line), lines, GL_STATIC_DRAW);
	
	//As we have sent line data to the gpu we no longer require it on the CPU side memory
	delete[] lines;

	//enable the vertex array state, since we're sending in an array of vertices
	glEnableVertexAttribArray(0);
	glEnableVertexAttribArray(1);

	//specify where our vertex array is, how many components each vertex has,
	//the data type of each component and whether the data is normalised or not
	glVertexAttribPointer(0, 4, GL_FLOAT, GL_FALSE, sizeof(Vertex), 0);
	glVertexAttribPointer(1, 4, GL_FLOAT, GL_FALSE, sizeof(Vertex), ((char*)0) + 16);

	glBindBuffer(GL_ARRAY_BUFFER, 0);

	//Create a world-space matrix for a camera
	m_cameraMatrix = glm::inverse(glm::lookAt(glm::vec3(10, 10, 10),
											  glm::vec3(0, 0, 0),
											  glm::vec3(0, 1, 0)));

	//Create a perspective projection matrix with a 90 degree field-of-view and widescreen aspect ratio
	m_projectionMatrix = glm::perspective(glm::pi<float>() * 0.25f, m_windowWidth / (float)m_windowHeight, 0.1f, 1000.0f);

	//--------------------------------------------------------------------------------------------------------------------//
	//Setup Skybox
	m_skybox = new Skybox();
	m_skybox->SetupSkybox();

	m_objModel = new OBJModel();
	if (m_objModel->Load(m_currentFile , m_scale))
	{
		TextureManager* pTM = TextureManager::GetInstance();
		//Load in texture for model if any are present
		for (int i = 0; i < m_objModel->GetMaterialCount(); i++)
		{
			OBJMaterial* mat = m_objModel->GetMaterialByIndex(i);
			for (int n = 0; n < OBJMaterial::TextureTypes::TextureTypes_Count; n++)
			{
				if (mat->textureFileNames[n].size() > 0)
				{
					unsigned int textureID = pTM->LoadTexture(mat->textureFileNames[n].c_str());
					mat->textureIDs[n] = textureID;
				}
			}
		}

		//Create OBJ shader program
		unsigned int obj_vertexShader = ShaderUtil::LoadShader("resource/shaders/obj_vertex.glsl", GL_VERTEX_SHADER);
		unsigned int obj_fragmentShader = ShaderUtil::LoadShader("resource/shaders/obj_fragment.glsl", GL_FRAGMENT_SHADER);
		m_objProgram = ShaderUtil::CreateProgram(obj_vertexShader, obj_fragmentShader);
		//Setup vertex and index buffer for OBJ rendering
		glGenBuffers(2, m_objModelBuffer);
		//Setup vertex buffer date
		glBindBuffer(GL_ARRAY_BUFFER, m_objModelBuffer[0]);

		glBindBuffer(GL_ARRAY_BUFFER, 0);
	}
	else
	{
		std::cout << "Failed to load Model" << std::endl;
		return false;
	}

	return  true;
}

void ModelRenderer::Update(float deltaTime)
{
	Utility::freeMovement(m_cameraMatrix, deltaTime, 10.f);
	//Set yup an imgui window to control BG colour
	ImGuiIO& io = ImGui::GetIO();
	ImVec2 window_size = ImVec2(400.f, 300.f);
	ImVec2 window_pos = ImVec2(io.DisplaySize.x * 0.01f, io.DisplaySize.y * 0.05f);
	char pathBuffer[500] = { };
	if (ImGui::Begin("Model Render Options"))
	{
		static float scale = m_scale;
		static bool checked = m_renderSkybox;
		//Allows user to turn on an off the skybox
		ImGui::Checkbox("Render Skybox", &checked);
		//Allow to change background colouring
		ImGui::ColorEdit3("Background Colour: ", glm::value_ptr(m_backgroundColour));
		//Allow the user to input a obj model location
		ImGui::InputText("File Path: ", pathBuffer, IM_ARRAYSIZE(pathBuffer));
		//Allow the user to change the model scale
		ImGui::SliderFloat("Model Scale: ", &scale, 0.1f, 10.f);
		if (glfwGetKey(m_window, GLFW_KEY_ENTER) == GLFW_PRESS)
		{
			m_scale = scale;
			m_currentFile = pathBuffer;
			std::fill_n(pathBuffer, IM_ARRAYSIZE(pathBuffer), ' ');
			std::cout << m_currentFile << std::endl;
		}
		m_renderSkybox = checked;
	}
	ImGui::End();
}

void ModelRenderer::Draw()
{
	if (m_currentFile != m_objModel->GetFilename())
	{
		m_objModel->Unload();
		if (m_objModel->Load(m_currentFile, (m_scale)))
		{
			TextureManager* pTM = TextureManager::GetInstance();
			//Load in texture for model if any are present
			for (int i = 0; i < m_objModel->GetMaterialCount(); i++)
			{
				OBJMaterial* mat = m_objModel->GetMaterialByIndex(i);
				for (int n = 0; n < OBJMaterial::TextureTypes::TextureTypes_Count; n++)
				{
					if (mat->textureFileNames[n].size() > 0)
					{
						unsigned int textureID = pTM->LoadTexture(mat->textureFileNames[n].c_str());
						mat->textureIDs[n] = textureID;
					}
				}
			}

			//Create OBJ shader program
			unsigned int obj_vertexShader = ShaderUtil::LoadShader("resource/shaders/obj_vertex.glsl", GL_VERTEX_SHADER);
			unsigned int obj_fragmentShader = ShaderUtil::LoadShader("resource/shaders/obj_fragment.glsl", GL_FRAGMENT_SHADER);
			m_objProgram = ShaderUtil::CreateProgram(obj_vertexShader, obj_fragmentShader);
			//Setup vertex and index buffer for OBJ rendering
			glGenBuffers(2, m_objModelBuffer);
			//Setup vertex buffer date
			glBindBuffer(GL_ARRAY_BUFFER, m_objModelBuffer[0]);

			glBindBuffer(GL_ARRAY_BUFFER, 0);
			m_previousFile = m_currentFile;
		}
		else
		{
			std::cout << "Failed to load Model" << std::endl;
			m_objModel->Load(m_previousFile, m_scale);
			m_currentFile = m_previousFile;
		}
	}

	//Clear the backbuffer
	glClearColor(m_backgroundColour.x, m_backgroundColour.y, m_backgroundColour.z, 1.f);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	//Get the view matrix from the world-space camera matrix
	glm::mat4 viewMatrix = glm::inverse(m_cameraMatrix);
	glm::mat4 projectionViewMatrix = m_projectionMatrix * viewMatrix;

	//Render the skybox
	if (m_renderSkybox)
	{
		m_skybox->RenderSkybox(viewMatrix, m_projectionMatrix);
	}

	//Enable shaders
	glUseProgram(m_uiProgram);

	//Send the projection matrix to the vertex shader
	//Ask the shader program for the location of the projection-view-matrix uniform variable
	unsigned int projectionViewUniformLocation = glGetUniformLocation(m_uiProgram, "ProjectionViewMatrix");
	//Send this location a pointer to our glm::mat4 (Send across float data)
	glUniformMatrix4fv(projectionViewUniformLocation , 1, false, glm::value_ptr(projectionViewMatrix));

	glBindBuffer(GL_ARRAY_BUFFER, m_lineVBO);
	glBufferData(GL_ARRAY_BUFFER, 42 * sizeof(Line), lines, GL_STATIC_DRAW);
	//Enable vertex array, as we're sending an array of vertices
	glEnableVertexAttribArray(0);
	glEnableVertexAttribArray(1);

	//Specify where our vertex array is, how many components each vertex has,
	//the data type of each component and wether the data is normalised or not
	glVertexAttribPointer(0, 4, GL_FLOAT, GL_FALSE, sizeof(Vertex), 0);
	glVertexAttribPointer(1, 4, GL_FLOAT, GL_FALSE, sizeof(Vertex), ((char*)0) + 16);

	glDrawArrays(GL_LINES, 0, 42 * 2);

	glBindBuffer(GL_ARRAY_BUFFER, 0);

	glUseProgram(0);



	glUseProgram(m_objProgram);
	//Set the projection view matrix for this shader
	projectionViewUniformLocation = glGetUniformLocation(m_objProgram, "ProjectionViewMatrix");
	//Send this location a pointer to the glm::mat4 (send across float data)
	glUniformMatrix4fv(projectionViewUniformLocation, 1, false, glm::value_ptr(projectionViewMatrix));
	for (int i = 0; i < m_objModel->GetMeshCount(); ++i)
	{
		//Get the Model Matrix Location from the shader program
		int modelMatrixUniformLocation = glGetUniformLocation(m_objProgram, "ModelMatrix");
		//Send the OBJ Model's world matrix data across to the shader program
		glUniformMatrix4fv(modelMatrixUniformLocation, 1, false, glm::value_ptr(m_objModel->GetWorldMatrix()));

		int cameraPositionUniformLocation = glGetUniformLocation(m_objProgram, "camPos");
		glUniform4fv(cameraPositionUniformLocation, 1, glm::value_ptr(m_cameraMatrix[3]));

		OBJMesh* pMesh = m_objModel->GetMeshByIndex(i);
		//send material data to shader
		int kA_location = glGetUniformLocation(m_objProgram, "kA");
		int kD_location = glGetUniformLocation(m_objProgram, "kD");
		int kS_location = glGetUniformLocation(m_objProgram, "kS");

		OBJMaterial* pMaterial = pMesh->m_material;
		if (pMaterial != nullptr)
		{
			//Send the OBJ Model's world matrix data across to the shader program
			glUniform4fv(kA_location, 1, glm::value_ptr(pMaterial->kA));
			glUniform4fv(kD_location, 1, glm::value_ptr(pMaterial->kD));
			glUniform4fv(kS_location, 1, glm::value_ptr(pMaterial->kS));

			//Get the location of the diffuse texture
			int texUniformLoc = glGetUniformLocation(m_objProgram, "DiffuseTexture");
			glUniform1i(texUniformLoc, 0); //Set diffuse texture to be GL_Texture0

			glActiveTexture(GL_TEXTURE0); //Set the active texture unit to texture0
			//bind the texture for diffuse for this material to the texture0
			glBindTexture(GL_TEXTURE_2D, pMaterial->textureIDs[OBJMaterial::TextureTypes::DiffuseTexture]);

			//Get the location of the specular texture
			texUniformLoc = glGetUniformLocation(m_objProgram, "SpecularTexture");
			glUniform1i(texUniformLoc, 1); //Set diffuse texture to be GL_Texture1

			glActiveTexture(GL_TEXTURE1); //Set the active texture unit to texture1
			//bind the texture for diffuse for this material to the texture0
			glBindTexture(GL_TEXTURE_2D, pMaterial->textureIDs[OBJMaterial::TextureTypes::SpecularTexture]);

			//Get the location of the normal texture
			texUniformLoc = glGetUniformLocation(m_objProgram, "NormalTexture");
			glUniform1i(texUniformLoc, 2); //Set diffuse texture to be GL_Texture2

			glActiveTexture(GL_TEXTURE2); //Set the active texture unit to texture2
			//bind the texture for diffuse for this material to the texture2
			glBindTexture(GL_TEXTURE_2D, pMaterial->textureIDs[OBJMaterial::TextureTypes::NormalTexture]);
		}
		else //No material to obtain lighting information from use defaults
		{
			//Send the OBJ Model's world matrix data across to the shader program
			glUniform4fv(kA_location, 1, glm::value_ptr(glm::vec4(0.25f, 0.25f, 0.25f, 1.f)));
			glUniform4fv(kD_location, 1, glm::value_ptr(glm::vec4(1.f, 1.f, 1.f, 1.f)));
			glUniform4fv(kS_location, 1, glm::value_ptr(glm::vec4(1.f, 1.f, 1.f, 64.f)));
		}
		glBindBuffer(GL_ARRAY_BUFFER, m_objModelBuffer[0]);
		glBufferData(GL_ARRAY_BUFFER, pMesh->m_vertices.size() * sizeof(OBJVertex), pMesh->m_vertices.data(), GL_STATIC_DRAW);

		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_objModelBuffer[1]);
		glEnableVertexAttribArray(0);	//position
		glEnableVertexAttribArray(1);	//normal
		glEnableVertexAttribArray(2);	//uv coord

		glVertexAttribPointer(0, 4, GL_FLOAT, GL_FALSE, sizeof(OBJVertex), ((char*)0) + OBJVertex::PositionOffset);
		glVertexAttribPointer(1, 4, GL_FLOAT, GL_TRUE, sizeof(OBJVertex), ((char*)0) + OBJVertex::NormalOffset);
		glVertexAttribPointer(2, 2, GL_FLOAT, GL_TRUE, sizeof(OBJVertex), ((char*)0) + OBJVertex::UVCoordOffset);

		glBufferData(GL_ELEMENT_ARRAY_BUFFER, pMesh->m_indices.size() * sizeof(unsigned int), pMesh->m_indices.data(), GL_STATIC_DRAW);
		glDrawElements(GL_TRIANGLES, pMesh->m_indices.size(), GL_UNSIGNED_INT, 0);
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
		glBindBuffer(GL_ARRAY_BUFFER, 0);
	}

	glDisableVertexAttribArray(0);
	glDisableVertexAttribArray(1);
	glDisableVertexAttribArray(2);

	glUseProgram(0);
}

void ModelRenderer::Destroy()
{
	delete m_objModel;
	delete[] lines;
	glDeleteBuffers(1, &m_lineVBO);
	ShaderUtil::DeleteProgram(m_uiProgram);
	ShaderUtil::DeleteProgram(m_objProgram);
	TextureManager::DestroyInstance();
	ShaderUtil::DestroyInstance();
}

void ModelRenderer::OnWindowResize(WindowResizeEvent* e)
{
	std::cout << "Member event handler called" << std::endl;
	if (e->GetWidth() > 0 && e->GetHeight() > 0)
	{
		m_projectionMatrix = glm::perspective(glm::pi<float>() * 0.25f, m_windowWidth / (float)m_windowHeight, 0.1f, 1000.0f);
		//Create a perspective projection matrix with a 90 degree field-of-view and widescreen aspect ratio
		m_projectionMatrix = glm::perspective(glm::pi<float>() * 0.25f, e->GetWidth() / (float)e->GetHeight(), 0.1f, 1000.0f);
		glViewport(0, 0, e->GetWidth(), e->GetHeight());
	}
	e->Handled();
}