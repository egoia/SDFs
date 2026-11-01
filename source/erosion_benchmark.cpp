// Comparaison des performances : erosion incrementale vs erosion par paquets
// A compiler en Release, sinon les temps mesures ne veulent rien dire.

#include <chrono>
#include <iostream>
#include <memory>
#include <random>
#include "mathematics.h"
#include "SI/SDFs.hpp"
#include "SI/erosion.hpp"

template <typename F>
double TimeMs(F&& f)
{
  auto start = std::chrono::steady_clock::now();

  f();

  auto end = std::chrono::steady_clock::now();

  return std::chrono::duration<double, std::milli>(end - start).count();
}

std::unique_ptr<NodeSDF> MakeTestScene()
{
  return std::make_unique<BoxSDF>(Vector(0.0), Vector(0.5), Vector(0.0, 1.0, 0.0));
}

// Temps moyen (en microsecondes) d'un appel a Value() sur l'arbre final.
// nbSamples : nombre de points evalues, 10000 donne une moyenne stable
double BenchmarkEvaluation(const NodeSDF& scene, int nbSamples, unsigned seed)
{
  std::mt19937 rng(seed);

  std::uniform_real_distribution<double> uniform(-1.0, 1.0);

  double sum = 0.0;

  double ms = TimeMs([&]() {
    for (int i = 0; i < nbSamples; i++)
    {
      Vector p(uniform(rng), uniform(rng), uniform(rng));
      sum += scene.Value(p);
    }
  });

  std::cout << "  (checksum " << sum << ")\n";

  return ms * 1000.0 / nbSamples;   
}

int main()
{
  // sceneCenter, sceneRadius : sphere englobante de MakeTestScene()
  // sceneRadius un peu plus grand que l'objet (0.87 pour la boite) = 1.5
  const Vector sceneCenter(0.0);
  const double sceneRadius = 1.5;

  // taille des crateres, défini arbitrairement comme ~5% de la taille de l'objet : 0.05
  const double impactRadius = 0.05;

  // même seed pour les 2 méthodes pour avoir les mêmes rayons et un test plus fidèle
  const unsigned seed = 42;

  // nombre de Value() pour mesurer le cout d'evaluation de l'arbre final
  const int nbSamples = 10000;

  // tailles de test, l'incremental est en O(n^2), ne pas aller trop haut
  const int impactCounts[] = { 100, 500, 1000, 2000 };

  // tailles de paquet testees
  const int batchSizes[] = { 10, 50, 100 };

  for (int nbImpacts : impactCounts)
  {
    std::cout << "=== " << nbImpacts << " impacts ===\n";

    // Incremental
    std::unique_ptr<NodeSDF> eroded;
    double buildMs = TimeMs([&]() {
    eroded = ErodeIncremental(MakeTestScene(), nbImpacts, impactRadius, sceneCenter, sceneRadius, seed);       });
    double evalUs = BenchmarkEvaluation(*eroded, nbSamples, seed);
    std::cout << "  incremental : construction " << buildMs << " ms, Value " << evalUs << " us\n";


    for (int batchSize : batchSizes)
    {
      buildMs = TimeMs([&]() {
        eroded = ErodeBatch(MakeTestScene(), nbImpacts, batchSize, impactRadius, sceneCenter, sceneRadius, seed);
      });
      evalUs = BenchmarkEvaluation(*eroded, nbSamples, seed);
      std::cout << "  paquets de " << batchSize << " : construction " << buildMs << " ms, Value " << evalUs << " us\n";

    }
  }

  return 0;
}
