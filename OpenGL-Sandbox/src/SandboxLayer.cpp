#include "SandboxLayer.h"
#include "GLCore/Util/Shader.h"
#include <print>


using namespace GLCore;
using namespace GLCore::Utils;

SandboxLayer::SandboxLayer()
{
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
		// positions          // colors           // texture coords
		 1.0f,  1.0f, 0.0f,   1.0f, 0.0f, 0.0f,   1.0f, 1.0f, // top right
		 1.0f, -1.0f, 0.0f,   0.0f, 1.0f, 0.0f,   1.0f, 0.0f, // bottom right
		-1.0f, -1.0f, 0.0f,   0.0f, 0.0f, 1.0f,   0.0f, 0.0f, // bottom left
		-1.0f,  1.0f, 0.0f,   1.0f, 1.0f, 0.0f,   0.0f, 1.0f  // top left 
	};

    unsigned int indices[] = {
       0, 1, 3, // first triangle
       1, 2, 3  // second triangle
    };
    
    glGenVertexArrays(1, &VAO);
    glGenBuffers(1, &VBO);
    glGenBuffers(1, &EBO);

    glBindVertexArray(VAO);

    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);

    // position attribute
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);
    // color attribute
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(3 * sizeof(float)));
    glEnableVertexAttribArray(1);
    // texture coord attribute
    glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(6 * sizeof(float)));
    glEnableVertexAttribArray(2);

    m_texture = LoadTexture("assets/container.jpg");

    // dimensions of the image
    
   
    glGenTextures(1, &tex_output);
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, tex_output);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA32F, tex_w, tex_h, 0, GL_RGBA, GL_FLOAT, NULL);
    glBindImageTexture(0, tex_output, 0, GL_FALSE, 0, GL_WRITE_ONLY, GL_RGBA32F);

    int work_grp_cnt[3];

    glGetIntegeri_v(GL_MAX_COMPUTE_WORK_GROUP_COUNT, 0, &work_grp_cnt[0]);
    glGetIntegeri_v(GL_MAX_COMPUTE_WORK_GROUP_COUNT, 1, &work_grp_cnt[1]);
    glGetIntegeri_v(GL_MAX_COMPUTE_WORK_GROUP_COUNT, 2, &work_grp_cnt[2]);

    printf("max global (total) work group counts x:%i y:%i z:%i\n", work_grp_cnt[0], work_grp_cnt[1], work_grp_cnt[2]);

    int work_grp_size[3];

    glGetIntegeri_v(GL_MAX_COMPUTE_WORK_GROUP_SIZE, 0, &work_grp_size[0]);
    glGetIntegeri_v(GL_MAX_COMPUTE_WORK_GROUP_SIZE, 1, &work_grp_size[1]);
    glGetIntegeri_v(GL_MAX_COMPUTE_WORK_GROUP_SIZE, 2, &work_grp_size[2]);

    printf("max local (in one shader) work group sizes x:%i y:%i z:%i\n",
        work_grp_size[0], work_grp_size[1], work_grp_size[2]);

    int work_grp_inv;

    glGetIntegerv(GL_MAX_COMPUTE_WORK_GROUP_INVOCATIONS, &work_grp_inv);
    printf("max local work group invocations %i\n", work_grp_inv);

    glm::mat4 transform = glm::mat4(0.0f);

    glm::vec3 pos = glm::vec3(1280.0f/2, 720.0f/2, 0.0f);

    transform = glm::translate(transform, pos);

    int vertexColorLocation = glGetUniformLocation(m_shader->GetRendererID(), "translate");
    glUseProgram(m_shader->GetRendererID());
    glUniformMatrix4fv(vertexColorLocation, 1, GL_FALSE, &transform[0][0]);
    
    
}

void SandboxLayer::OnDetach()
{
	// Shutdown here
}

void SandboxLayer::OnEvent(Event& event)
{
    EventDispatcher dispatcher(event);
    dispatcher.Dispatch<WindowResizeEvent>([&]{
        

        });




}

void SandboxLayer::OnUpdate(Timestep ts)
{

    glUseProgram(m_comp_shader);
    glDispatchCompute((GLuint)tex_w, (GLuint)tex_h, 1);

    // make sure writing to image has finished before read
    glMemoryBarrier(GL_SHADER_IMAGE_ACCESS_BARRIER_BIT);


    glClearColor(0.5f, 0.5f, 0.5f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT);

    glBindTexture(GL_TEXTURE_2D, m_texture.TextureID);

    glUseProgram(m_shader->GetRendererID());

    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, tex_output);
    glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);


    


}

void SandboxLayer::OnImGuiRender()
{
	// ImGui here
}
