#version 330 core

layout(location = 0) in vec3 position;

uniform mat4 projection;
uniform mat4 view;
uniform mat4 model;

out vec4 world_pos;

void main() {
    world_pos = model * vec4(position, 1.0);
    gl_Position = projection * view * world_pos;
}
