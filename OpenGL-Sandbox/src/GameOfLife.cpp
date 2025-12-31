#include "GameOfLife.h"
#include <GLFW/include/GLFW/glfw3.h>


using namespace GLCore;
using namespace GLCore::Utils;

GameOfLife::GameOfLife(int GridSize)
	:m_CameraController(1.0f), m_GridSize(GridSize),
	m_ParticleStates_in(GridSize* GridSize),
	m_ParticleStates_out(GridSize* GridSize),
	m_Grid(GridSize* GridSize)
{
	m_Shader = Shader::FromGLSLTextFiles("Shaders/CellRender.vert.glsl", "Shaders/CellRender.frag.glsl");

	m_GridShader = Shader::FromGLSLTextFiles("Shaders/GridRender.vert.glsl", "Shaders/GridRender.frag.glsl");

	m_CompShaderGameOfLife = CreateComputeShader("Shaders/GameOfLife.comp.glsl");
	m_CompShaderLiquidGas = CreateComputeShader("Shaders/LiquidGas.comp.glsl");

	if (m_ShouldCreateFramebuffer)
	{
		CreateFramebufferWithTexture();
		m_ShouldCreateFramebuffer = false;
	}

	FillGrid();
	RandomGrid();

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

	glGenBuffers(1, &m_SSBO_grid);
	glBindBuffer(GL_SHADER_STORAGE_BUFFER, m_SSBO_grid);
	glBufferData(GL_SHADER_STORAGE_BUFFER, m_Grid.size() * sizeof(int), m_Grid.data(), GL_STATIC_DRAW);
	glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 3, m_SSBO_grid);
	glBindBuffer(GL_SHADER_STORAGE_BUFFER, 0);
}

GameOfLife::~GameOfLife()
{
	GLuint buffers[] = { m_VBO, m_EBO, m_SSBO_in, m_SSBO_out, m_SSBO_grid };
	glDeleteBuffers(IM_ARRAYSIZE(buffers), buffers);

	glDeleteVertexArrays(1, &m_VAO);

	glDeleteProgram(m_Shader->GetRendererID());
	glDeleteProgram(m_CompShaderGameOfLife);
	glDeleteProgram(m_CompShaderLiquidGas);

	glDeleteFramebuffers(1, &m_FBO);

}

void GameOfLife::OnComputeGameOfLife()
{
	glUseProgram(m_CompShaderGameOfLife);

	int location = glGetUniformLocation(m_CompShaderGameOfLife, "u_GridSize");
	glUniform1i(location, m_GridSize);

	glDispatchCompute(m_GridSize / 8, m_GridSize / 8, 1);
	glMemoryBarrier(GL_SHADER_STORAGE_BARRIER_BIT);

	glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 1, m_SSBO_in);

	glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 2, m_SSBO_out);

	std::swap(m_SSBO_in, m_SSBO_out);

	glUseProgram(0);
}

void GameOfLife::OnComputeLiquidGas()
{
	glUseProgram(m_CompShaderLiquidGas);

	int location = glGetUniformLocation(m_CompShaderLiquidGas, "u_GridSize");
	glUniform1i(location, m_GridSize);

	location = glGetUniformLocation(m_CompShaderLiquidGas, "u_Temperature");
	glUniform1f(location, m_Temperature);

	location = glGetUniformLocation(m_CompShaderLiquidGas, "u_ChemicalPotential");
	glUniform1f(location, m_ChemcialPotential);

	static int step = 0;
	location = glGetUniformLocation(m_CompShaderLiquidGas, "u_Step");
	glUniform1i(location, step);
	step++;

	auto time = float(glfwGetTime());

	location = glGetUniformLocation(m_CompShaderLiquidGas, "u_Time");
	glUniform1f(location, time);

	glDispatchCompute(m_GridSize / 8, m_GridSize / 8, 1);
	glMemoryBarrier(GL_SHADER_STORAGE_BARRIER_BIT);

	glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 1, m_SSBO_in);

	glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 2, m_SSBO_out);

	std::swap(m_SSBO_in, m_SSBO_out);
}

