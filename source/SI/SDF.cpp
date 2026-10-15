#include "SI/SDFs.hpp"
#include "mathematics.h"
#include <math.h>
#include <algorithm>

NodeSDF::NodeSDF() = default;

SphereSDF::SphereSDF(const Vector& center, const float radius)
  : center(center), radius(radius)
{
}

double SphereSDF::Value(const Vector& p) const
{
  return Norm(center - p) - radius;
}

BoxSDF::BoxSDF(const Vector& center, const Vector size, const Vector& up)
  : center(center), size(size), up(up)
{
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

CylindreSDF::CylindreSDF(const Vector& A, const Vector& B, const float radius)
  : B(B), A(A), radius(radius)
{
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

CapsuleSDF::CapsuleSDF(const Vector& A, const Vector& B, const float threyshold)
  : B(B), A(A), threyshold(threyshold)
{
}

double CapsuleSDF::Value(const Vector&) const
{
  return 0.0;
}

PlaneSDF::PlaneSDF(const Vector& center, const Vector& normal)
  : center(center), normal(normal)
{
}

double PlaneSDF::Value(const Vector&) const
{
  return 0.0;
}

ConeSDF::ConeSDF(const Vector& A, const Vector& B, const float radius)
  : B(B), A(A), radius(radius)
{
}

double ConeSDF::Value(const Vector& p) const
{
  /*
  Vector AB = B-A;
  Vector AP = p-A;
  float pv = AP * Normalized(AB);
  float pu = std::sqrt(Norm(AP) * Norm(AP) - pv * pv);

  float h = Norm(AB);

  float radius_value = std::lerp(radius, 0.0f, std::clamp(pv/h, 0.0f, 1.0f));
  float qu = pu - radius_value;
  float qv = Math::Max(pv - h, -pv);
  Vector q = Vector(qu,qv,0.0);

  float outside_dist = Norm(Vector::Max(q,Vector()));
  float inside_dist = Math::Min(0.0,Math::Max(q[0], q[1]));
  return outside_dist + inside_dist;*/
  return 0.0;
}

DonutSDF::DonutSDF() = default;

double DonutSDF::Value(const Vector&) const
{
  return 0.0;
}

UnionSDF::UnionSDF( std::unique_ptr<NodeSDF>& left,  std::unique_ptr<NodeSDF>& right) : left(std::move(left)), right(std::move(right))
{
}

UnionSDF::UnionSDF(const std::vector<std::unique_ptr<NodeSDF>>&)
{
}

double UnionSDF::Value(const Vector& p) const
{
  return Math::Min(left->Value(p), right->Value(p));
}

IntersectionSDF::IntersectionSDF( std::unique_ptr<NodeSDF>& left,  std::unique_ptr<NodeSDF>& right) : left(std::move(left)), right(std::move(right))
{
}

IntersectionSDF::IntersectionSDF(const std::vector<std::unique_ptr<NodeSDF>>&)
{
}

double IntersectionSDF::Value(const Vector& p) const
{
  return Math::Max(left->Value(p), right->Value(p));
}

BlendSDF::BlendSDF( std::unique_ptr<NodeSDF>& left,  std::unique_ptr<NodeSDF>& right, float k) : left(std::move(left)), right(std::move(right)), k(k)
{
}

BlendSDF::BlendSDF(const std::vector<std::unique_ptr<NodeSDF>>&)
{
}

double BlendSDF::Value(const Vector& p) const //from inigo quilez
{
  float k4 = 4.0 * k;
  float a = left->Value(p);
  float b = right->Value(p);
  float h = Math::Max(k4-std::abs(a-b), 0.0);
  return std::min(a,b) - h*h*0.25/k4;
}

SubstractionSDF::SubstractionSDF( std::unique_ptr<NodeSDF>& minuend,  std::unique_ptr<NodeSDF>& subtrahend) : minuend(std::move(minuend)), subtrahend(std::move(subtrahend))
{
}

double SubstractionSDF::Value(const Vector& p) const
{
  return Math::Max(minuend->Value(p), -subtrahend->Value(p));
}

GridDomainRepetionSDF::GridDomainRepetionSDF(std::unique_ptr<NodeSDF>& primitive, const Vector& period)
  : primitive(std::move(primitive)), period(period)
{
}

double GridDomainRepetionSDF::Value(const Vector&) const
{
  return 0.0;
}