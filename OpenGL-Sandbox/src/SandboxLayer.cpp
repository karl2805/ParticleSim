#include "SandboxLayer.h"



using namespace GLCore;
using namespace GLCore::Utils;



SandboxLayer::SandboxLayer()
	:m_WindowSize(glm::vec2(2560, 1440))
{

}

SandboxLayer::~SandboxLayer()
{
	delete m_Game;
	delete m_LiquidGas;

}

void SandboxLayer::OnAttach()
{
	EnableGLDebugging();
	m_Game->m_GridSize = INIT_GRID_SIZE;

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

	/*dispatcher.Dispatch<MouseButtonPressedEvent>([&](MouseButtonPressedEvent& e)
		{
			if (e.GetMouseButton() == RightClick)
			{
				m_Game->AddCell(m_XMousePos, m_YMousePos);

			}
			return false;
		});*/

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
	m_Game->m_Temperature = m_TempSlider;
	m_Game->m_ChemcialPotential = m_CPSlider;

	if (m_ComputeState == COMPUTE && m_TimeCounter >= m_UpdateFrequency)
	{
		switch (m_WhatSimulation)
		{
		case GAMEOFLIFE:
		{
			m_Game->OnComputeGameOfLife();
			break;
		}

		case LIQUIDGAS:
		{
			m_Game->OnComputeLiquidGas();
			break;
		}
		default:
			break;
		}

		m_TimeCounter = 0;
	}

	m_Game->OnRender(ts);


	m_TimeCounter += ts;

	glBindFramebuffer(GL_FRAMEBUFFER, 0);
	glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
	glClear(GL_COLOR_BUFFER_BIT);
}

void SandboxLayer::OnRender()
{

}

void SandboxLayer::OnImGuiRender()
{
	ImGui::GetIO().FontGlobalScale = 1.5f;
	ImGui::DockSpaceOverViewport();

	ImGui::GetIO().WantCaptureMouse = false;


	ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2{ 0, 0 });

	ImGui::Begin("Viewport", nullptr, ImGuiWindowFlags_NoResize);
	ImVec2 viewportpanelsize = ImGui::GetContentRegionAvail();

	if (m_ViewportSize != *((glm::vec2*)&viewportpanelsize))
	{
		m_Game->m_FBTextureSize = { viewportpanelsize.x, viewportpanelsize.y };
		m_Game->ReloadFramebuffer();

		m_ViewportSize = { viewportpanelsize.x, viewportpanelsize.y };
	}


	ImGui::Image((ImTextureID)m_Game->GetFBTextureID(), viewportpanelsize);
	ImGui::End();
	ImGui::PopStyleVar();

	ImGui::Begin("Controls");

	if (ImGui::Button("Start"))
	{
		m_ComputeState = m_ComputeState == COMPUTE ? PAUSED : COMPUTE;
	}

	if (ImGui::Button("Reset"))
	{
		m_Game->ResetSimulation();
		m_ComputeState = PAUSED;
	}


	if (ImGui::SliderInt("GridSize", &m_GridSize, 4, 2000))
	{
		m_ComputeState = PAUSED;
		m_Game->ChangeGridSize(m_GridSize);
	}

	float borderthickness = 0.1f;

	if (ImGui::SliderFloat("Grid Thickness", &borderthickness, 0.0f, 1.0f))
	{
		m_Game->SetGridBorderThickness(borderthickness);
	}

	ImGui::SliderFloat("Update Frequency", &m_UpdateFrequency, 0.0f, 0.1f);
	

	if (ImGui::Button("Show Grid"))
	{
		m_Game->ToggleGrid();
		m_LiquidGas->ToggleGrid();
	}

	if (ImGui::Button("Switch Sim"))
	{
		m_WhatSimulation = m_WhatSimulation == GAMEOFLIFE ? LIQUIDGAS : GAMEOFLIFE;
	}

	double max = 10.0;
	double min = 0.0;
	ImGui::SliderScalar("Temperature", ImGuiDataType_Double, &m_TempSlider, &min, &max);

	max = -1.0;
	min = -3.0;
	ImGui::SliderScalar("Chemical Potential", ImGuiDataType_Double, &m_CPSlider, &min, &max);
	


	
	ImGui::End();



}