void GameOfLife::OnRender(Timestep ts)
{
	m_CameraController.OnUpdate(ts);
	glBindFramebuffer(GL_FRAMEBUFFER, m_FBO);
	glViewport(0, 0, (int)m_FBTextureSize.x, (int)m_FBTextureSize.y);

	glClearColor(m_ClearColor.r, m_ClearColor.g, m_ClearColor.b, m_ClearColor.a);
	glClear(GL_COLOR_BUFFER_BIT);

	glUseProgram(m_Shader->GetRendererID());

	int location = glGetUniformLocation(m_Shader->GetRendererID(), "u_GridSize");
	glUniform1i(location, m_GridSize);

	location = glGetUniformLocation(m_Shader->GetRendererID(), "u_ViewProjection");
	glUniformMatrix4fv(location, 1, GL_FALSE, glm::value_ptr(m_CameraController.GetCamera().GetViewProjectionMatrix()));

	glUseProgram(m_GridShader->GetRendererID());

	location = glGetUniformLocation(m_GridShader->GetRendererID(), "u_GridSize");
	glUniform1i(location, m_GridSize);

	location = glGetUniformLocation(m_GridShader->GetRendererID(), "u_ViewProjection");
	glUniformMatrix4fv(location, 1, GL_FALSE, glm::value_ptr(m_CameraController.GetCamera().GetViewProjectionMatrix()));

	location = glGetUniformLocation(m_GridShader->GetRendererID(), "u_ClearColor");
	glUniform4fv(location, 1, glm::value_ptr(m_ClearColor));

	location = glGetUniformLocation(m_GridShader->GetRendererID(), "u_BorderThickness");
	glUniform1f(location, m_BorderThickness);

	glUseProgram(m_Shader->GetRendererID());

	glDrawElementsInstanced(GL_TRIANGLES, 6, GL_UNSIGNED_INT, nullptr, m_GridSize * m_GridSize);


	if (m_ShowGrid)
	{
		glUseProgram(m_GridShader->GetRendererID());

		glDrawElementsInstanced(GL_TRIANGLES, 6, GL_UNSIGNED_INT, nullptr, m_GridSize * m_GridSize);
	}

	glBindFramebuffer(GL_FRAMEBUFFER, 0);
	
}

void GameOfLife::CreateFramebufferWithTexture()
{
	glGenFramebuffers(1, &m_FBO);
	glBindFramebuffer(GL_FRAMEBUFFER, m_FBO);

	glGenTextures(1, &m_FBTexture);
	glBindTexture(GL_TEXTURE_2D, m_FBTexture);

	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, int(m_FBTextureSize.x), int(m_FBTextureSize.y), 0, GL_RGB, GL_UNSIGNED_BYTE, NULL);

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, m_FBTexture, 0);

	if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
	{
		LOG_WARN("Framebuffer Failed to Complete!");
		LOG_WARN(glCheckFramebufferStatus(GL_FRAMEBUFFER));
	}

	glBindFramebuffer(GL_FRAMEBUFFER, 0);

}

void GameOfLife::OnEvent(Event& event)
{
	//m_CameraController.OnEvent(event);
}

void GameOfLife::RandomGrid()
{
	for (int i = 0; i < m_GridSize * m_GridSize; i++)
	{
		int choices = 2;
		int picked_choice = (int)(rand() % choices);
		if (picked_choice == 1) {
			m_ParticleStates_in.at(i) = 1;
		}
	}
}

void GameOfLife::FillGrid()
{
	for (int i = 0; i < m_GridSize * m_GridSize; i++)
	{
		m_Grid.at(i) = 1;
	}
}


void GameOfLife::ReloadFramebuffer()
{
	glDeleteFramebuffers(1, &m_FBO);

	glDeleteTextures(1, &m_FBTexture);

	this->CreateFramebufferWithTexture();
}

void GameOfLife::SetGridBorderThickness(float thickness)
{
	m_BorderThickness = thickness;
}


GLuint GameOfLife::GetFBTextureID()
{
	return m_FBTexture;
}

void GameOfLife::AddCell(int xcoord, int ycoord)
{
	int index = GetCellIndex(xcoord, ycoord, this);

	std::cout << "Index: " << index << "\n";
	m_ParticleStates_in.at(index) = 1;

	glBindBuffer(GL_SHADER_STORAGE_BUFFER, m_SSBO_in);
	glBufferData(GL_SHADER_STORAGE_BUFFER, m_ParticleStates_in.size() * sizeof(int), m_ParticleStates_in.data(), GL_STATIC_DRAW);
	glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 1, m_SSBO_in);
}

