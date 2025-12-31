#version 430 core


layout (location = 0) in vec2 aPos;

layout(std430, binding = 1) buffer layoutName
{
    int data_SSBO[];
};

layout(std430, binding = 4) buffer debugsdf
{
    int data_debug[];
};


out vec2 v_LocalPosition;
uniform int u_GridSize;

float temp = 0.9f;

int i = int(gl_InstanceID);

vec2 cell = vec2(i % u_GridSize, floor(i / u_GridSize));


uniform mat4 u_ViewProjection;

flat out int Debug;
flat out int State;


void main()
{
	
	v_LocalPosition = aPos;
	vec2 position = (aPos + 1) / u_GridSize - 1;

	position = position + 2 * (cell / u_GridSize);

	State = data_SSBO[i];
	Debug = data_debug[i];

	gl_Position = u_ViewProjection * vec4(position, 0.0f, 1.0f);

}