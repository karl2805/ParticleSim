#version 430

layout(local_size_x = 8, local_size_y = 8) in;

layout(binding = 1, std430) readonly buffer ssbo_in
{
    int State_in[];
};

layout(binding = 2, std430) buffer ssbo_out
{
    int State_out[];
};

uniform int u_GridSize;

int CellIndex(vec2 CellCoord)
{
   return (int(CellCoord.y) * u_GridSize + int(CellCoord.x));
}

int cellActive(int x, int y)
{
    return int(State_in[CellIndex(vec2(x,y))]);
}


void main() 
{
    /*if (State_in[CellIndex(gl_GlobalInvocationID.xy)] == 1)
    {
        State_out[CellIndex(gl_GlobalInvocationID.xy)] = 0;
    }

    else
    {
        State_out[CellIndex(gl_GlobalInvocationID.xy)] = 1;
    }*/


    State_out[CellIndex(gl_GlobalInvocationID.xy)] = State_in[CellIndex(vec2(gl_GlobalInvocationID.x, gl_GlobalInvocationID.y + 1))];
 
  
}