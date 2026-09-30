#version 330 core

out vec4 FragColor;

uniform vec2 uResolution;

const vec3 cameraPosition = vec3(0.0, 0.0, 3.0);
const float sphereRadius = 0.75;

float sceneDistance(vec3 point)
{
    return length(point) - sphereRadius;
}

vec3 estimateNormal(vec3 point)
{
    const float epsilon = 0.001;
    return normalize(vec3(
        sceneDistance(point + vec3(epsilon, 0.0, 0.0)) - sceneDistance(point - vec3(epsilon, 0.0, 0.0)),
        sceneDistance(point + vec3(0.0, epsilon, 0.0)) - sceneDistance(point - vec3(0.0, epsilon, 0.0)),
        sceneDistance(point + vec3(0.0, 0.0, epsilon)) - sceneDistance(point - vec3(0.0, 0.0, epsilon))
    ));
}

void main()
{
    vec2 screenPosition = (2.0 * gl_FragCoord.xy - uResolution) / uResolution.y;
    vec3 rayDirection = normalize(vec3(screenPosition * 0.41421356, -1.0));
    float travelDistance = 0.0;
    bool hit = false;
    vec3 hitPosition = vec3(0.0);

    for (int stepIndex = 0; stepIndex < 128; ++stepIndex)
    {
        vec3 point = cameraPosition + rayDirection * travelDistance;
        float distanceToScene = sceneDistance(point);

        if (distanceToScene < 0.001)
        {
            hit = true;
            hitPosition = point;
            break;
        }

        travelDistance += distanceToScene;
        if (travelDistance > 100.0)
        {
            break;
        }
    }

    if (!hit)
    {
        vec3 background = mix(vec3(0.015, 0.025, 0.04), vec3(0.08, 0.12, 0.17),
                              clamp(0.5 + screenPosition.y * 0.2, 0.0, 1.0));
        FragColor = vec4(background, 1.0);
        return;
    }

    vec3 normal = estimateNormal(hitPosition);
    vec3 lightDirection = normalize(vec3(-0.5, 0.8, 0.6));
    vec3 viewDirection = normalize(cameraPosition - hitPosition);
    float diffuse = max(dot(normal, lightDirection), 0.0);
    float specular = pow(max(dot(reflect(-lightDirection, normal), viewDirection), 0.0), 32.0);
    vec3 color = vec3(0.12, 0.62, 0.86) * (0.16 + 0.84 * diffuse) + vec3(0.5) * specular;

    FragColor = vec4(color, 1.0);
}