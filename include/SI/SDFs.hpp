// Implicits

#pragma once

#include <iostream>
#include <memory>
#include "implicits.h"

class NodeSDF : public AnalyticScalarField
{
public:
  NodeSDF();
};

class SphereSDF : public NodeSDF
{
private:
  Vector center;
  float radius;
public:
  SphereSDF(const Vector& center, const float radius);
  double Value(const Vector&) const override;
};

class BoxSDF : public NodeSDF
{
private:
  Vector center;
  Vector size;
  Vector up;
public:
  BoxSDF(const Vector& center, const Vector size, const Vector& up);
  double Value(const Vector&) const override;
};

class CylindreSDF : public NodeSDF
{
private:
  Vector B;
  Vector A; 
  float radius;
public:
  CylindreSDF(const Vector& A, const Vector& B, const float radius);
  double Value(const Vector&) const override;
};

class CapsuleSDF : public NodeSDF //SEGMENT
{
private:
  Vector B;
  Vector A; 
  float threyshold;
public:
  CapsuleSDF(const Vector& A, const Vector& B, const float threyshold);
  double Value(const Vector&) const override;
};

class PlaneSDF : public NodeSDF
{
private: 
  Vector center;
  Vector normal;
public:
  PlaneSDF(const Vector& center, const Vector& normal);
  double Value(const Vector&) const override;
};

class ConeSDF : public NodeSDF
{
private:
  Vector B;
  Vector A; 
  float radius;
public:
  ConeSDF(const Vector& A, const Vector& B, const float radius);
  double Value(const Vector&) const override;
};

class DonutSDF : public NodeSDF
{
public:
  DonutSDF();
  double Value(const Vector&) const override;
};

class UnionSDF : public NodeSDF
{
private : 
  std::unique_ptr<NodeSDF> left, right;
public:
  UnionSDF( std::unique_ptr<NodeSDF>& left,  std::unique_ptr<NodeSDF>& right);
  UnionSDF(const std::vector<std::unique_ptr<NodeSDF>>& primitives); //TODO should create a union tree of all primitives
  double Value(const Vector&) const override;
};

class IntersectionSDF : public NodeSDF
{
private : 
  std::unique_ptr<NodeSDF> left, right;
public:
  IntersectionSDF( std::unique_ptr<NodeSDF>& left,  std::unique_ptr<NodeSDF>& right);
  IntersectionSDF(const std::vector<std::unique_ptr<NodeSDF>>& primitives); //TODO should create an intersection tree of all primitives
  double Value(const Vector&) const override;
};

class BlendSDF : public NodeSDF
{
private : 
  std::unique_ptr<NodeSDF> left, right;
  float k;
public:
  BlendSDF( std::unique_ptr<NodeSDF>& left,  std::unique_ptr<NodeSDF>& right, float k);
  BlendSDF(const std::vector<std::unique_ptr<NodeSDF>>& primitives); //TODO should create a blend tree of all primitives
  double Value(const Vector&) const override;
};

class SubstractionSDF : public NodeSDF
{
private : 
  std::unique_ptr<NodeSDF> minuend, subtrahend;
public:
  SubstractionSDF( std::unique_ptr<NodeSDF>& minuend,  std::unique_ptr<NodeSDF>& subtrahend);
  double Value(const Vector&) const override;
};

class GridDomainRepetionSDF : public NodeSDF
{
private : 
  std::unique_ptr<NodeSDF> primitive;
  Vector period;
public:
  GridDomainRepetionSDF( std::unique_ptr<NodeSDF>& primitive, const Vector& period);
  double Value(const Vector&) const override;
};


