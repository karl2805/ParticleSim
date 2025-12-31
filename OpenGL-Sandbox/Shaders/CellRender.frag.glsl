#version 440 core



in vec2 v_LocalPosition;

out vec4 FragColor;

float BorderWidth = 0.0f;

flat in int State;
flat in int Debug;


void main()
{
    
    FragColor = vec4(0.5, 0.5, 0.5, 1.0);
     //set border
     if (v_LocalPosition.x < -1 + BorderWidth || v_LocalPosition.x > 1 - BorderWidth ||
         v_LocalPosition.y < -1 + BorderWidth || v_LocalPosition.y > 1 - BorderWidth)
     {
        FragColor = vec4(0.0, 0.0, 0.0, 1.0);
     }

     if (State == 1)
     {
        FragColor = vec4(0.0, 0.0, 1.0, 1.0);
     }

     if (Debug == 1)
     {
        FragColor = vec4(1.0, 0.0, 0.0, 1.0);
     }
     if (Debug == 2)
     {
        FragColor = vec4(0.0, 1.0, 0.0, 1.0);
     }

     

}