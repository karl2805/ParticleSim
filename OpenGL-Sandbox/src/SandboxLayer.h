#pragma once

#include <GLCore.h>
#include <GLCoreUtils.h>
#include "GLCore/Util/Texture.h"
#include "GLCore/Util/Shader.h"

class SandboxLayer : public GLCore::Layer
{
public:
	SandboxLayer();
	virtual ~SandboxLayer();

	virtual void OnAttach() override;
	virtual void OnDetach() override;
	virtual void OnEvent(GLCore::Event& event) override;
	virtual void OnUpdate(GLCore::Timestep ts) override;
	virtual void OnImGuiRender() override;
private:
	Texture m_texture;
	GLCore::Utils::Shader* m_shader;
	unsigned int VBO, VAO, EBO;
};