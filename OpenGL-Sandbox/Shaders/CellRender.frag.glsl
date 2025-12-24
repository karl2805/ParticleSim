#version 440 core

in vec2 v_LocalPosition;

out vec4 FragColor;

float BorderWidth = 0.0f;

void main()
{
    
     FragColor = vec4(0.08, 0.33, 0.78, 1.0);

     //set border
     if (v_LocalPosition.x < -1 + BorderWidth || v_LocalPosition.x > 1 - BorderWidth ||
         v_LocalPosition.y < -1 + BorderWidth || v_LocalPosition.y > 1 - BorderWidth)
     {
        FragColor = vec4(0.0, 0.0, 0.0, 1.0);
     }

}