#include "SandboxLayer.h"



using namespace GLCore;
using namespace GLCore::Utils;



SandboxLayer::SandboxLayer()
{
    
   

}

SandboxLayer::~SandboxLayer()
{
    delete m_Game;

}

void SandboxLayer::OnAttach()
{
	EnableGLDebugging();
    
    

    

}

void SandboxLayer::OnDetach()
{
    
}



void SandboxLayer::OnEvent(Event& event)
{
    m_Game->OnEvent(event);

    EventDispatcher dispatcher(event);

    dispatcher.Dispatch<MouseMovedEvent>([&](MouseMovedEvent& e) 
        {
            m_XMousePos = e.GetX();
            m_YMousePos = e.GetY();

            

            return false;
        });

	dispatcher.Dispatch<MouseButtonPressedEvent>([&](MouseButtonPressedEvent& e)
		{
            if (e.GetMouseButton() == RightClick)
            {
                m_Game->AddCell(m_XMousePos, m_YMousePos);

            }
            return false;
		});

    dispatcher.Dispatch<MouseButtonPressedEvent>([&](MouseButtonPressedEvent& e)
        {
            if (e.GetMouseButton() == LeftClick)
            {
                m_Game->RemoveCell(m_XMousePos, m_YMousePos);

            }
            return false;
        });
    
}

void SandboxLayer::OnUpdate(Timestep ts)
{
    
    if (m_ComputeState == COMPUTE && m_TimeCounter >= m_UpdateFrequency)
    {
        m_Game->OnCompute();
        m_TimeCounter = 0;
    }
    
    m_Game->OnRender(ts);

    m_TimeCounter += ts;
}

void SandboxLayer::OnRender()
{

}

void SandboxLayer::OnImGuiRender()
{
    if (ImGui::Button("Start"))
    {
        m_ComputeState = m_ComputeState == COMPUTE ? PAUSED : COMPUTE;
    }

    if (ImGui::SliderInt("GridSize", &m_GridSize, 4, 1000))
    {
        m_ComputeState = PAUSED;
        m_Game = ChangeGridSize(m_Game, m_GridSize);
    }

    if (ImGui::Button("Show Grid"))
    {
        m_Game->ToggleGrid();
    }

}
