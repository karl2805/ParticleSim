#version 430 core


layout (location = 0) in vec2 aPos;

layout(std430, binding = 3) buffer layoutName
{
    int data_SSBO[];
};


out vec2 v_LocalPosition;
uniform int u_GridSize;

float temp = 0.9f;

int i = int(gl_InstanceID);

vec2 cell = vec2(i % u_GridSize, floor(i / u_GridSize));

int State = data_SSBO[gl_InstanceID];

uniform mat4 u_ViewProjection;




void main()
{
	
	v_LocalPosition = aPos;
	vec2 position = (aPos + 1) / u_GridSize - 1;

	position = position * State + 2 * (cell / u_GridSize);

	gl_Position = u_ViewProjection * vec4(position, 0.0f, 1.0f);

}