


//* PRIMITIVES

float sphere_SDF(vec3 p, vec3 centre, float radius)
{
    return length(p-centre)-radius;
}


//* OPERATOR

float union_op(float v1, float v2){
    return min(v1,v2);
}

float intersection_op(float v1, float v2){
    return max(v1,v2);
}

float substract_op(float v1, float v2){
    return intersection_op(v1, -v2);
}

// quadratic polynomial
float smooth_union_op( float v1, float v2, float k )
{
    k *= 4.0;
    float h = max( k-abs(v1-v2), 0.0 )/k;
    return min(v1,v2) - h*h*k*(1.0/4.0);
}



//* RENDERING

float diffuse_light(vec3 normal, vec3 light){
    return max(0.0,dot(normal, -light));
}

float specular_light(vec3 normal, vec3 light, vec3 view, float strength, float shininess){
    vec3 r = light - 2 * dot(light, normal)*normal;
    return  strength * pow(max(0.0, dot(-view, r)), shininess);
}