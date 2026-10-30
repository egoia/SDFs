// Erosion d'objets SDF par projection de spheres

#pragma once

#include <memory>
#include <random>
#include <vector>
#include "mathematics.h"
#include "SI/SDFs.hpp"
#include "ray.h"

// Direction aleatoire uniforme sur la sphere.
Vector RandomDirection(std::mt19937& rng);

// Rayon "projectile" : origine sur la sphere englobante (center, radius) de l'objet,
// direction vers center.
Ray RandomRay(const Vector& center, double radius, std::mt19937& rng);

// Arbre d'union des noeuds nodes[begin, end[ .
std::unique_ptr<NodeSDF> MakeUnionTree(std::vector<std::unique_ptr<NodeSDF>>& nodes,
                                       int begin, int end);

// Erosion incrementale : chaque sphere est soustraite des son impact,
// le rayon suivant voit donc deja les crateres precedents.
// scene        : objet a eroder (on en prend la propriete)
// nbImpacts    : nombre de rayons lances
// impactRadius : rayon des spheres retirees
// center       : centre de la sphere englobante de l'objet
// radius       : rayon de la sphere englobante de l'objet
// seed         : graine, pour comparer les deux methodes sur les memes tirages
// retourne Sub(Sub(Sub(scene, s0), s1), s2)...
std::unique_ptr<NodeSDF> ErodeIncremental(std::unique_ptr<NodeSDF> scene, int nbImpacts,
                                          double impactRadius,
                                          const Vector& center, double radius,
                                          unsigned seed);

// Erosion par paquets : batchSize rayons sont lances sur la scene telle qu'elle est,
// les spheres obtenues sont reunies avec MakeUnionTree puis soustraites en une fois.
// Les rayons d'un meme paquet ne voient pas les crateres les uns des autres.
// batchSize    : nombre de rayons par paquet
// (autres parametres identiques a ErodeIncremental)
// retourne Sub(Sub(scene, Union(paquet0)), Union(paquet1))...
std::unique_ptr<NodeSDF> ErodeBatch(std::unique_ptr<NodeSDF> scene, int nbImpacts,
                                    int batchSize, double impactRadius,
                                    const Vector& center, double radius,
                                    unsigned seed);
