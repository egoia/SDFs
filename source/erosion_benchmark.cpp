// Comparaison des performances : erosion incrementale vs erosion par paquets
// A compiler en Release, sinon les temps mesures ne veulent rien dire.
//
// Pour chaque nombre d'impacts et chaque taille de paquet, on mesure :
//   - le temps de construction de l'arbre erode
//   - le temps moyen d'un appel a Value() sur l'arbre final
// La comparaison des formes se fait visuellement avec le viewer marching-cube.

#include <algorithm>
#include <chrono>
#include <cstdio>
#include <memory>
#include <random>
#include <vector>
#include "mathematics.h"
#include "SI/SDFs.hpp"
#include "SI/erosion.hpp"

// sphere englobante des projectiles, un peu plus grande que la boite (demi-diagonale 0.87)
const Vector sceneCenter(0.0);
const double sceneRadius = 1.5;

// taille des crateres, ~5% de la taille de l'objet
const double impactRadius = 0.05;

// meme seed pour toutes les methodes : memes rayons tires
const unsigned seed = 42;

// nombres d'impacts testes
const int impactCounts[] = { 100, 500, 1000, 2000, 4000 };

// tailles de paquet testees
const int batchSizes[] = { 5, 10, 25, 50, 100, 500 };

// chaque temps est mesure plusieurs fois, on garde la mediane
const int nbRepetitions = 5;

// nombre de points pour mesurer le temps d'un Value()
const int nbTimingSamples = 10000;

std::unique_ptr<NodeSDF> MakeTestScene()
{
  return std::make_unique<BoxSDF>(Vector(0.0), Vector(0.5), Vector(0.0, 1.0, 0.0));
}

// Chronometre f() et renvoie le temps ecoule en millisecondes.
template <typename F>
double ChronoMs(F&& f)
{
  auto start = std::chrono::steady_clock::now();
  f();
  auto end = std::chrono::steady_clock::now();
  return std::chrono::duration<double, std::milli>(end - start).count();
}

double Median(std::vector<double> values)
{
  std::sort(values.begin(), values.end());
  return values[values.size() / 2];
}

// Points tires uniformement dans le cube [-0.6, 0.6]^3, qui contient la boite
std::vector<Vector> RandomPoints(int nb, unsigned pointSeed)
{
  std::mt19937 rng(pointSeed);
  std::uniform_real_distribution<double> uniform(-0.6, 0.6);
  std::vector<Vector> points;
  for (int i = 0; i < nb; i++)
    points.push_back(Vector(uniform(rng), uniform(rng), uniform(rng)));
  return points;
}

// batchSize = 0 : erosion incrementale
std::unique_ptr<NodeSDF> Erode(int nbImpacts, int batchSize)
{
  if (batchSize == 0)
    return ErodeIncremental(MakeTestScene(), nbImpacts, impactRadius, sceneCenter, sceneRadius, seed);
  return ErodeBatch(MakeTestScene(), nbImpacts, batchSize, impactRadius, sceneCenter, sceneRadius, seed);
}

// Temps median de construction (ms), l'arbre construit est renvoye dans eroded
double BenchmarkBuild(int nbImpacts, int batchSize, std::unique_ptr<NodeSDF>& eroded)
{
  std::vector<double> times;
  for (int rep = 0; rep < nbRepetitions; rep++)
    times.push_back(ChronoMs([&]() { eroded = Erode(nbImpacts, batchSize); }));
  return Median(times);
}

// Temps median d'un appel a Value() (us), sur des points tires a l'avance
double BenchmarkValue(const NodeSDF& scene, const std::vector<Vector>& points)
{
  double sum = 0.0;
  std::vector<double> times;
  for (int rep = 0; rep < nbRepetitions; rep++)
    times.push_back(ChronoMs([&]() {
      for (const Vector& p : points)
        sum += scene.Value(p);
    }) * 1000.0 / points.size());
  if (sum == 0.123456) std::printf(" "); // empeche le compilateur de supprimer la boucle
  return Median(times);
}

int main()
{
  const std::vector<Vector> timingPoints = RandomPoints(nbTimingSamples, 1);

  for (int nbImpacts : impactCounts)
  {
    std::printf("\n=== %d impacts ===\n", nbImpacts);
    std::printf("methode          construction (ms)   Value (us)\n");

    std::unique_ptr<NodeSDF> incremental;
    double buildMs = BenchmarkBuild(nbImpacts, 0, incremental);
    double valueUs = BenchmarkValue(*incremental, timingPoints);
    std::printf("incremental      %17.2f   %10.2f\n", buildMs, valueUs);

    for (int batchSize : batchSizes)
    {
      if (batchSize >= nbImpacts)
        continue;

      std::unique_ptr<NodeSDF> batch;
      buildMs = BenchmarkBuild(nbImpacts, batchSize, batch);
      valueUs = BenchmarkValue(*batch, timingPoints);
      std::printf("paquets de %-4d  %17.2f   %10.2f\n", batchSize, buildMs, valueUs);
    }
  }

  return 0;
}
