#version 330 core

out vec4 FragColor;

uniform vec2 uResolution;
uniform vec3 camPos;
uniform vec4 backgroundColor;
uniform mat4 invMVP;

const int ITERATIONS = 100000; 
const float EPSILON = 0.01;
const float MAX_DISTANCE = 100;

float sphere_radius = 0.50;
vec3 sphere_centre = vec3(0);

float sphere_SDF(vec3 p, vec3 centre, float radius)
{
    return length(p-centre)-radius;
}

float value(vec3 p)
{
    return sphere_SDF(p,sphere_centre,sphere_radius);
}

bool sphere_tracing(vec3 rayStart, vec3 rayDir, out vec3 surfacePoint){
    float distance = 0;
    for(int i = 0; i<ITERATIONS; i++){
        vec3 p = rayStart + rayDir * distance;
        float sample = value(p); //Sphere
        if(sample < EPSILON || distance > MAX_DISTANCE) break;
        distance += sample; //Move along sphere
    } 
    surfacePoint = rayStart + rayDir * distance;
    return distance < MAX_DISTANCE;
}

void main()
{
    vec2 positionClip = (gl_FragCoord.xy / uResolution) * 2.0 - 1.0;
    vec4 positionWorld = invMVP * vec4(positionClip, -1.0,1.0);
    vec3 rayStart = positionWorld.xyz / positionWorld.w;
    vec3 rayDir = rayStart - camPos;
    vec3 surfacePoint;
    if(sphere_tracing(rayStart, rayDir, surfacePoint)){
        FragColor = vec4(1.0,0.0,0.0, 1.0);
    }
    else{
        FragColor = backgroundColor;
    }

}