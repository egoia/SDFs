#include "orbit_camera.hpp"

void OrbitCamera::orbit(float dX, float dY){
    yaw += dX * orbit_speed;
    pitch+= dY * orbit_speed;
    pitch = glm::clamp(pitch, -89.0f, 89.f);

    float radius = distance(target_position, wrld_position);
    vec3 local_pos;
    local_pos.x = radius * cos(radians(pitch)) * cos(radians(yaw));
    local_pos.y = radius * sin(radians(pitch));
    local_pos.z = radius * sin(radians(yaw)) * cos(radians(pitch));
    wrld_position = local_pos + target_position;
}

void OrbitCamera::zoom(float wheelDelta){
    const float radius = glm::distance(target_position, wrld_position);
    const float newRadius = glm::clamp(radius * std::pow(0.9f, wheelDelta), 1.25f, 50.0f);
    const vec3 direction = glm::normalize(wrld_position - target_position);
    wrld_position = target_position + direction * newRadius;
}

mat4 OrbitCamera::getView()const{
    return glm::lookAt(wrld_position, target_position, glm::vec3(0.0f, 1.0f, 0.0f));
}

mat4 OrbitCamera::getProject()const{
    return glm::perspective(fov, WIDTH/HEIGHT, near, far);
}