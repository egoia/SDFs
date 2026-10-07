# Install

`sudo apt install libsdl2-dev libglm-dev libgl-dev`

# Build

Configurer et compiler le pipeline Marching Cubes :

```bash
cmake --preset marching-cube
cmake --build --preset marching-cube
```

L'executable est genere dans `build/build-marching-cube/marching-cube`.

Configurer et compiler le pipeline Sphere Tracing :

```bash
cmake --preset sphere-tracing
cmake --build --preset sphere-tracing
```

L'executable est genere dans `build/build-sphere-tracing/sphere-tracing`.

# Deux pipelines
- sphere tracing
- marching cube

# Primitives
- Sphere (Leon)
- Box (Elyas)
- Segment (Capsule)
- Courbe (bonus)
- Cylindre
- Plane
- Donut
- Cone
- Bulle

# Operators 
- Union (Leon)
- Intersection (Elyas)
- Soustraction
- Union Smooth
- Rotation
- Translation 
- Scale
- Domain Repetition 
- Limited Domain Repetition
- Displacement
- Twist
- Bend
- Noisify

# Ideas
- aplatir arbres dans GPU
- Z buffer avec plusieurs draw call pour differents material types
- compute normal not with the derivative but with normal of each primitive combined

# Optimization
- gradient avec 4 samples au lieu de 6
- BVH
- réplications en cercle et sphere avec les coordonnées sphérique 
- replication cylindrique 
- replication le long d'une courbe ? (plein de donut)

# Rendering 
- BRDF 
- BSDF ? 
- bubble
- water
- lava
- cloud

