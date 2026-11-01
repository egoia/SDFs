// Implicits

#pragma once

#include <iostream>
#include <memory>
#include "mathematics.h"
#include "implicits.h"
#include "ray.h"

class NodeSDF : public AnalyticScalarField
{
public:
  NodeSDF();
  virtual std::string ToGLSL(const std::string& p = "p") const;

  bool Intersect(const Ray& ray, double& t,
                 double tMax = 100.0, double eps = 1e-4, int maxSteps = 256) const;
  double BenchmarkEvaluation(int nbSamples, unsigned seed) const;
};

class SphereSDF : public NodeSDF
{
private:
  Vector center;
  float radius;
public:
  SphereSDF(const Vector& center, const float radius);
  double Value(const Vector&) const override;
  std::string ToGLSL(const std::string& p = "p") const override;
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
  std::string ToGLSL(const std::string& p = "p") const override;
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
  std::string ToGLSL(const std::string& p = "p") const override;
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
  std::string ToGLSL(const std::string& p = "p") const override;
};

class PlaneSDF : public NodeSDF
{
private: 
  Vector center;
  Vector normal;
public:
  PlaneSDF(const Vector& center, const Vector& normal);
  double Value(const Vector&) const override;
  std::string ToGLSL(const std::string& p = "p") const override;
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
  std::string ToGLSL(const std::string& p = "p") const override;
};

class DonutSDF : public NodeSDF
{
private:
  Vector center;
  Vector up;
  float R, r;
public:
  DonutSDF(const Vector& center, const float R, const float r, const Vector& up = Vector(0.0, 0.0, 1.0));
  double Value(const Vector&) const override;
  std::string ToGLSL(const std::string& p = "p") const override;
};

class UnionSDF : public NodeSDF
{
private : 
  std::unique_ptr<NodeSDF> left, right;
public:
  UnionSDF( std::unique_ptr<NodeSDF>& left,  std::unique_ptr<NodeSDF>& right);
  UnionSDF(const std::vector<std::unique_ptr<NodeSDF>>& primitives); //TODO should create a union tree of all primitives
  double Value(const Vector&) const override;
  std::string ToGLSL(const std::string& p = "p") const override;
};

class IntersectionSDF : public NodeSDF
{
private : 
  std::unique_ptr<NodeSDF> left, right;
public:
  IntersectionSDF( std::unique_ptr<NodeSDF>& left,  std::unique_ptr<NodeSDF>& right);
  IntersectionSDF(const std::vector<std::unique_ptr<NodeSDF>>& primitives); //TODO should create an intersection tree of all primitives
  double Value(const Vector&) const override;
  std::string ToGLSL(const std::string& p = "p") const override;
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
  std::string ToGLSL(const std::string& p = "p") const override;
};

class SubstractionSDF : public NodeSDF
{
private : 
  std::unique_ptr<NodeSDF> minuend, subtrahend;
public:
  SubstractionSDF( std::unique_ptr<NodeSDF>& minuend,  std::unique_ptr<NodeSDF>& subtrahend);
  double Value(const Vector&) const override;
  std::string ToGLSL(const std::string& p = "p") const override;
};

class GridDomainRepetionSDF : public NodeSDF
{
private : 
  std::unique_ptr<NodeSDF> primitive;
  Vector period;
public:
  GridDomainRepetionSDF( std::unique_ptr<NodeSDF>& primitive, const Vector& period);
  double Value(const Vector&) const override;
  std::string ToGLSL(const std::string& p = "p") const override;
};

class TwistSDF : public NodeSDF
{
private :
  std::unique_ptr<NodeSDF> primitive;
  float k;
public:
  TwistSDF( std::unique_ptr<NodeSDF>& primitive, float k);
  double Value(const Vector&) const override;
};

class NoiseSDF : public NodeSDF
{
private :
  std::unique_ptr<NodeSDF> primitive;
  float amplitude, frequency;
  static double Hash(const Vector&);
  static double Noise(const Vector&);
public:
  NoiseSDF( std::unique_ptr<NodeSDF>& primitive, float amplitude, float frequency);
  double Value(const Vector&) const override;
};

class BoundingBoxSDF : public NodeSDF
{
private : 
    std::unique_ptr<BoxSDF> box;
    std::unique_ptr<NodeSDF> tree;

public:
  BoundingBoxSDF(std::unique_ptr<NodeSDF>& tree, std::unique_ptr<BoxSDF>& box);
  double Value(const Vector&) const override;
  std::string ToGLSL(const std::string& p = "p") const override;

};
