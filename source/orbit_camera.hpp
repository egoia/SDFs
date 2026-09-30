#pragma once
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glad/glad.h>
using namespace glm;

class orbit_camera {
    public : 
        vec3 wrld_position;
        vec3 target_position;

        float yaw;//y
        float pitch;//x
        float orbit_speed;

        float fov;
        float near; 
        float far;

        const float WIDTH;
        const float HEIGHT;

        orbit_camera(const vec3& target_position, float distance, float fov, float near, float far, float width, float height) : 
            target_position(target_position), wrld_position(target_position+vec3(0,0,distance)), yaw(0), pitch(0), fov(radians(fov)), near(near), far(far), WIDTH(width), HEIGHT(height){};
        
        mat4 getView() const;
        mat4 getProject() const;

        void orbit(float valueX, float valueY);
        
};