void GameOfLife::RemoveCell(int xcoord, int ycoord)
{
	int index = GetCellIndex(xcoord, ycoord, this);

	std::cout << "Index: " << index << "\n";
	m_ParticleStates_in.at(index) = 0;

	glBindBuffer(GL_SHADER_STORAGE_BUFFER, m_SSBO_in);
	glBufferData(GL_SHADER_STORAGE_BUFFER, m_ParticleStates_in.size() * sizeof(int), m_ParticleStates_in.data(), GL_STATIC_DRAW);
	glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 1, m_SSBO_in);
}

int GetCellIndex(int xcoord, int ycoord, GameOfLife* game)
{
	float gridsize = game->m_GridSize;

	float cellwidth = 1000.0f / gridsize;

	glm::vec2 CellCoord = glm::vec2(floor((xcoord) / cellwidth), floor((std::abs(ycoord - 1000)) / cellwidth));

	std::cout << "Coord x" << CellCoord.x << "  y" << CellCoord.y << "\n";

	return (CellCoord.y) * gridsize + (CellCoord.x);
}

void GameOfLife::ChangeGridSize(int newGridSize)
{
	GLuint buffers[] = { m_SSBO_in, m_SSBO_out, m_SSBO_grid };
	glDeleteBuffers(2, buffers);


	m_ParticleStates_in.resize(newGridSize * newGridSize);
	m_ParticleStates_out.resize(newGridSize * newGridSize);
	m_Grid.resize(newGridSize * newGridSize);

	std::fill(m_ParticleStates_in.begin(), m_ParticleStates_in.end(), 0);
	std::fill(m_ParticleStates_out.begin(), m_ParticleStates_out.end(), 0);
	std::fill(m_Grid.begin(), m_Grid.end(), 0);

	m_GridSize = newGridSize;
	

	FillGrid();
	RandomGrid();

	//create SSBO Buffer
	glGenBuffers(1, &m_SSBO_in);
	glBindBuffer(GL_SHADER_STORAGE_BUFFER, m_SSBO_in);
	glBufferData(GL_SHADER_STORAGE_BUFFER, m_ParticleStates_in.size() * sizeof(int), m_ParticleStates_in.data(), GL_STATIC_DRAW);
	glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 1, m_SSBO_in);

	glGenBuffers(1, &m_SSBO_out);
	glBindBuffer(GL_SHADER_STORAGE_BUFFER, m_SSBO_out);
	glBufferData(GL_SHADER_STORAGE_BUFFER, m_ParticleStates_out.size() * sizeof(int), m_ParticleStates_out.data(), GL_STATIC_DRAW);
	glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 2, m_SSBO_out);

	glGenBuffers(1, &m_SSBO_grid);
	glBindBuffer(GL_SHADER_STORAGE_BUFFER, m_SSBO_grid);
	glBufferData(GL_SHADER_STORAGE_BUFFER, m_Grid.size() * sizeof(int), m_Grid.data(), GL_STATIC_DRAW);
	glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 3, m_SSBO_grid);
	glBindBuffer(GL_SHADER_STORAGE_BUFFER, 0);
	
}

void GameOfLife::ResetSimulation()
{
	std::fill(m_ParticleStates_in.begin(), m_ParticleStates_in.end(), 0);
	std::fill(m_ParticleStates_out.begin(), m_ParticleStates_out.end(), 0);

	RandomGrid();

	glBindBuffer(GL_SHADER_STORAGE_BUFFER, m_SSBO_in);
	glBufferData(GL_SHADER_STORAGE_BUFFER, m_ParticleStates_in.size() * sizeof(int), m_ParticleStates_in.data(), GL_STATIC_DRAW);
	glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 1, m_SSBO_in);

	glBindBuffer(GL_SHADER_STORAGE_BUFFER, m_SSBO_out);
	glBufferData(GL_SHADER_STORAGE_BUFFER, m_ParticleStates_out.size() * sizeof(int), m_ParticleStates_out.data(), GL_STATIC_DRAW);
	glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 2, m_SSBO_out);
}
