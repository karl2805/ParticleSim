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
	friend int GetCellIndex(int xcoord, int ycoord, GameOfLife* game);
public:

	GameOfLife(int GridSize);
	~GameOfLife();

	void OnCompute();
	void OnRender(Timestep ts);
	void OnEvent(Event& e);
	void RandomGrid();
	void AddCell(int xcoord, int ycoord);
	void RemoveCell(int xcoord, int ycoord);
	void FillGrid();
	void ToggleGrid() { m_ShowGrid = m_ShowGrid ? false : true; };

private:
	int m_GridSize;

	GLCore::Utils::OrthographicCameraController m_CameraController;

	std::vector<int> m_ParticleStates_in;
	std::vector<int> m_ParticleStates_out;
	std::vector<int> m_Grid;

	//Conditions
	bool m_ShowGrid = true;

	//Drawing Stuff
	GLuint m_VBO, m_VAO, m_EBO;
	GLuint m_SSBO_in, m_SSBO_out, m_SSBO_grid;
	GLuint m_CompShader;
	GLCore::Utils::Shader* m_Shader;
	GLCore::Utils::Shader* m_GridShader;
	glm::vec4 m_ClearColor = glm::vec4(0.5f, 0.5f, 0.5f, 1.0f);


};

GameOfLife* ChangeGridSize(GameOfLife* game, int newGridSize);

