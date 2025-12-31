#version 440 core

in vec2 v_LocalPosition;

out vec4 FragColor;

uniform float u_BorderThickness;

uniform vec4 u_ClearColor;

void main()
{
    
     FragColor = u_ClearColor;

     //set border
     if (v_LocalPosition.x < -1 + u_BorderThickness || v_LocalPosition.x > 1 - u_BorderThickness ||
         v_LocalPosition.y < -1 + u_BorderThickness || v_LocalPosition.y > 1 - u_BorderThickness)
     {
        FragColor = vec4(0.0, 0.0, 0.0, 1.0);
     }

     else
     {  
        discard;
     }

}