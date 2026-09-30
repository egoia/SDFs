
#ifdef VERTEX_SHADER
void main()
{
    vec3 positions[3] = vec3[3](
        vec3(-0.5, -0.5, 0.0),
        vec3( 0.5, -0.5, 0.0),
        vec3( 0.0,  0.5, 0.0)
    );
    
    // vec4 attendu : on passe 1.0 pour la composante w
    gl_Position = vec4(positions[gl_VertexID], 1.0);
}
#endif

#ifdef FRAGMENT_SHADER
out vec4 FragColor;

void main()
{
    FragColor = vec4(0.8, 0.4, 0.0, 1.0);
}
#endif