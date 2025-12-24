#include "GameOfLife.h"

using namespace GLCore;
using namespace GLCore::Utils;

GameOfLife::GameOfLife(int GridSize)
	:m_CameraController(1.0f), m_GridSize(GridSize),
	m_ParticleStates_in(GridSize * GridSize),
	m_ParticleStates_out(GridSize * GridSize)
{
	m_Shader = Shader::FromGLSLTextFiles("Shaders/vertex.vert.glsl", "Shaders/fragment.frag.glsl");

	m_CompShader = CreateComputeShader("Shaders/GameOfLife.comp.glsl");

	//RandomGrid();

	float vertices[3 * 4] = {
		// positions       
		 1.0f,  1.0f, 0.0f,
		 1.0f, -1.0f, 0.0f,
		-1.0f, -1.0f, 0.0f,
		-1.0f,  1.0f, 0.0f
	};

	glGenVertexArrays(1, &m_VAO);
	glGenBuffers(1, &m_VBO);
	glGenBuffers(1, &m_EBO);

	glBindVertexArray(m_VAO);

	glBindBuffer(GL_ARRAY_BUFFER, m_VBO);
	glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

	unsigned int indices[3 * 3] = {
	   0, 1, 3, // first triangle
	   1, 2, 3  // second triangle
	};

	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_EBO);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);

	// position attribute
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
	glEnableVertexAttribArray(0);


	//create SSBO Buffer
	glGenBuffers(1, &m_SSBO_in);
	glBindBuffer(GL_SHADER_STORAGE_BUFFER, m_SSBO_in);
	glBufferData(GL_SHADER_STORAGE_BUFFER, m_ParticleStates_in.size() * sizeof(int), m_ParticleStates_in.data(), GL_STATIC_DRAW);
	glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 1, m_SSBO_in);
	glBindBuffer(GL_SHADER_STORAGE_BUFFER, 0);

	glGenBuffers(1, &m_SSBO_out);
	glBindBuffer(GL_SHADER_STORAGE_BUFFER, m_SSBO_out);
	glBufferData(GL_SHADER_STORAGE_BUFFER, m_ParticleStates_out.size() * sizeof(int), m_ParticleStates_out.data(), GL_STATIC_DRAW);
	glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 2, m_SSBO_out);
	glBindBuffer(GL_SHADER_STORAGE_BUFFER, 0);
}

GameOfLife::~GameOfLife()
{
	GLuint buffers[] = { m_VBO, m_EBO, m_SSBO_in, m_SSBO_out };
	glDeleteBuffers(IM_ARRAYSIZE(buffers), buffers);

	glDeleteVertexArrays(1, &m_VAO);

	glDeleteProgram(m_Shader->GetRendererID());
	glDeleteProgram(m_CompShader);


}

void GameOfLife::OnCompute()
{
	glUseProgram(m_CompShader);

	int location = glGetUniformLocation(m_CompShader, "u_GridSize");
	glUniform1i(location, m_GridSize);

	glDispatchCompute(m_GridSize / 8, m_GridSize / 8, 1);
	glMemoryBarrier(GL_SHADER_STORAGE_BARRIER_BIT);

	glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 1, m_SSBO_in);

	glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 2, m_SSBO_out);

	std::swap(m_SSBO_in, m_SSBO_out);
}

void GameOfLife::OnRender(Timestep ts)
{
	m_CameraController.OnUpdate(ts);
	glClearColor(0.5f, 0.5f, 0.5f, 1.0f);
	glClear(GL_COLOR_BUFFER_BIT);

	glUseProgram(m_Shader->GetRendererID());

	int location = glGetUniformLocation(m_Shader->GetRendererID(), "u_GridSize");
	glUniform1i(location, m_GridSize);

	location = glGetUniformLocation(m_Shader->GetRendererID(), "u_ViewProjection");
	glUniformMatrix4fv(location, 1, GL_FALSE, glm::value_ptr(m_CameraController.GetCamera().GetViewProjectionMatrix()));

	glDrawElementsInstanced(GL_TRIANGLES, 6, GL_UNSIGNED_INT, nullptr, m_GridSize * m_GridSize);
}

void GameOfLife::OnEvent(Event& event)
{
	m_CameraController.OnEvent(event);
}

void GameOfLife::RandomGrid()
{
	for (int i = 0; i < m_GridSize * m_GridSize; i++)
	{
		int choices = 5;
		int picked_choice = (int)(rand() % choices);
		if (picked_choice == 1) {
			m_ParticleStates_in.at(i) = 1;
		}
	}
}


void GameOfLife::AddCell(int xcoord, int ycoord)
{
	int index = GetCellIndex(xcoord, ycoord, this);
	m_ParticleStates_in.at(index) = 1;

	glBindBuffer(GL_SHADER_STORAGE_BUFFER, m_SSBO_in);
	glBufferData(GL_SHADER_STORAGE_BUFFER, m_ParticleStates_in.size() * sizeof(int), m_ParticleStates_in.data(), GL_STATIC_DRAW);
	glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 1, m_SSBO_in);
}

int GetCellIndex(int xcoord, int ycoord, GameOfLife* game)
{
	int gridsize = game->m_GridSize;

	glm::vec2 CellCoord = glm::vec2(xcoord / gridsize, (ycoord - 1000)/ gridsize);


	return (CellCoord.y) * gridsize + (CellCoord.x);
}

GameOfLife* ChangeGridSize(GameOfLife* game, int newGridSize)
{
	delete game;

	GameOfLife* newGame = new GameOfLife(newGridSize);

	return newGame;
	
}
