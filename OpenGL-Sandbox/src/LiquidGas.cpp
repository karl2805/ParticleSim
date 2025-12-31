#include "LiquidGas.h"


using namespace GLCore;
using namespace GLCore::Utils;

#include <random>
#include <GLCore/Core/Log.h>
#include <cstdlib>
#include <ctime>





LiquidGas::LiquidGas(int GridSize)
	:m_CameraController(1.0f), m_GridSize(GridSize),
	m_Grid(GridSize* GridSize),
	m_ParticleStates(GridSize * GridSize),
	m_Debug(GridSize * GridSize),
	m_Distribution(0, GridSize - 1)
{
	m_Shader = Shader::FromGLSLTextFiles("Shaders/CellRender.vert.glsl", "Shaders/CellRender.frag.glsl");

	m_GridShader = Shader::FromGLSLTextFiles("Shaders/GridRender.vert.glsl", "Shaders/GridRender.frag.glsl");

	if (m_ShouldCreateFramebuffer)
	{
		CreateFramebufferWithTexture();
		m_ShouldCreateFramebuffer = false;
	}

	FillGrid();
	RandomParticlesFill();
	
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

	glGenBuffers(1, &m_SSBO_particles);
	glBindBuffer(GL_SHADER_STORAGE_BUFFER, m_SSBO_particles);
	glBufferData(GL_SHADER_STORAGE_BUFFER, m_ParticleStates.size() * sizeof(int), m_ParticleStates.data(), GL_STATIC_DRAW);
	glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 1, m_SSBO_particles);
	glBindBuffer(GL_SHADER_STORAGE_BUFFER, 0);

	glGenBuffers(1, &m_SSBO_grid);
	glBindBuffer(GL_SHADER_STORAGE_BUFFER, m_SSBO_grid);
	glBufferData(GL_SHADER_STORAGE_BUFFER, m_Grid.size() * sizeof(int), m_Grid.data(), GL_STATIC_DRAW);
	glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 3, m_SSBO_grid);
	glBindBuffer(GL_SHADER_STORAGE_BUFFER, 0);

	glGenBuffers(1, &m_SSBO_debug);
	glBindBuffer(GL_SHADER_STORAGE_BUFFER, m_SSBO_debug);
	glBufferData(GL_SHADER_STORAGE_BUFFER, m_Debug.size() * sizeof(int), m_Debug.data(), GL_STATIC_DRAW);
	glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 4, m_SSBO_debug);
	glBindBuffer(GL_SHADER_STORAGE_BUFFER, 0);
}

LiquidGas::~LiquidGas()
{
	GLuint buffers[] = { m_VBO, m_EBO, m_SSBO_grid, m_SSBO_particles};
	glDeleteBuffers(IM_ARRAYSIZE(buffers), buffers);

	glDeleteVertexArrays(1, &m_VAO);

	glDeleteProgram(m_Shader->GetRendererID());

	glDeleteFramebuffers(1, &m_FBO);

}

int LiquidGas::CellIndex(int x, int y)
{
	if (std::sqrt(m_ParticleStates.size()) != int(std::sqrt(m_ParticleStates.size())))
		LOG_WARN("NOT A SQUARE NUMBER! CELL INDEX ERROR");
	
	
	int gridheight = m_GridSize;
	return (y * gridheight + x);
}

int LiquidGas::CellActive(int x, int y)
{

	//out of bounds check
	if (x > m_GridSize - 1 || x < 0 ||
		y > m_GridSize - 1 || y < 0)
	{
		return 0;
	}

	int state = m_ParticleStates[this->CellIndex(x, y)];
	if (state == 0 || state == 1)
		return state;

	else
	{
		LOG_WARN("CELL STATE NOT 0 or 1 !!!!!!");
		return state;
	}
}

std::random_device rd;
std::mt19937 gen(rd());
std::uniform_real_distribution<double> dist(0.0, 1.0);

#define BIT(x) 1 << x
enum EdgeCase
{
	NONE = BIT(0),

	MAX_X_1 = BIT(1),
	MAX_X_2 = BIT(2),

	MIN_X_1 = BIT(3),
	MIN_X_2 = BIT(4),

	MAX_Y_1 = BIT(5),
	MAX_Y_2 = BIT(6),

	MIN_Y_1 = BIT(7),
	MIN_Y_2 = BIT(8)
};

struct Particles
{
	glm::ivec2 a;
	glm::ivec2 b;
	int max;
};

int GetEdgeCase(Particles& p)
{
	int edgecase = NONE;

	if (p.a.x == p.max)
		edgecase |= MAX_X_1;

	if (p.b.x == p.max)
		edgecase |= MAX_X_2;

	if (p.a.x == 0)
		edgecase |= MIN_X_1;

	if (p.b.x == 0)
		edgecase |= MIN_X_2;

	if (p.a.y == p.max)
		edgecase |= MAX_Y_1;

	if (p.b.y == p.max)
		edgecase |= MAX_Y_2;

	if (p.a.y == 0)
		edgecase |= MIN_Y_1;

	if (p.b.y == 0)
		edgecase |= MIN_Y_2;

	return edgecase;
}


