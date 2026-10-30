#include <memory>
#include <random>
#include "mathematics.h"
#include "ray.h"
#include "SI/SDFs.hpp"
#include "SI/erosion.hpp"

Vector RandomDirection(std::mt19937& rng)
{
  // tire un double dans [-1, 1]
  std::uniform_real_distribution<double> uniform(-1.0, 1.0);

  //on tire un point dans le cube [-1,1]^3 jusqu'a ce qu'il tombe
  // dans la boule, puis on le projette sur la sphere en le normalisant.
  while (true)
  {
    Vector v(uniform(rng), uniform(rng), uniform(rng));


    double n2 = SquaredNorm(v);
    if(n2<1.0 && n2>1e-6) return Normalized(v);
  }
}

Ray RandomRay(const Vector& center, double radius, std::mt19937& rng)
{
  // u : direction aleatoire, donne la position de l'origine sur la sphere englobante
  Vector u = RandomDirection(rng);

  Vector origin = center + radius * u;
  return Ray(origin, -u);
}

std::unique_ptr<NodeSDF> MakeUnionTree(std::vector<std::unique_ptr<NodeSDF>>& nodes,
                                       int begin, int end)
{
  // Cas de base : un seul noeud dans [begin, end[
  if(end == begin+1) return std::move(nodes[begin]);


  int middle = (begin + end) / 2;

  // left  : union de la moitie gauche, vaut MakeUnionTree(nodes, begin, middle)
  // right : union de la moitie droite, vaut MakeUnionTree(nodes, middle, end)
  std::unique_ptr<NodeSDF> left = MakeUnionTree(nodes, begin, middle);
  std::unique_ptr<NodeSDF> right = MakeUnionTree(nodes, middle, end);

  return std::make_unique<UnionSDF>(left, right);
}

std::unique_ptr<NodeSDF> ErodeIncremental(std::unique_ptr<NodeSDF> scene, int nbImpacts,
                                          double impactRadius,
                                          const Vector& center, double radius,
                                          unsigned seed)
{
  std::mt19937 rng(seed);

  for (int i = 0; i < nbImpacts; i++)
  {
    //projectile aleatoire
    Ray ray = RandomRay(center, radius, rng);

    double t;
    if(scene->Intersect(ray, t)) {
      Vector impact = ray(t);
      std::unique_ptr<NodeSDF> crater = std::make_unique<SphereSDF>(impact, impactRadius);
      scene = std::make_unique<SubstractionSDF>(scene, crater);

    }
  }

  return scene;
}

std::unique_ptr<NodeSDF> ErodeBatch(std::unique_ptr<NodeSDF> scene, int nbImpacts,
                                    int batchSize, double impactRadius,
                                    const Vector& center, double radius,
                                    unsigned seed)
{
  std::mt19937 rng(seed);

  std::vector<std::unique_ptr<NodeSDF>> craters;

  for (int i = 0; i < nbImpacts; i++)
  {
    Ray ray = RandomRay(center, radius, rng);
    double t;
    if(scene->Intersect(ray, t)) {
      Vector impact = ray(t);
      craters.push_back(std::make_unique<SphereSDF>(impact, impactRadius));
    }

    if((i+1) % batchSize == 0 && !craters.empty()) {
      std::unique_ptr<NodeSDF> holes = MakeUnionTree(craters,0, craters.size());
      scene = std::make_unique<SubstractionSDF>(scene,holes);
      craters.clear();
    }
  }
  if(!craters.empty()) {
    std::unique_ptr<NodeSDF> holes = MakeUnionTree(craters,0, craters.size());
    scene = std::make_unique<SubstractionSDF>(scene,holes);
  }


  return scene;
}
