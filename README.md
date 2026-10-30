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

Configurer et compiler l'erosion (benchmark + visualisation, toujours en Release) :

```bash
cmake --preset erosion-benchmark
cmake --build --preset erosion-benchmark
cd build/build-erosion-benchmark
./erosion-benchmark   # compare erosion incrementale / par paquets
./erosion-viewer      # affiche les deux resultats cote a cote (gauche : incremental, droite : paquets)
```

Les executables doivent etre lances depuis leur dossier de build (shaders charges en chemin relatif `assets/...`).

`source/ray_marching.cpp` contient un `main` et est exclu de `COMMON_SOURCES` dans le `CMakeLists.txt`.

# Erosion (vieillissement d'objets)

Objectif : simuler le vieillissement d'un objet SDF en projetant des spheres qui creusent localement sa surface, puis comparer une erosion incrementale et une erosion par paquets.

## Fichiers

| Fichier | Contenu |
|---|---|
| `include/SI/SDFs.hpp`, `source/SI/SDF.cpp` | `NodeSDF::Intersect` : intersection rayon / SDF par sphere tracing |
| `include/SI/erosion.hpp`, `source/SI/erosion.cpp` | algorithme d'erosion (aucun code de mesure ou d'affichage) |
| `source/erosion_benchmark.cpp` | mesures de performance |
| `source/erosion_viewer.cpp` | visualisation des deux erosions par marching cubes |

## Intersection : `bool NodeSDF::Intersect(const Ray& ray, double& t, double tMax = 100, double eps = 1e-4, int maxSteps = 256) const`

Sphere tracing sur l'arbre C++ (le sphere tracing de `sphere_tracing.cpp` est cote GLSL). On part de `t = 0` et on avance de `d = Value(ray(t))` jusqu'a `d < eps` (impact, `ray(t)` est sur la surface) ou `t > tMax` (rate). La direction du rayon doit etre normalisee. Defini dans `NodeSDF`, donc disponible pour toutes les primitives et tous les operateurs.

## Erosion

- `RandomDirection` : direction uniforme sur la sphere unite (tirage par rejet dans le cube).
- `RandomRay(center, radius, rng)` : origine aleatoire sur la sphere englobante de l'objet, direction vers son centre.
- `MakeUnionTree(nodes, begin, end)` : arbre d'union equilibre (profondeur log2(n)) d'une liste de noeuds.
- `ErodeIncremental(scene, nbImpacts, impactRadius, center, radius, seed)` : chaque impact est soustrait immediatement, le rayon suivant voit donc les crateres precedents. Arbre produit : `Sub(Sub(Sub(objet, s0), s1), s2)...`
- `ErodeBatch(scene, nbImpacts, batchSize, impactRadius, center, radius, seed)` : les `batchSize` rayons d'un paquet sont lances sur la meme scene, leurs crateres sont reunis par `MakeUnionTree` puis soustraits en une fois. Arbre produit : `Sub(Sub(objet, Union(paquet0)), Union(paquet1))...`. Approximation : les rayons d'un meme paquet ne voient pas les crateres les uns des autres.

Les deux methodes tirent les memes rayons pour une meme `seed`. Les noeuds sont des `std::unique_ptr<NodeSDF>` (comme le reste du projet) : stocker les noeuds par valeur casserait le polymorphisme de `Value` (slicing).

## Resultats (boite de demi-cotes 0.5, cratere de rayon 0.05, Release)

| impacts | incremental : construction / `Value` | paquets de 100 : construction / `Value` |
|---|---|---|
| 100 | 0.27 ms / 0.98 us | 0.03 ms / 0.36 us |
| 500 | 7.7 ms / 4.2 us | 2.5 ms / 1.6 us |
| 1000 | 40 ms / 8.5 us | 13.6 ms / 3.3 us |
| 2000 | 209 ms / 17 us | 75 ms / 6.5 us |

- Construction quadratique dans les deux cas : chaque `Intersect` parcourt un arbre contenant tous les crateres precedents.
- Cout de `Value` lineaire en n dans les deux cas (l'union equilibree a une profondeur log k mais toujours k feuilles) ; les paquets gagnent un facteur constant ~2.5.
- Les sommes de `Value` (checksum) different d'environ 0.1 % : l'approximation des paquets change peu la forme.
- Piste : elaguer avec des volumes englobants (BVH) pour sortir du lineaire.

## Recuperer l'arbre erode pour l'aplatir (export GLSL)

L'erosion se fait cote CPU (elle a besoin de `Intersect`), le resultat est un arbre C++ classique :

```cpp
std::unique_ptr<NodeSDF> root = ErodeBatch(std::move(objet), nbImpacts, batchSize,
                                           impactRadius, center, radius, seed);
// root est la racine a donner a l'algorithme d'aplatissement
```

Ce que l'aplatissement doit pouvoir lire sur cet arbre :

1. **La racine** : le `std::unique_ptr<NodeSDF>` renvoye par `ErodeIncremental` / `ErodeBatch` (il garde la propriete de tout l'arbre, il doit rester en vie pendant l'aplatissement).
2. **Le type de chaque noeud**. L'erosion ne cree que trois types en plus de l'objet de depart :
   - `SubstractionSDF` (un par impact en incremental, un par paquet sinon),
   - `UnionSDF` (interieur des paquets),
   - `SphereSDF` (les crateres).

   Identification par `dynamic_cast` ou par une methode virtuelle de type a ajouter a `NodeSDF`.
3. **Les enfants des operateurs**, dans l'ordre : `SubstractionSDF` n'est pas commutatif, il faut distinguer `minuend` (objet) et `subtrahend` (crateres). Ces membres sont `private` : ajouter des accesseurs const (ex. `const NodeSDF& Minuend() const`, `Left()`, `Right()`) ou declarer l'aplatisseur `friend`.
4. **Les parametres des feuilles** : `center` et `radius` pour `SphereSDF` (identiques pour tous les crateres : `impactRadius`), plus ceux de l'objet de depart (ex. `BoxSDF` : `center`, `size`, `up`). Memes accesseurs a prevoir.

Tailles a prevoir cote GLSL pour n impacts touches et des paquets de k :

| | incremental | paquets |
|---|---|---|
| spheres | n | n |
| `SubstractionSDF` | n | ~n/k |
| `UnionSDF` | 0 | ~n - n/k |
| profondeur de l'arbre | n + 1 | ~n/k + log2(k) + 1 |
| pile d'evaluation postfixe | 2 | ~log2(k) + 2 |

L'arbre incremental est un peigne gauche : tres profond mais evaluable avec une pile de 2 en postfixe. Le nombre de noeuds (~2n) peut depasser les limites de taille des tableaux uniformes : passer par un SSBO ou une texture au-dela de quelques centaines d'impacts.

# Deux pipelines
- sphere tracing
- marching cube

# Primitives
- Sphere (Leon)
- Box (Elyas)
- Segment (Capsule)
- Cylindre
- Plane
- Cone
- Donut (optionnel)

# Operators 
- Union (Leon)
- Intersection (Elyas)
- Soustraction
- Union Smooth
- Domain Repetition 
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
- Antialiasing

