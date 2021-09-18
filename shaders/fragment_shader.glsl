#version 330 core

out vec4 FragColor;

uniform mat4 projection_inverse;
uniform vec3 camera_center;
uniform vec3 light_pos;
// uniform vec3 sphere_center;
// uniform float sphere_radius;
uniform float max_depth;
uniform int width;
uniform int height;

float threshold = 0.01f;
vec3 sphere_center = vec3(0.0f, 0.0f, -5.0f);
float sphere_radius = 0.5f;

vec3 camera_direction(vec2 screen_pos) {
    return normalize(vec3(projection_inverse*vec4(screen_pos, -1.0f, 1.0f)));
}

float sphere_sdf(vec3 pos) {
    return length(pos - sphere_center) - sphere_radius;
}

void main()
{
    vec3 direction = camera_direction(2* vec2(gl_FragCoord.x/width, gl_FragCoord.y/height) - 1);
    float distance;
    vec3 current_pos = camera_center;
    int iter = 0;

    do {
        distance = sphere_sdf(current_pos);
        current_pos = current_pos + direction * distance;
        if (distance < threshold) {
            FragColor = vec4(1.0, 0.0, 0.0, 1.0);
            return;
        }
        iter++;
    } while (dot(current_pos - camera_center, current_pos - camera_center) < max_depth*max_depth && iter < 100);
    discard;
}