constexpr double kB = 1.380649e-23;
void LiquidGas::OnCompute()
{

	
	int max_xy = m_GridSize - 1;
	for (int i = 0; i < m_GridSize * m_GridSize; i++)
	{
		glm::ivec2 particle_1{ m_Distribution(m_Generator), m_Distribution(m_Generator) };
		glm::ivec2 particle_2{ m_Distribution(m_Generator), m_Distribution(m_Generator) };

		
		
		//do nothing if equal
		if (CellActive(particle_1.x, particle_1.y) == CellActive(particle_2.x, particle_2.y))
			return;

		int active_neighbours_1 = -1, active_neighbours_2 = -1;

		active_neighbours_1 = CellActive((particle_1.x + 1), (particle_1.y + 1)) +
			CellActive((particle_1.x + 1), (particle_1.y)) +
			CellActive((particle_1.x + 1), (particle_1.y - 1)) +
			CellActive((particle_1.x), (particle_1.y - 1)) +
			CellActive((particle_1.x - 1), (particle_1.y - 1)) +
			CellActive((particle_1.x - 1), (particle_1.y)) +
			CellActive((particle_1.x - 1), (particle_1.y + 1)) +
			CellActive((particle_1.x), (particle_1.y + 1));

		active_neighbours_2 = CellActive((particle_2.x + 1), (particle_2.y + 1)) +
			CellActive((particle_2.x + 1), (particle_2.y)) +
			CellActive((particle_2.x + 1), (particle_2.y - 1)) +
			CellActive((particle_2.x), (particle_2.y - 1)) +
			CellActive((particle_2.x - 1), (particle_2.y - 1)) +
			CellActive((particle_2.x - 1), (particle_2.y)) +
			CellActive((particle_2.x - 1), (particle_2.y + 1)) +
			CellActive((particle_2.x), (particle_2.y + 1));
		


		int energy_diff = std::abs(active_neighbours_1 - active_neighbours_2);
		

		double q = std::exp( -energy_diff / (m_Temperature));

		double swap_probability = (q) / (1.0 + q);

		int particle_1_index = CellIndex(particle_1.x, particle_1.y);
		int particle_2_index = CellIndex(particle_2.x, particle_2.y);

		if (dist(gen) < swap_probability)
			std::swap(m_ParticleStates[particle_1_index], m_ParticleStates[particle_2_index]);
	}

	glBindBuffer(GL_SHADER_STORAGE_BUFFER, m_SSBO_particles);
	glBufferData(GL_SHADER_STORAGE_BUFFER, m_ParticleStates.size() * sizeof(int), m_ParticleStates.data(), GL_STATIC_DRAW);
	glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 1, m_SSBO_particles);
	glBindBuffer(GL_SHADER_STORAGE_BUFFER, 0);

	glBindBuffer(GL_SHADER_STORAGE_BUFFER, m_SSBO_debug);
	glBufferData(GL_SHADER_STORAGE_BUFFER, m_Debug.size() * sizeof(int), m_Debug.data(), GL_STATIC_DRAW);
	glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 4, m_SSBO_debug);
	glBindBuffer(GL_SHADER_STORAGE_BUFFER, 0);

}

void LiquidGas::OnRender(Timestep ts)
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

void LiquidGas::CreateFramebufferWithTexture()
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

void LiquidGas::OnEvent(Event& event)
{
	//m_CameraController.OnEvent(event);
}

void LiquidGas::RandomParticlesFill()
{
	for (int i = 0; i < m_GridSize * m_GridSize; i++)
	{
		int choices = 2;
		int picked_choice = (int)(rand() % choices);
		if (picked_choice == 1) {
			m_ParticleStates.at(i) = 1;
		}
	}
}

void LiquidGas::TestFill()
{
	for (int i = 0; i < 600; i++)
	{
		m_ParticleStates[i] = 1;
	}
}

void LiquidGas::FillGrid()
{
	for (int i = 0; i < m_GridSize * m_GridSize; i++)
	{
		m_Grid.at(i) = 1;
	}
}


void LiquidGas::ReloadFramebuffer()
{
	glDeleteFramebuffers(1, &m_FBO);

	glDeleteTextures(1, &m_FBTexture);

	this->CreateFramebufferWithTexture();
}

void LiquidGas::SetGridBorderThickness(float thickness)
{
	m_BorderThickness = thickness;
}


GLuint LiquidGas::GetFBTextureID()
{
	return m_FBTexture;
}



void LiquidGas::ChangeGridSize(int newGridSize)
{
	m_ParticleStates.resize(newGridSize * newGridSize);
	m_Grid.resize(newGridSize * newGridSize);

	std::fill(m_ParticleStates.begin(), m_ParticleStates.end(), 0);
	std::fill(m_Grid.begin(), m_Grid.end(), 0);

	m_GridSize = newGridSize;

	FillGrid();
	RandomParticlesFill();

	//create SSBO Buffer
	glGenBuffers(1, &m_SSBO_grid);
	glBindBuffer(GL_SHADER_STORAGE_BUFFER, m_SSBO_grid);
	glBufferData(GL_SHADER_STORAGE_BUFFER, m_Grid.size() * sizeof(int), m_Grid.data(), GL_STATIC_DRAW);
	glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 3, m_SSBO_grid);
	glBindBuffer(GL_SHADER_STORAGE_BUFFER, 0);

	glGenBuffers(1, &m_SSBO_particles);
	glBindBuffer(GL_SHADER_STORAGE_BUFFER, m_SSBO_particles);
	glBufferData(GL_SHADER_STORAGE_BUFFER, m_ParticleStates.size() * sizeof(int), m_ParticleStates.data(), GL_STATIC_DRAW);
	glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 1, m_SSBO_grid);
	glBindBuffer(GL_SHADER_STORAGE_BUFFER, 0);

}

