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

float sphere_intersection(vec3 start_pos, vec3 direction, float max_distance) {
  float distance;
  vec3 current_pos = start_pos;
  int iter = 0;
  do {
      distance = sphere_sdf(current_pos);
      current_pos = current_pos + direction * distance;
      if (distance < threshold) {
          return length(current_pos - start_pos);
      }
      iter++;
  } while (dot(current_pos - start_pos, current_pos - start_pos) < max_distance*max_distance && iter < 100);
  return max_distance;
}

void main()
{
    vec3 direction = camera_direction(2* vec2(gl_FragCoord.x/width, gl_FragCoord.y/height) - 1);
    float sphere_distance = sphere_intersection(camera_center, direction, max_depth);
    if (sphere_distance < max_depth) {
      vec3 sphere_intercept = camera_center + sphere_distance * direction;
      float light_distance = sphere_intersection(sphere_intercept, normalize(light_pos - sphere_intercept), 10.0f);
      FragColor = vec4(light_distance/10.0f, 0.0f, 0.0f, 1.0f);
      return;
      if (light_distance > length(light_pos - sphere_intercept)) {
          FragColor = vec4(1.0, 0.0, 0.0, 1.0);
      } else {
          FragColor = vec4(0.0, 0.0, 0.0, 1.0);
      }
    } else {
      discard;
    }
}
