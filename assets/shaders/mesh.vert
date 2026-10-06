#version 330 core

layout(location = 0) in vec3 aPosition;
layout(location = 1) in vec3 aNormal;

uniform mat4 uView;
uniform mat4 uProjection;

out vec3 vNormal;

void main()
{
    vNormal = aNormal;
    gl_Position = uProjection * uView * vec4(aPosition, 1.0);
}
