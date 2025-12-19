#version 430

#define MAX_ITERATION 500

layout(local_size_x = 1, local_size_y = 1) in;
layout(rgba32f, binding = 0) uniform image2D img_output;

uniform mat4 translate;

vec2 resolution = vec2(1280, 720);

void main() {
  vec4 pixel = vec4(0.5, 0.5, 0.5, 1.0);
  ivec2 pixel_coord = ivec2(gl_GlobalInvocationID.xy);






  

  // output to a specific pixel in the image
  imageStore(img_output, pixel_coord, pixel);
}