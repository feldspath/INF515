#version 330 core

out vec4 FragColor;
in vec4 world_pos;

// Matrices
uniform mat4 model;
uniform mat4 projection_inverse;
uniform vec3 camera_center;
uniform vec3 light_pos;

// Sphere marching parameters
uniform float max_depth;
uniform int width;
uniform int height;
float intersection_threshold = 0.001;
float surface_threshold = intersection_threshold * 1.01;

// Material
vec3 color = vec3(1.0, 0.0, 0.0);
float diffuse = 0.5;
float ambient = 0.05;
float specular = 0.5;
float roughness = 16;

// sdf texture & parameters
uniform sampler3D sdf_texture;
uniform vec3 volume_origin;
uniform float volume_size;
uniform int nb_texels;
float voxel_size = volume_size / nb_texels;

float distance_estimate(vec3 position) {
    position = position + vec3(0.5f, 0.5, 0.5);
    vec3 tex_coord = (position - volume_origin) / volume_size;
    float value = texture(sdf_texture, position).r;
    return value * 8.0 - 4.0;
}

vec3 normal_estimate(vec3 p) {
    const float h = 0.3f;
    const vec2 k = vec2(1, -1);
    return normalize(
        k.xyy * distance_estimate(p + k.xyy * h) + k.yyx * distance_estimate(p + k.yyx * h) +
        k.yxy * distance_estimate(p + k.yxy * h) + k.xxx * distance_estimate(p + k.xxx * h));
}

// vec3 normal_estimate(vec3 position) {}

vec3 camera_direction(vec2 screen_pos) {
    return normalize(vec3(projection_inverse * vec4(screen_pos, -1.0f, 1.0f)));
}

vec2 sphere_intersection(vec3 start_pos, vec3 direction, float max_distance) {
    float distance;
    vec3 current_pos = start_pos;
    int iter = 0;
    do {
        distance = distance_estimate(current_pos);
        current_pos = current_pos + direction * distance;
        if (distance < intersection_threshold) {
            return vec2(length(current_pos - start_pos), iter);
        }
        iter++;
    } while (dot(current_pos - start_pos, current_pos - start_pos) < max_distance * max_distance &&
             iter < 100);
    return vec2(max_distance, iter);
}

vec3 project_on_sphere(vec3 pos) {
    return pos + normal_estimate(pos) * (surface_threshold - distance_estimate(pos));
}

void main() {
    vec3 direction =
        camera_direction(2 * vec2(-gl_FragCoord.x / width, -gl_FragCoord.y / height) + vec2(1.0f));
    vec2 intersect = sphere_intersection(camera_center, direction, 100.0f);
    float sphere_distance = intersect.x;
    if (sphere_distance >= max_depth) {
        FragColor = vec4(0.0f, 0.0f, 1.0f * intersect.y / 100, 1.0f);
        return;
    }
    vec3 sphere_intercept = camera_center + sphere_distance * direction;
    // vec3 sphere_intercept = project_on_sphere(camera_center + sphere_distance * direction);
    if (intersect.y < 10) {
        color = vec3(0.0f, 1.0f, 0.0f);
    }
    vec3 normal = normal_estimate(sphere_intercept);
    vec3 light_direction = normalize(light_pos - sphere_intercept);
    vec3 light_reflection = reflect(light_direction, normal);
    float light_distance = sphere_intersection(sphere_intercept, light_direction, 150.0f).x;
    vec3 diffuse_color = vec3(0.0);
    vec3 specular_color = vec3(0.0);
    vec3 ambient_color = ambient * color;
    if (light_distance > length(light_pos - sphere_intercept)) {
        diffuse_color = diffuse * max(dot(light_direction, normal), 0.0) * color;
        specular_color =
            specular * pow(max(dot(light_reflection, direction), 0.0f), roughness) * vec3(1.0f);
    }
    FragColor = vec4(diffuse_color + ambient_color + specular_color, 1.0f);
    // FragColor = vec4(1.0f, 0.0f, 0.0f, 1.0f);
}
