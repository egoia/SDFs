#include "SI/SDFs.hpp"
#include "mathematics.h"
#include <math.h>
#include <cmath>
#include <algorithm>
#include <chrono>
#include <iostream>
#include <memory>
#include <random>
#include "mathematics.h"

NodeSDF::NodeSDF() = default;

std::string NodeSDF::ToGLSL(const std::string&) const
{
  return {};
}

template <typename F>
double TimeMs(F&& f)
{
  auto start = std::chrono::steady_clock::now();

  f();

  auto end = std::chrono::steady_clock::now();

  return std::chrono::duration<double, std::milli>(end - start).count();
}

// Temps moyen (en microsecondes) d'un appel a Value() sur l'arbre final.
// nbSamples : nombre de points evalues, 10000 donne une moyenne stable
double NodeSDF::BenchmarkEvaluation(int nbSamples, unsigned seed) const
{
  std::mt19937 rng(seed);

  std::uniform_real_distribution<double> uniform(-1.0, 1.0);

  double sum = 0.0;

  double ms = TimeMs([&]() {
    for (int i = 0; i < nbSamples; i++)
    {
      Vector p(uniform(rng), uniform(rng), uniform(rng));
      sum += Value(p);
    }
  });


  return ms * 1000.0 / nbSamples;   
}

//* ====== SPHERE ======
SphereSDF::SphereSDF(const Vector& center, const float radius)
  : center(center), radius(radius)
{
}

std::string SphereSDF::ToGLSL(const std::string& p) const
{
  return "sphere_SDF(" + p + ", " + center.ToGLSL() + ", " + std::to_string(radius) +")";
}

double SphereSDF::Value(const Vector& p) const
{
  return Norm(center - p) - radius;
}


//* ====== BOX ======
BoxSDF::BoxSDF(const Vector& center, const Vector size, const Vector& up)
  : center(center), size(size), up(up)
{
}

std::string BoxSDF::ToGLSL(const std::string& p) const
{
  return "box_SDF(" + p + ", " + center.ToGLSL() + ", " + size.ToGLSL() + ", "+ up.ToGLSL() +")";
}

double BoxSDF::Value(const Vector& p) const
{
    Vector p_relatif = p-center;

    Vector right, forward;
    up.Orthonormal(right, forward);

    Vector p_local(p_relatif * right, p_relatif * up, p_relatif * forward);

    Vector q = Abs(p_local) - size;

    float outside_dist = Norm(Vector::Max(q, Vector(0.0)));
    float inside_dist = Math::Min(0.0,Math::Max(Math::Max(q[0], q[1]), q[2]));
    return outside_dist + inside_dist;
}


//* ====== CYLINDRE ======
CylindreSDF::CylindreSDF(const Vector& A, const Vector& B, const float radius)
  : B(B), A(A), radius(radius)
{
}

std::string CylindreSDF::ToGLSL(const std::string& p) const
{
  return "cylinder_SDF(" + p + ", " + A.ToGLSL() + ", " + B.ToGLSL() + ", " + std::to_string(radius) +")";
}

double CylindreSDF::Value(const Vector& p) const
{
  Vector AB = B-A;
  Vector AP = p-A;
  float pv = AP * Normalized(AB);
  float pu = std::sqrt(Math::Max(0.0, Norm(AP) * Norm(AP) - pv * pv));

  float h = Norm(AB);
  float qu = pu - radius;
  float qv = Math::Max(pv - h, -pv);
  Vector q = Vector(qu,qv,0.0);

  float outside_dist = Norm(Vector::Max(q, Vector(0.0)));
  float inside_dist = Math::Min(0.0,Math::Max(q[0], q[1]));
  return outside_dist + inside_dist;
}


//* ====== CAPSULE ======
CapsuleSDF::CapsuleSDF(const Vector& A, const Vector& B, const float threyshold)
  : B(B), A(A), threyshold(threyshold)
{
}

double distanceSegment(const Vector& A, const Vector& B, const Vector& p ){
  Vector AP = p-A;
  Vector AB = B-A;
  double h = Math::Clamp((AP * AB) / (AB * AB), 0.0, 1.0);
  return Norm(AP - AB*h);
}

double CapsuleSDF::Value(const Vector& p) const //from inigo quilez
{
  return distanceSegment(A,B,p) - threyshold;
}

std::string CapsuleSDF::ToGLSL(const std::string& p) const
{
  return "segment_SDF(" + p + ", " + A.ToGLSL() + ", " + B.ToGLSL() + ", " + std::to_string(threyshold) +")";
}


//* ====== PLANE ======
PlaneSDF::PlaneSDF(const Vector& center, const Vector& normal)
  : center(center), normal(normal)
{
}

