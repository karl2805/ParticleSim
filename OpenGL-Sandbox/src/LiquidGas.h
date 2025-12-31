#pragma once
#include <array>
#include <GLCore.h>
#include <GLCoreUtils.h>
#include "GLCore/Core/Log.h"
#include "GLCore/Util/Shader.h"
#include <random>
#include <cmath>


using namespace GLCore;
using namespace GLCore::Utils;


class LiquidGas
{
	friend int GetCellIndex(int xcoord, int ycoord, LiquidGas* game);
public:

	LiquidGas(int GridSize);
	~LiquidGas();

	void OnCompute();
	void OnRender(Timestep ts);
	void CreateFramebufferWithTexture();
	void OnEvent(Event& e);
	void RandomParticlesFill();
	void AddCell(int xcoord, int ycoord);
	void RemoveCell(int xcoord, int ycoord);
	void FillGrid();
	void ToggleGrid() { m_ShowGrid = m_ShowGrid ? false : true; };
	void ChangeGridSize(int newsize);

	int CellIndex(int x, int y);
	int CellActive(int x, int y);

	void ReloadFramebuffer();

	void TestFill();

	//Setters
	void SetGridBorderThickness(float thickness);


	GLuint GetFBTextureID();

	int m_GridSize;
	glm::vec2 m_FBTextureSize = glm::vec2(500.0f, 500.0f);

	GLCore::Utils::OrthographicCameraController m_CameraController;

	double m_Temperature = 5;

private:

	std::default_random_engine m_Generator;
	std::uniform_int_distribution<int> m_Distribution;


	std::vector<int> m_Grid;
	std::vector<int> m_ParticleStates;
	std::vector<int> m_Debug;

	//Conditions
	bool m_ShowGrid = false;
	bool m_ShouldCreateFramebuffer = true;
	float m_BorderThickness = 0.1f;


	//Drawing Stuff
	GLuint m_VBO, m_VAO, m_EBO;
	GLuint m_SSBO_grid, m_SSBO_particles, m_SSBO_debug;
	GLCore::Utils::Shader* m_Shader;
	GLCore::Utils::Shader* m_GridShader;
	glm::vec4 m_ClearColor = glm::vec4(0.5f, 0.5f, 0.5f, 1.0f);

	//Framebuffer
	GLuint m_FBO;
	GLuint m_FBTexture;
};

LiquidGas* ChangeGridSize(LiquidGas* game, int newGridSize);



