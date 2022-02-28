#include "Skybox.h"

#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/ext.hpp>

#include "Texture.h"
#include "ShaderUtil.h"

//Constructor and Destructor
Skybox::Skybox() : m_SkyboxVBO(0), m_SkyboxVAO(0), m_SkyboxShader(0)
{
    //Call to the CubeMap constructor to construct a new CubeMap as Skybox is constructed
    m_SkyboxTexture = new CubeMap();
}
Skybox::~Skybox()
{
    delete m_SkyboxTexture;
}

void Skybox::SetupSkybox()
{
    ShaderUtil* ShaderUtilInst = ShaderUtil::GetInstance();

    //Create the Skybox Shader Program
    unsigned int vertexShader = ShaderUtil::LoadShader("resource/shaders/SB_vertex.glsl", GL_VERTEX_SHADER);
    unsigned int fragmentShader = ShaderUtil::LoadShader("resource/shaders/SB_fragment.glsl", GL_FRAGMENT_SHADER);
    m_SkyboxShader = ShaderUtil::CreateProgram(vertexShader, fragmentShader);
    //Delete shaders once program is created
    glDeleteShader(vertexShader);
    glDeleteShader(fragmentShader);

    float skyboxVertices[] = {
        // positions
        -1.0f,  1.0f, -1.0f,
        -1.0f, -1.0f, -1.0f,
         1.0f, -1.0f, -1.0f,
         1.0f, -1.0f, -1.0f,
         1.0f,  1.0f, -1.0f,
        -1.0f,  1.0f, -1.0f,

        -1.0f, -1.0f,  1.0f,
        -1.0f, -1.0f, -1.0f,
        -1.0f,  1.0f, -1.0f,
        -1.0f,  1.0f, -1.0f,
        -1.0f,  1.0f,  1.0f,
        -1.0f, -1.0f,  1.0f,

         1.0f, -1.0f, -1.0f,
         1.0f, -1.0f,  1.0f,
         1.0f,  1.0f,  1.0f,
         1.0f,  1.0f,  1.0f,
         1.0f,  1.0f, -1.0f,
         1.0f, -1.0f, -1.0f,

        -1.0f, -1.0f,  1.0f,
        -1.0f,  1.0f,  1.0f,
         1.0f,  1.0f,  1.0f,
         1.0f,  1.0f,  1.0f,
         1.0f, -1.0f,  1.0f,
        -1.0f, -1.0f,  1.0f,

        -1.0f,  1.0f, -1.0f,
         1.0f,  1.0f, -1.0f,
         1.0f,  1.0f,  1.0f,
         1.0f,  1.0f,  1.0f,
        -1.0f,  1.0f,  1.0f,
        -1.0f,  1.0f, -1.0f,

        -1.0f, -1.0f, -1.0f,
        -1.0f, -1.0f,  1.0f,
         1.0f, -1.0f, -1.0f,
         1.0f, -1.0f, -1.0f,
        -1.0f, -1.0f,  1.0f,
         1.0f, -1.0f,  1.0f
    };

    glGenBuffers(1, &m_SkyboxVBO);
    glGenVertexArrays(1, &m_SkyboxVAO);
    glBindBuffer(GL_ARRAY_BUFFER, m_SkyboxVBO);
    glBindVertexArray(m_SkyboxVAO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(skyboxVertices), &skyboxVertices, GL_STATIC_DRAW);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
}

void Skybox::RenderSkybox(glm::mat4 viewMatrix, glm::mat4 projectionMatrix)
{
    glDepthMask(GL_FALSE);
    glUseProgram(m_SkyboxShader);

    int skyboxLocation = glGetUniformLocation(m_SkyboxShader, "CubeMap");
    glUniform1i(skyboxLocation, 0);

    glm::mat4 viewMat = glm::mat4(glm::mat3(viewMatrix)); //Remove translation from view matrix

    //Pass view and projection matrix to the skybox shader
    int projectionLocation = glGetUniformLocation(m_SkyboxShader, "ProjectionMatrix");
    glUniformMatrix4fv(projectionLocation, 1, GL_FALSE, glm::value_ptr(projectionMatrix));

    int viewLocation = glGetUniformLocation(m_SkyboxShader, "View");
    glUniformMatrix4fv(viewLocation, 1, GL_FALSE, glm::value_ptr(viewMat));

    glBindVertexArray(m_SkyboxVAO);
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_CUBE_MAP, m_SkyboxTexture->GetCubeMapTexture());
    glDrawArrays(GL_TRIANGLES, 0, 36);
    glBindVertexArray(0);

    //Return the depth function to default and stop using Skybox shaders
    glDepthMask(GL_TRUE); 
    glUseProgram(0);
}