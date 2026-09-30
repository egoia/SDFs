#include "orbit_camera.hpp"

void OrbitCamera::orbit(float valueX, float valueY){
    yaw += valueX * orbit_speed;
    pitch+= valueY * orbit_speed;
    glm::clamp(pitch, -89.0f, 89.f);

    float radius = distance(target_position, wrld_position);
    vec3 local_pos;
    local_pos.x = radius * cos(radians(pitch)) * cos(radians(yaw));
    local_pos.y = radius * sin(radians(pitch));
    local_pos.z = radius * sin(radians(yaw));
    wrld_position = local_pos + target_position;
}

mat4 OrbitCamera::getView()const{
    return glm::lookAt(wrld_position, target_position, glm::vec3(0.0f, 1.0f, 0.0f));
}

mat4 OrbitCamera::getProject()const{
    return glm::perspective(fov, WIDTH/HEIGHT, near, far);
}