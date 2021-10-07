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
float intersection_threshold = 0.2;
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
    vec3 tex_coord = (position - volume_origin) / volume_size;
    float value = texture(sdf_texture, position).r;
    return value * 8.0 - 4.0;
}

// vec3 normal_estimate(vec3 position) {}

vec3 camera_direction(vec2 screen_pos) {
    return normalize(vec3(projection_inverse * vec4(screen_pos, -1.0f, 1.0f)));
}

float sphere_intersection(vec3 start_pos, vec3 direction, float max_distance) {
    float distance;
    vec3 current_pos = start_pos;
    int iter = 0;
    do {
        distance = distance_estimate(current_pos);
        current_pos = current_pos + direction * distance;
        if (distance < intersection_threshold) {
            return length(current_pos - start_pos);
        }
        iter++;
    } while (dot(current_pos - start_pos, current_pos - start_pos) < max_distance * max_distance &&
             iter < 300);
    return max_distance;
}

void main() {
    vec3 direction =
        camera_direction(2 * vec2(-gl_FragCoord.x / width, -gl_FragCoord.y / height) + vec2(1.0f));
    float sphere_distance = sphere_intersection(camera_center, direction, max_depth);
    float sample = texture(sdf_texture, vec3(world_pos.x + 0.5f, world_pos.y + 0.5f, 0.5f)).r;
    vec3 current_pos = camera_center;
    float dist;
    for (int i = 0; i < 0; i++) {
        dist = distance_estimate(current_pos);
        current_pos = current_pos + direction * dist;
    }
    dist = distance_estimate(current_pos);
    FragColor = vec4(dist - 0.55, 0.0f, 0.0f, 1.0f);
    return;
    if (sphere_distance >= max_depth) {
        discard;
    }
    // vec3 sphere_intercept = project_on_sphere(camera_center + sphere_distance * direction);
    // vec3 normal = torus_normal(sphere_intercept);
    // vec3 light_direction = normalize(light_pos - sphere_intercept);
    // vec3 light_reflection = reflect(light_direction, normal);
    // float light_distance = sphere_intersection(sphere_intercept, light_direction, 150.0f);
    // vec3 diffuse_color = vec3(0.0);
    // vec3 specular_color = vec3(0.0);
    // vec3 ambient_color = ambient * color;
    // if (light_distance > length(light_pos - sphere_intercept)) {
    //     diffuse_color = diffuse * dot(light_direction, normal) * color;
    //     specular_color =
    //         specular * pow(max(dot(light_reflection, direction), 0.0f), roughness) * vec3(1.0f);
    // }
    // FragColor = vec4(diffuse_color + ambient_color + specular_color, 1.0f);
    FragColor = vec4(1.0f, 0.0f, 0.0f, 1.0f);
}
