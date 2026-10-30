


//* PRIMITIVES

float sphere_SDF(vec3 p, vec3 centre, float radius)
{
    return length(p-centre)-radius;
}

// segment [a,b] avec une epaisseur (capsule), radius = 0 pour un segment "pur"
float segment_SDF(vec3 p, vec3 a, vec3 b, float radius)
{
    vec3 pa = p-a, ba = b-a;
    float h = clamp( dot(pa,ba)/dot(ba,ba), 0.0, 1.0 );
    return length( pa - ba*h ) - radius;
}

// plan de normale n et de hauteur h le long de n (n doit etre normalise)
float plane_SDF(vec3 p, vec3 n, float h)
{
    return dot(p,n) + h;
}

// tore dans le plan xz, R = grand rayon, r = rayon du tube
float torus_SDF(vec3 p, vec3 centre, float R, float r)
{
    p -= centre;
    vec2 q = vec2(length(p.xz)-R, p.y);
    return length(q)-r;
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

//* DOMAIN OPERATORS (s'appliquent au point p avant l'evaluation de la SDF)

// repetition infinie de periode s : value(repetition_op(p, s))
vec3 repetition_op(vec3 p, vec3 s)
{
    return p - s*round(p/s);
}

// repetition limitee : l = nombre de repetitions de chaque cote, par axe
vec3 limited_repetition_op(vec3 p, float s, vec3 l)
{
    return p - s*clamp(round(p/s), -l, l);
}

// torsion autour de l'axe y, k = angle (radians) par unite de hauteur
// attention : SDF pas exacte, il faut reduire le pas du sphere tracing si k est grand
vec3 twist_op(vec3 p, float k)
{
    float c = cos(k*p.y);
    float s = sin(k*p.y);
    mat2  m = mat2(c,-s,s,c);
    return vec3(m*p.xz, p.y);
}

//* NOISE

float hash(vec3 p)
{
    p = fract( p*0.3183099 + 0.1 );
    p *= 17.0;
    return fract( p.x*p.y*p.z*(p.x+p.y+p.z) );
}

// value noise 3D dans [-1,1]
float noise(vec3 x)
{
    vec3 i = floor(x);
    vec3 f = fract(x);
    f = f*f*(3.0-2.0*f);

    return -1.0 + 2.0*mix(mix(mix( hash(i+vec3(0,0,0)), hash(i+vec3(1,0,0)), f.x),
                              mix( hash(i+vec3(0,1,0)), hash(i+vec3(1,1,0)), f.x), f.y),
                          mix(mix( hash(i+vec3(0,0,1)), hash(i+vec3(1,0,1)), f.x),
                              mix( hash(i+vec3(0,1,1)), hash(i+vec3(1,1,1)), f.x), f.y), f.z);
}

// deplacement de la surface par du bruit
// d = valeur de la SDF en p, amplitude = hauteur du bruit, frequency = echelle du bruit
// attention : la SDF n'est plus exacte, garder une amplitude faible ou reduire le pas
float noisify_op(float d, vec3 p, float amplitude, float frequency)
{
    return d + amplitude*noise(p*frequency);
}



//* RENDERING

float diffuse_light(vec3 normal, vec3 light){
    return max(0.0,dot(normal, -light));
}

float specular_light(vec3 normal, vec3 light, vec3 view, float strength, float shininess){
    vec3 r = light - 2 * dot(light, normal)*normal;
    return  strength * pow(max(0.0, dot(-view, r)), shininess);
}