double PlaneSDF::Value(const Vector& p) const
{
  return (p-center) * Normalized(normal);
}

std::string PlaneSDF::ToGLSL(const std::string& p) const
{
  return "plane_SDF(" + p + " - " + center.ToGLSL() + ", " + normal.ToGLSL() + ", 0.0)";
}


//* ====== CONE ======
ConeSDF::ConeSDF(const Vector& A, const Vector& B, const float radius)
  : B(B), A(A), radius(radius)
{
}


double ConeSDF::Value(const Vector& p) const
{
  Vector AB = B-A;
  Vector AP = p-A;
  float pv = AP * Normalized(AB);
  float pu = std::sqrt(Norm(AP) * Norm(AP) - pv * pv);

  Vector q = Vector(pu,pv,0.0);

  float h = Norm(AB);

  Vector Aprime = Vector(0.0,0.0,0.0);
  Vector Bprime = Vector(0.0,h,0.0);
  Vector u = Vector(1.0,0.0,0.0);
  float distanceToSide = distanceSegment(Bprime, u * radius, q);
  float distanceToBase = distanceSegment(Aprime, u*radius, q);
  float distance = std::min(distanceToBase, distanceToSide);

  bool isInside = (pv > 0.0f) && (pu<std::lerp(radius, 0.0f, std::clamp(pv/h, 0.0f, 1.0f)));

  return isInside ? -distance : distance;
}

std::string ConeSDF::ToGLSL(const std::string& p) const
{
  return "cone_SDF(" + p + ", " + A.ToGLSL() + ", " + B.ToGLSL() + ", " + std::to_string(radius) +")";
}


//* ====== DONUT ======
DonutSDF::DonutSDF(const Vector& center, const float R, const float r, const Vector& up)
  : center(center), up(Normalized(up)), R(R), r(r)
{
}

double DonutSDF::Value(const Vector& p) const //from inigo quilez
{
  Vector p_relatif = p-center;

  double h = p_relatif * up;
  double d = std::sqrt(Math::Max(0.0, SquaredNorm(p_relatif) - h*h)); //distance a l'axe

  double qx = d - R;
  return std::sqrt(qx*qx + h*h) - r;
}

std::string DonutSDF::ToGLSL(const std::string& p) const
{
  return "torus_SDF(" + p + ", " + center.ToGLSL() + ", " + up.ToGLSL() + ", " + std::to_string(R) + ", " + std::to_string(r) +")";
}


//* ====== UNION ======
UnionSDF::UnionSDF( std::unique_ptr<NodeSDF>& left,  std::unique_ptr<NodeSDF>& right) : left(std::move(left)), right(std::move(right))
{
}

UnionSDF::UnionSDF(const std::vector<std::unique_ptr<NodeSDF>>&)
{
}

std::string UnionSDF::ToGLSL(const std::string& p) const
{
  return "union_op(" + left->ToGLSL(p) + ", " + right->ToGLSL(p) +")";
}

double UnionSDF::Value(const Vector& p) const
{
  return Math::Min(left->Value(p), right->Value(p));
}


//* ====== INTERSECTION ======
IntersectionSDF::IntersectionSDF( std::unique_ptr<NodeSDF>& left,  std::unique_ptr<NodeSDF>& right) : left(std::move(left)), right(std::move(right))
{
}

IntersectionSDF::IntersectionSDF(const std::vector<std::unique_ptr<NodeSDF>>&)
{
}

std::string IntersectionSDF::ToGLSL(const std::string& p) const
{
  return "intersection_op(" + left->ToGLSL(p) + ", " + right->ToGLSL(p) +")";
}

double IntersectionSDF::Value(const Vector& p) const
{
  return Math::Max(left->Value(p), right->Value(p));
}


//* ====== BLEND ======
BlendSDF::BlendSDF( std::unique_ptr<NodeSDF>& left,  std::unique_ptr<NodeSDF>& right, float k) : left(std::move(left)), right(std::move(right)), k(k)
{
}

BlendSDF::BlendSDF(const std::vector<std::unique_ptr<NodeSDF>>&)
{
}

std::string BlendSDF::ToGLSL(const std::string& p) const
{
  return  "blend_op(" + left->ToGLSL(p) + ", " + right->ToGLSL(p) +", "+ std::to_string(k)+")";
}

double BlendSDF::Value(const Vector& p) const //from inigo quilez
{
  float k4 = 4.0 * k;
  float a = left->Value(p);
  float b = right->Value(p);
  float h = Math::Max(k4-std::abs(a-b), 0.0);
  return std::min(a,b) - h*h*0.25/k4;
}


