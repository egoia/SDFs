#include "SI/SDFs.hpp"

NodeSDF::NodeSDF() = default;

SphereSDF::SphereSDF(const Vector& center, const float radius)
  : center(center), radius(radius)
{
}

double SphereSDF::Value(const Vector&) const
{
  return 0;
}

BoxSDF::BoxSDF(const Vector& center, const float side)
  : center(center), side(side)
{
}

double BoxSDF::Value(const Vector&) const
{
  return 0.0;
}

CylindreSDF::CylindreSDF(const Vector& center, const Vector& dir, const float height, const float radius)
  : center(center), dir(dir), height(height), radius(radius)
{
}

double CylindreSDF::Value(const Vector&) const
{
  return 0.0;
}

CapsuleSDF::CapsuleSDF(const Vector& center, const Vector& dir, const float height, const float threyshold)
  : center(center), dir(dir), height(height), theryshold(threyshold)
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

ConeSDF::ConeSDF(const Vector& center, const Vector& dir, const float height, const float radius)
  : center(center), dir(dir), height(height), radius(radius)
{
}

double ConeSDF::Value(const Vector&) const
{
  return 0.0;
}

DonutSDF::DonutSDF() = default;

double DonutSDF::Value(const Vector&) const
{
  return 0.0;
}

UnionSDF::UnionSDF(const std::unique_ptr<NodeSDF>&, const std::unique_ptr<NodeSDF>&)
{
}

UnionSDF::UnionSDF(const std::vector<std::unique_ptr<NodeSDF>>&)
{
}

double UnionSDF::Value(const Vector&) const
{
  return 0.0;
}

IntersectionSDF::IntersectionSDF(const std::unique_ptr<NodeSDF>&, const std::unique_ptr<NodeSDF>&)
{
}

IntersectionSDF::IntersectionSDF(const std::vector<std::unique_ptr<NodeSDF>>&)
{
}

double IntersectionSDF::Value(const Vector&) const
{
  return 0.0;
}

BlendSDF::BlendSDF(const std::unique_ptr<NodeSDF>&, const std::unique_ptr<NodeSDF>&)
{
}

BlendSDF::BlendSDF(const std::vector<std::unique_ptr<NodeSDF>>&)
{
}

double BlendSDF::Value(const Vector&) const
{
  return 0.0;
}

SubstractionSDF::SubstractionSDF(const std::unique_ptr<NodeSDF>&, const std::unique_ptr<NodeSDF>&)
{
}

double SubstractionSDF::Value(const Vector&) const
{
  return 0.0;
}

GridDomainRepetionSDF::GridDomainRepetionSDF(const std::unique_ptr<NodeSDF>&, const Vector&)
{
}

double GridDomainRepetionSDF::Value(const Vector&) const
{
  return 0.0;
}