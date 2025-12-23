#pragma once


#include <GLCore.h>
#include "GLCore/Util/Texture.h"
#include "GameOfLife.h"

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
	
	
	
	GameOfLife m_Game;


	int m_WindowHeight = 720;
	int m_WindowWidth = 1280;

	
};