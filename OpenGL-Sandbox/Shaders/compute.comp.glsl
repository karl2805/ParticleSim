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
   return int(CellCoord.y) * u_GridSize + int(CellCoord.x);
}

int cellActive(int x, int y)
{
    return int(State_in[CellIndex(vec2(x,y))]);
}


void main() {

 vec2 cell = gl_GlobalInvocationID.xy;

 int activeNeighbors = cellActive(int(cell.x+1), int(cell.y+1)) +
                        cellActive(int(cell.x+1), int(cell.y)) +
                        cellActive(int(cell.x+1), int(cell.y-1)) +
                        cellActive(int(cell.x), int(cell.y-1)) +
                        cellActive(int(cell.x-1), int(cell.y-1)) +
                        cellActive(int(cell.x-1), int(cell.y)) +
                        cellActive(int(cell.x-1), int(cell.y+1)) +
                        cellActive(int(cell.x), int(cell.y+1));

int i = CellIndex(cell.xy);

switch (activeNeighbors) {
  case 2: { // Active cells with 2 neighbors stay active.
    State_out[i] = State_in[i];
    break;
  }
  case 3: { // Cells with 3 neighbors become or stay active.
    State_out[i] = 1;
    break;
  }
    default: { // Cells with < 2 or > 3 neighbors become inactive.
    State_out[i] = 0;
    }

   
  }

  
}