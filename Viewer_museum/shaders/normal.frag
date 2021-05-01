#version 330

smooth in vec3 Normal;
vec3 color = vec3(0.6);

out vec4 frag_color;

void main (void) {
 vec3 N = normalize(Normal);

 // write Total Color:
 frag_color = vec4(N, 1.0);
}
