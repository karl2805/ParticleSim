#pragma once
#include <array>
#include <GLCore.h>
#include <GLCoreUtils.h>
#include "GLCore/Util/Texture.h"
#include "GLCore/Util/Shader.h"

#define GRID_SIZE 100

using namespace GLCore;
using namespace GLCore::Utils;

class GameOfLife
{
public:

	GameOfLife();
	~GameOfLife();

	void OnCompute();
	void OnRender(Timestep ts);
	void OnEvent(Event& e);

private:
	int m_GridSize = GRID_SIZE;


	//SSBO containers
	std::array<int, GRID_SIZE * GRID_SIZE> m_ParticleStates_in = { 0 };
	std::array<int, GRID_SIZE * GRID_SIZE> m_ParticleStates_out = { 0 };


	GLCore::Utils::OrthographicCameraController m_CameraController;







	//Drawing Stuff
	GLuint m_VBO, m_VAO, m_EBO;
	GLuint m_SSBO_in, m_SSBO_out;
	GLuint m_CompShader;
	GLCore::Utils::Shader* m_Shader;


};

