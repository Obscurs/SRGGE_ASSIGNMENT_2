#version 330

out vec4 FragColor;
in vec2 TexCoords;

uniform sampler2D texture1;

void main (void) {
 FragColor = texture(texture1, TexCoords);
 //FragColor = vec4(1.0,0.0,0.0,1.0);
}
