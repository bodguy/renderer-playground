#version 330 core

in vec2 textureCoords;
out vec4 out_Colour;

uniform sampler2D colourTexture;

void main(void) {
    vec4 colour = texture(colourTexture, textureCoords);
    float brightness = dot(colour.rgb, vec3(0.2126, 0.7152, 0.0722));
    out_Colour = colour * brightness;
}