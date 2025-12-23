#include "SandboxLayer.h"
#include "GLCore/Util/Shader.h"
#include <print>
#include <windows.h>
#include "GLCore/Core/KeyCodes.h"

using namespace GLCore;
using namespace GLCore::Utils;

SandboxLayer::SandboxLayer()
    : m_CameraController(1.0f / 1.0f)
{
   

}

SandboxLayer::~SandboxLayer()
{
}

void SandboxLayer::OnAttach()
{
	EnableGLDebugging();

    m_shader = Shader::FromGLSLTextFiles("Shaders/vertex.vert.glsl", "Shaders/fragment.frag.glsl");

    m_comp_shader = CreateComputeShader("Shaders/GameOfLife.comp.glsl");

    for (int i = 0; i < m_GridSize * m_GridSize; i++)
    {
        int choices = 5;
        int picked_choice = (int)(rand() % choices);
        if (picked_choice == 1) {
            m_ParticleStates_in[i] = 1;
        }
    }
    
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
    glDeleteBuffers(sizeof(vertices), &m_VBO);
    glDeleteBuffers(sizeof(indices), &m_EBO);

  

    //set the ping pong arrays to zero
    memset(m_ParticleStates_in, 0, sizeof(m_ParticleStates_in));
    memset(m_ParticleStates_out, 0, sizeof(m_ParticleStates_out));
}



void SandboxLayer::OnEvent(Event& event)
{

    m_CameraController.OnEvent(event);

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
            glBufferData(GL_SHADER_STORAGE_BUFFER, sizeof(m_ParticleStates_in), m_ParticleStates_in, GL_STATIC_DRAW);
            glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 1, m_SSBO_in);

            return false;
        });

    
    

  

}

void SandboxLayer::OnUpdate(Timestep ts)
{
    m_CameraController.OnUpdate(ts);
    glClearColor(0.5f, 0.5f, 0.5f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT);

    glUseProgram(m_shader->GetRendererID());

    int location = glGetUniformLocation(m_shader->GetRendererID(), "u_GridSize");
    glUniform1i(location, m_GridSize);

    location = glGetUniformLocation(m_shader->GetRendererID(), "u_ViewProjection");
    glUniformMatrix4fv(location, 1, GL_FALSE, glm::value_ptr(m_CameraController.GetCamera().GetViewProjectionMatrix()));

    glDrawElementsInstanced(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0, m_GridSize * m_GridSize);
    
    m_TimestepLogger += ts;

    if (m_TimestepLogger >= m_UpdateFrequency && m_Compute)
    {

        glUseProgram(m_comp_shader);

        location = glGetUniformLocation(m_comp_shader, "u_GridSize");
        glUniform1i(location, m_GridSize);

        glDispatchCompute(m_GridSize * m_GridSize, 1, 1);
        glMemoryBarrier(GL_SHADER_STORAGE_BARRIER_BIT);

        glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 1, m_SSBO_in);

        glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 2, m_SSBO_out);

        std::swap(m_SSBO_in, m_SSBO_out);

        m_TimestepLogger = 0;
    }
}

void SandboxLayer::OnRender()
{

}

void SandboxLayer::OnImGuiRender()
{
    ImGui::Begin("Viewport");

    

    if (ImGui::Button(m_State.c_str()))
    {
        m_Compute = m_Compute ? false : true;
        m_State = m_Compute ? "Pause Simulation" : "Run Simulation";
    }

    if (ImGui::Button("Reset"))
    {
        OnDetach();
        OnAttach();
    }


    ImGui::SliderFloat("Speed", &m_UpdateFrequency, 0.2f, 0.0f);
    ImGui::SliderInt("GridSize", &m_GridSize, 4, 1000);

    ImGui::End();

}
