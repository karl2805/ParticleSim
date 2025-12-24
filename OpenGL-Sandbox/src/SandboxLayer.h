#pragma once



#include "GameOfLife.h"

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
	
	
	
	GameOfLife* m_Game = new GameOfLife(64);

	State m_ComputeState = PAUSED;
	float m_UpdateFrequency = 0.05f;
	float m_TimeCounter = 0;

	int m_GridSize = 64;


	float m_XMousePos;
	float m_YMousePos;


	
};