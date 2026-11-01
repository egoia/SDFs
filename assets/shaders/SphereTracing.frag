
out vec4 FragColor;

uniform vec2 uResolution;
uniform vec3 camPos;
uniform vec4 backgroundColor;
uniform mat4 invMVP;

const int ITERATIONS = 100000; 
const float EPSILON = 0.0001;
const float MAX_DISTANCE = 100;
const  float AMBIANT_LIGHT = 0.5;
const vec3 light_dir = normalize(vec3(0,-1,-1));

float value(vec3 p); 

vec3 gradient(vec3 p){
    return vec3(
        value(p+vec3(1,0,0)*EPSILON) - value(p-vec3(1,0,0)*EPSILON),
        value(p+vec3(0,1,0)*EPSILON) - value(p-vec3(0,1,0)*EPSILON),
        value(p+vec3(0,0,1)*EPSILON) - value(p-vec3(0,0,1)*EPSILON)
        )/2*EPSILON;
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
    vec3 rayDir = normalize(rayStart - camPos);
    vec3 surfacePoint;
    if(sphere_tracing(rayStart, rayDir, surfacePoint)){
        vec3 n = normalize(gradient(surfacePoint));
        vec3 view = normalize(surfacePoint - camPos);
        //* blinn-phong
        float lighting = clamp( 0.6 * diffuse_light(n, light_dir) + AMBIANT_LIGHT + specular_light(n, light_dir, view, .20, 8.0),
            0.0, 1.0);
        FragColor = vec4(vec3(0.7, 0.12, 0.12) * lighting, 1.0);
    }
    else{
        FragColor = backgroundColor;
    }
}