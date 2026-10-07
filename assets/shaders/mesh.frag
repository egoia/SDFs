
in vec3 vNormal;
out vec4 FragColor;

void main()
{
    vec3 normal = normalize(vNormal);
    vec3 lightDirection = normalize(vec3(-0.5, 0.8, 0.6));
    float diffuse = max(dot(normal, lightDirection), 0.0);
    vec3 color = vec3(0.12, 0.62, 0.86) * (0.2 + 0.8 * diffuse);
    FragColor = vec4(color, 1.0);
}
