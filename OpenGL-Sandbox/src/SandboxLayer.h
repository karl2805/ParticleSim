#pragma once

#include "GameOfLife.h"
#include "LiquidGas.h"

#define INIT_GRID_SIZE 256

enum State
{
	COMPUTE,
	PAUSED
};

enum MouseButton
{
	RightClick = 0,
	LeftClick
};

enum SimulationSelector
{
	GAMEOFLIFE,
	LIQUIDGAS
};

class SandboxLayer : public GLCore::Layer
{
public:
	SandboxLayer();
	virtual ~SandboxLayer();

	virtual void OnAttach() override;
	virtual void OnDetach() override;
	virtual void OnEvent(GLCore::Event& event) override;
	virtual void OnUpdate(GLCore::Timestep ts) override;
	virtual void OnRender() override;
	virtual void OnImGuiRender() override;
private:
	
	
	
	GameOfLife* m_Game = new GameOfLife(INIT_GRID_SIZE);
	LiquidGas* m_LiquidGas = new LiquidGas(INIT_GRID_SIZE);

	State m_ComputeState = PAUSED;
	SimulationSelector m_WhatSimulation = LIQUIDGAS;

	float m_UpdateFrequency = 0.0f;
	float m_TimeCounter = 0;

	int m_GridSize = INIT_GRID_SIZE;


	float m_XMousePos;
	float m_YMousePos;

	//IMGUI sliders
	double m_TempSlider = 2.0f;
	double m_CPSlider = 2.0f;

	glm::vec2 m_WindowSize;

	glm::vec2 m_ViewportSize;
};