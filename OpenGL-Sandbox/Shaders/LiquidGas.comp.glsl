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
uniform float u_Temperature;
uniform float u_ChemicalPotential;
uniform float u_Time;
uniform int u_Step;

int CellIndex(vec2 CellCoord)
{
   return int(CellCoord.y) * u_GridSize + int(CellCoord.x);
}

int SpinState(int x, int y)
{
    int array_value = int(State_in[CellIndex(vec2(x,y))]);

    return array_value == 1 ? 1 : -1;
}

int ParticlePresent(int x, int y)
{
    return int(State_in[CellIndex(vec2(x,y))]);
}


uint hash(uint x)
{
    x ^= x >> 16;
    x *= 0x7feb352d;
    x ^= x >> 15;
    x *= 0x846ca68b;
    x ^= x >> 16;
    return x;
}

float rand(uint seed)
{
    return float(hash(seed)) / float(0xffffffffu);
}

void main() 
{

vec2 cell = gl_GlobalInvocationID.xy;

int i = CellIndex(cell);

//float random = rand(dot(cell, vec2(12.9898, 78.233)) + u_Time);
//float random2 = rand(dot(cell, vec2(12.9898, 78.233)) + u_Time);


uint seed = uint(cell.x) + uint(cell.y) * uint(u_GridSize) + u_Step * 1664525u;

float random = rand(seed);



int energy_surrouding = SpinState(int(cell.x+1), int(cell.y+1)) +
                        SpinState(int(cell.x+1), int(cell.y)) +
                        SpinState(int(cell.x+1), int(cell.y-1)) +
                        SpinState(int(cell.x), int(cell.y-1)) +
                        SpinState(int(cell.x-1), int(cell.y-1)) +
                        SpinState(int(cell.x-1), int(cell.y)) +
                        SpinState(int(cell.x-1), int(cell.y+1)) +
                        SpinState(int(cell.x), int(cell.y+1));

int particles_surrounding = ParticlePresent(int(cell.x+1), int(cell.y+1)) +
                        ParticlePresent(int(cell.x+1), int(cell.y)) +
                        ParticlePresent(int(cell.x+1), int(cell.y-1)) +
                        ParticlePresent(int(cell.x), int(cell.y-1)) +
                        ParticlePresent(int(cell.x-1), int(cell.y-1)) +
                        ParticlePresent(int(cell.x-1), int(cell.y)) +
                        ParticlePresent(int(cell.x-1), int(cell.y+1)) +
                        ParticlePresent(int(cell.x), int(cell.y+1));

float energy = 2.0f * float(SpinState(int(cell.x), int(cell.y))) * float(energy_surrouding);

//float q = exp((energy - u_ChemicalPotential * float(particles_surrounding)) / u_Temperature);

//float probability = 1.0f / (1.0f + exp((float(energy - u_ChemicalPotential * float(particles_surrounding))) / float(u_Temperature)));
float probability = 1.0f / (1.0f + exp((float(energy)) / float(u_Temperature)));


if (random < probability ) 
{
    State_out[i] = 1 - State_in[i];
}

else 
{
    State_out[i] = State_in[i];
}




  
}