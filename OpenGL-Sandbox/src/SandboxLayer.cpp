#include "SandboxLayer.h"
#include "GLCore/Util/Shader.h"
#include <print>
#include <windows.h>
#include "GLCore/Core/KeyCodes.h"

using namespace GLCore;
using namespace GLCore::Utils;

SandboxLayer::SandboxLayer()
{
    for (int i = 0; i < GRID_SIZE * GRID_SIZE; i++)
    {
        int choices = 10;
        int picked_choice = (int)(rand() % choices);
        if (picked_choice == 1) {
            m_ParticleStates_in[i] = 1;
        }
    }

   /* m_ParticleStates_in[500] = 1;
    m_ParticleStates_in[500] = 1;
    m_ParticleStates_in[500] = 1;
    m_ParticleStates_in[500] = 1;
    m_ParticleStates_in[500] = 1;*/


}

SandboxLayer::~SandboxLayer()
{
}

void SandboxLayer::OnAttach()
{
	EnableGLDebugging();

    m_shader = Shader::FromGLSLTextFiles("Shaders/vertex.vert.glsl", "Shaders/fragment.frag.glsl");

    m_comp_shader = CreateComputeShader("Shaders/compute.comp.glsl");

    
	float vertices[] = {
		// positions       
		 1.0f,  1.0f, 0.0f,
		 1.0f, -1.0f, 0.0f,
		-1.0f, -1.0f, 0.0f,
		-1.0f,  1.0f, 0.0f
	};

    unsigned int indices[] = {
       0, 1, 3, // first triangle
       1, 2, 3  // second triangle
    };
    
    glGenVertexArrays(1, &m_VAO);
    glGenBuffers(1, &m_VBO);
    glGenBuffers(1, &m_EBO);

    glBindVertexArray(m_VAO);

    glBindBuffer(GL_ARRAY_BUFFER, m_VBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_EBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);

    // position attribute
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);

    
    //create SSBO Buffer
    glGenBuffers(1, &m_SSBO_in);
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, m_SSBO_in);
    glBufferData(GL_SHADER_STORAGE_BUFFER, sizeof(m_ParticleStates_in), m_ParticleStates_in, GL_STATIC_DRAW);
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 1, m_SSBO_in);
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, 0);

    glGenBuffers(1, &m_SSBO_out);
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, m_SSBO_out);
    glBufferData(GL_SHADER_STORAGE_BUFFER, sizeof(m_ParticleStates_out), m_ParticleStates_out, GL_STATIC_DRAW);
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 2, m_SSBO_out);
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, 0);

}

void SandboxLayer::OnDetach()
{
	// Shutdown here
}

int CellIndex(glm::vec2 CellCoord)
{
    return int(CellCoord.y) * GRID_SIZE + int(CellCoord.x) * 0.001;
}

void SandboxLayer::OnEvent(Event& event)
{
    EventDispatcher dispatcher(event);

    dispatcher.Dispatch<MouseMovedEvent>(
        [&](MouseMovedEvent& e)
        {
            m_MousePos.x = e.GetX();
            m_MousePos.y = e.GetY();

            std::cout << m_MousePos.x << "   " << m_MousePos.y << "\n";

            return false;
        });

    dispatcher.Dispatch<MouseButtonPressedEvent>(
        [&](MouseButtonPressedEvent& e)
        {
            
            m_ParticleStates_in[1000] = 1;
            m_ParticleStates_out[1000] = 1;

            std::cout << "preessed";

            glBindBuffer(GL_SHADER_STORAGE_BUFFER, m_SSBO_in);
            glBufferData(GL_SHADER_STORAGE_BUFFER, sizeof(m_ParticleStates_in), m_ParticleStates_in, GL_DYNAMIC_STORAGE_BIT);
            glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 1, m_SSBO_in);

            return false;
        });

    dispatcher.Dispatch<KeyPressedEvent>(
        [&](KeyPressedEvent&e)
        {
            m_Compute = m_Compute ? false : true;

            return false;
        });

    
    
}

void SandboxLayer::OnUpdate(Timestep ts)
{
   

    glClearColor(0.5f, 0.5f, 0.5f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT);

   
    glUseProgram(m_shader->GetRendererID());

    

    //set uniforms

    int location = glGetUniformLocation(m_shader->GetRendererID(), "u_GridSize");
    glUniform1i(location, m_GridSize);

    glDrawElementsInstanced(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0, GRID_SIZE * GRID_SIZE);

   
    
        

       
        glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 1, m_SSBO_in);
        glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 2, m_SSBO_out);

        
        Sleep(100);
    

    
    glUseProgram(m_comp_shader);

    glDispatchCompute(GRID_SIZE / 8, GRID_SIZE / 8, 1);
    glMemoryBarrier(GL_SHADER_STORAGE_BARRIER_BIT);

    std::swap(m_SSBO_in, m_SSBO_out);
}

void SandboxLayer::OnRender()
{

}

void SandboxLayer::OnImGuiRender()
{
	// ImGui here
}
