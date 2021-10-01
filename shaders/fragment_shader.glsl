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

vec3 color = vec3(1.0, 0.0, 0.0);
float diffuse = 0.5;
float ambient = 0.05;
float specular = 0.5;
float roughness = 16;

uniform uint sdf_samples[128 * 125];
uniform vec3 volume_origin;
uniform float volume_size;
uniform int nb_blocks;

float block_size = volume_size / nb_blocks;
float voxel_size = block_size / 8;

ivec3 ijk_from_pos(vec3 position) {
    vec3 relative_pos = position - volume_origin;
    ivec3 ijk = ivec3(floor(relative_pos / voxel_size));
    return ijk;
}

float distance_estimate(vec3 position) {
    ivec3 ijk = ijk_from_pos(position);
    int i = ijk.x;
    int j = ijk.y;
    int k = ijk.z;

    ivec3 block_index = ijk / 8;
    int offset = (block_index.x * 5 * 5 + block_index.y * 5 + block_index.z) * 128;

    ijk = ijk % 8;
    uint temp = sdf_samples[offset + (i * 8 * 8 + j * 8 + k) / 4];
    float value = float((temp >> 8 * (k % 4)) & uint(0xff));
    return value / 32.0 - 4.0;
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
             iter < 100);
    return max_distance;
}

void main() {
    vec3 direction =
        camera_direction(2 * vec2(-gl_FragCoord.x / width, -gl_FragCoord.y / height) + vec2(1.0f));
    float sphere_distance = sphere_intersection(camera_center, direction, max_depth);
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
