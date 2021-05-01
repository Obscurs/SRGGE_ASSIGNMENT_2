#version 330

layout (location = 0) in vec3 vert;
layout (location = 1) in vec2 texCoords;

uniform mat4 projection;
uniform mat4 view;
uniform mat4 model;
uniform vec3 offsets[1000];
out vec2 TexCoords;

void main(void)  {
  TexCoords = texCoords;
  vec3 offset = offsets[gl_InstanceID];
  vec3 Position = vec3(model * vec4(vert+offset, 1.0));
  gl_Position = projection * view * vec4(Position, 1.0);
}
