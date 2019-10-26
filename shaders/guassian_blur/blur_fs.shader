#version 330 core

out vec4 out_colour;
in vec2 blurTexCoords[11];

uniform sampler2D originalTexture;

void main(void) {
  out_colour = vec4(0.0);
  out_colour += texture(originalTexture, blurTexCoords[0]) * 0.0093;
  out_colour += texture(originalTexture, blurTexCoords[1]) * 0.028002;
  out_colour += texture(originalTexture, blurTexCoords[2]) * 0.065984;
  out_colour += texture(originalTexture, blurTexCoords[3]) * 0.121703;
  out_colour += texture(originalTexture, blurTexCoords[4]) * 0.175713;
  out_colour += texture(originalTexture, blurTexCoords[5]) * 0.198596;
  out_colour += texture(originalTexture, blurTexCoords[6]) * 0.175713;
  out_colour += texture(originalTexture, blurTexCoords[7]) * 0.121703;
  out_colour += texture(originalTexture, blurTexCoords[8]) * 0.065984;
  out_colour += texture(originalTexture, blurTexCoords[9]) * 0.028002;
  out_colour += texture(originalTexture, blurTexCoords[10]) * 0.0093;
}