//* ====== SUBSTRACTION ======
SubstractionSDF::SubstractionSDF( std::unique_ptr<NodeSDF>& minuend,  std::unique_ptr<NodeSDF>& subtrahend) : minuend(std::move(minuend)), subtrahend(std::move(subtrahend))
{
}

std::string SubstractionSDF::ToGLSL(const std::string& p) const
{
  return "substract_op(" + minuend->ToGLSL(p) + ", " + subtrahend->ToGLSL(p) +")";
}

double SubstractionSDF::Value(const Vector& p) const
{
  return Math::Max(minuend->Value(p), -subtrahend->Value(p));
}


//* ====== DOMAIN REPETITION ======
GridDomainRepetionSDF::GridDomainRepetionSDF(std::unique_ptr<NodeSDF>& primitive, const Vector& period)
  : primitive(std::move(primitive)), period(period)
{
}

double GridDomainRepetionSDF::Value(const Vector& p) const 
{
  Vector q = Vector(p);
  for (int i = 0; i < 3; i++)
    q[i] -= period[i] * std::round(p[i] / period[i]);
  return primitive->Value(q);
}

std::string GridDomainRepetionSDF::ToGLSL(const std::string& p) const
{
  return primitive->ToGLSL("repetition_op(" + p + ", " + period.ToGLSL() + ")");
}

//* ====== TWIST ======
TwistSDF::TwistSDF(std::unique_ptr<NodeSDF>& primitive, float k)
  : primitive(std::move(primitive)), k(k)
{
}

double TwistSDF::Value(const Vector& p) const 
{
  double c = std::cos(k*p[2]);
  double s = std::sin(k*p[2]);
  Vector q(c*p[0] - s*p[1], s*p[0] + c*p[1], p[2]);
  return primitive->Value(q);
}



//* ====== NOISE ======
NoiseSDF::NoiseSDF(std::unique_ptr<NodeSDF>& primitive, float amplitude, float frequency)
  : primitive(std::move(primitive)), amplitude(amplitude), frequency(frequency)
{
}

double NoiseSDF::Hash(const Vector& p)
{
  Vector q;
  for (int i = 0; i < 3; i++)
  {
    double x = p[i]*0.3183099 + 0.1;
    q[i] = 17.0 * (x - std::floor(x));
  }
  double h = q[0]*q[1]*q[2]*(q[0]+q[1]+q[2]);
  return h - std::floor(h);
}

double NoiseSDF::Noise(const Vector& x) //value noise 3D dans [-1,1]
{
  Vector i(std::floor(x[0]), std::floor(x[1]), std::floor(x[2]));
  Vector f = x - i;
  for (int k = 0; k < 3; k++)
    f[k] = f[k]*f[k]*(3.0 - 2.0*f[k]);

  auto mix = [](double a, double b, double t) { return a + (b-a)*t; };
  auto h = [&](double dx, double dy, double dz) { return Hash(i + Vector(dx, dy, dz)); };

  return -1.0 + 2.0*mix(mix(mix(h(0,0,0), h(1,0,0), f[0]),
                            mix(h(0,1,0), h(1,1,0), f[0]), f[1]),
                        mix(mix(h(0,0,1), h(1,0,1), f[0]),
                            mix(h(0,1,1), h(1,1,1), f[0]), f[1]), f[2]);
}

double NoiseSDF::Value(const Vector& p) const 
{
  return primitive->Value(p) + amplitude * Noise(p * frequency);
}



//* ====== SPHERE TRACING  ======
bool NodeSDF::Intersect(const Ray& ray, double& t, double tMax, double eps, int maxSteps) const
{
  t = 0.0;
  for (int i = 0; i < maxSteps; i++)
  {

    Vector p = ray(t);
    double d= Value(p);
    if (d<eps) return true;
    t+=d;
    if (t>tMax) return false;
  }
  return false;
}


//* ====== BOUNDING BOX ======
BoundingBoxSDF::BoundingBoxSDF( std::unique_ptr<NodeSDF>& tree,  std::unique_ptr<BoxSDF>& box) : tree(std::move(tree)), box(std::move(box))
{
}

const double EPSILON = 0.00000001;

double BoundingBoxSDF::Value(const Vector& p) const
{
  double b = box->Value(p);
  if(b - EPSILON > 0) return b - EPSILON;
  else return tree->Value(p);
}

std::string BoundingBoxSDF::ToGLSL(const std::string& p) const {
  const std::string boxDistance = box->ToGLSL(p);
  constexpr const char* marchEpsilonOffset = "0.0002";

  return "((" + boxDistance + " > 0.0) ? "
       "(" + boxDistance + " + " + marchEpsilonOffset + ") : "
       "(" + tree->ToGLSL(p) + "))";
}