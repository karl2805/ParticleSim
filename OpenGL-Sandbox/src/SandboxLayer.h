#pragma once

#define GRID_SIZE 100




#include <GLCore.h>
#include <GLCoreUtils.h>
#include "GLCore/Util/Texture.h"
#include "GLCore/Util/Shader.h"

class SandboxLayer : public GLCore::Layer
{
public:
	SandboxLayer();
	virtual ~SandboxLayer();

	virtual void OnAttach() override;
	virtual void OnDetach() override;
	virtual void OnEvent(GLCore::Event& event) override;
	virtual void OnUpdate(GLCore::Timestep ts) override;
	virtual void OnRender() override;
	virtual void OnImGuiRender() override;
private:
	Texture m_texture;
	GLuint m_VBO, m_VAO, m_EBO, m_SSBO_in, m_SSBO_out;
	GLCore::Utils::Shader* m_shader;


	float vertices[3 * 4] = {
		// positions       
		 1.0f,  1.0f, 0.0f,
		 1.0f, -1.0f, 0.0f,
		-1.0f, -1.0f, 0.0f,
		-1.0f,  1.0f, 0.0f
	};

	unsigned int indices[3 * 3] = {
	   0, 1, 3, // first triangle
	   1, 2, 3  // second triangle
	};

	GLCore::Utils::OrthographicCameraController m_CameraController;


	GLint m_comp_shader;

	int m_WindowHeight = 720;
	int m_WindowWidth = 1280;

	GLuint tex_output;

	glm::vec2 m_MousePos;

	float m_TimestepLogger = 0;
	float m_UpdateFrequency = 0.01;

	std::string m_State = "Run Simulation";


	int m_GridSize = GRID_SIZE;

	int m_ParticleStates_in[GRID_SIZE * GRID_SIZE] = { 0 };
	int m_ParticleStates_out[GRID_SIZE * GRID_SIZE] = { 0 };

	bool m_Compute = false;

	int m_Counter = 0;

};