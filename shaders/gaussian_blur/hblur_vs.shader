#version 330 core

layout (location = 0) in vec3 position;
out vec2 blurTexCoords[11];

uniform float targetWidth;

void main(void) {
  gl_Position = vec4(position, 1.0);
  vec2 centerTexCoords = position.xy * 0.5 + 0.5;
  float pixelSize = 1.0 / targetWidth;

  for(int i = -5; i <= 5; i++) {
    blurTexCoords[i+5] = centerTexCoords + vec2(pixelSize * i, 0.0);
  }
}