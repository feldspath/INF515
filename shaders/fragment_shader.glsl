#version 330 core

out vec4 FragColor;

uniform mat4 model;
uniform mat4 projection_inverse;
uniform vec3 camera_center;
uniform vec3 light_pos;
// uniform float sphere_radius;
uniform float max_depth;
uniform int width;
uniform int height;

float intersection_threshold = 0.001f;
float surface_threshold = intersection_threshold * 1.01;
vec3 sphere_center = vec3(model * vec4(0.0f, 0.0f, 0.0f, 1.0f));
float sphere_radius = 0.5f;
vec2 torus_params = vec2(0.3f, 0.1f);

vec3 color = vec3(1.0, 0.0, 0.0);
float diffuse = 0.5;
float ambient = 0.05;
float specular = 0.5;
float roughness = 16;

vec3 camera_direction(vec2 screen_pos) {
    return normalize(vec3(projection_inverse * vec4(screen_pos, -1.0f, 1.0f)));
}

float sphere_sdf(vec3 pos) { return length(pos - sphere_center) - sphere_radius; }

vec3 sphere_normal(vec3 pos) { return normalize(pos - sphere_center); }

float torus_sdf(vec3 pos) {
    vec2 q = vec2(length(pos.xz - sphere_center.xz) - torus_params.x, pos.y - sphere_center.y);
    return length(q) - torus_params.y;
}

float sphere_intersection(vec3 start_pos, vec3 direction, float max_distance) {
    float distance;
    vec3 current_pos = start_pos;
    int iter = 0;
    do {
        distance = sphere_sdf(current_pos);
        current_pos = current_pos + direction * distance;
        if (distance < intersection_threshold) {
            return length(current_pos - start_pos);
        }
        iter++;
    } while (dot(current_pos - start_pos, current_pos - start_pos) < max_distance * max_distance &&
             iter < 100);
    return max_distance;
}

vec3 project_on_sphere(vec3 pos) {
    return pos + sphere_normal(pos) * (surface_threshold - sphere_sdf(pos));
}

void main() {
    vec3 direction =
        camera_direction(2 * vec2(-gl_FragCoord.x / width, -gl_FragCoord.y / height) + vec2(1.0f));
    float sphere_distance = sphere_intersection(camera_center, direction, max_depth);
    if (sphere_distance >= max_depth) {
        discard;
    }
    vec3 sphere_intercept = project_on_sphere(camera_center + sphere_distance * direction);
    vec3 normal = sphere_normal(sphere_intercept);
    vec3 light_direction = normalize(light_pos - sphere_intercept);
    vec3 light_reflection = reflect(light_direction, normal);
    float light_distance = sphere_intersection(sphere_intercept, light_direction, 150.0f);
    vec3 diffuse_color = vec3(0.0);
    vec3 specular_color = vec3(0.0);
    vec3 ambient_color = ambient * color;
    if (light_distance > length(light_pos - sphere_intercept)) {
        diffuse_color = diffuse * dot(light_direction, normal) * color;
        specular_color =
            specular * pow(max(dot(light_reflection, direction), 0.0f), roughness) * vec3(1.0f);
    }
    FragColor = vec4(diffuse_color + ambient_color + specular_color, 1.0f);
}
