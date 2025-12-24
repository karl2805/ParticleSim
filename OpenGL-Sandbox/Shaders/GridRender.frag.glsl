#version 440 core

in vec2 v_LocalPosition;

out vec4 FragColor;

float BorderWidth = 0.1f;

uniform vec4 u_ClearColor;

void main()
{
    
     FragColor = u_ClearColor;

     //set border
     if (v_LocalPosition.x < -1 + BorderWidth || v_LocalPosition.x > 1 - BorderWidth ||
         v_LocalPosition.y < -1 + BorderWidth || v_LocalPosition.y > 1 - BorderWidth)
     {
        FragColor = vec4(0.0, 0.0, 0.0, 1.0);
     }

     else
     {  
        discard;
     }

}