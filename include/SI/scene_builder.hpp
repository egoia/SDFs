#pragma once

#include "SI/SDFs.hpp"

#include <memory>
#include <vector>

namespace SDFScenes {

enum class PrimitiveType {
    Sphere,
    Box,
    Plane,
    Cylinder,
    Capsule,
    Cone,
    Torus
};

inline std::unique_ptr<NodeSDF> BuildDemoScene()
{
    const Vector up(0.0, 1.0, 0.0);
    const Vector centers[] = {
        Vector(-9.0,  2.0, 0.0), Vector(-6.0,  2.0, 0.0),
        Vector(-3.0,  2.0, 0.0), Vector( 0.0,  2.0, 0.0),
        Vector( 3.0,  2.0, 0.0), Vector( 6.0,  2.0, 0.0),
        Vector( 9.0,  2.0, 0.0), Vector(-9.0, -2.0, 0.0),
        Vector(-6.0, -2.0, 0.0), Vector(-3.0, -2.0, 0.0)
    };

    std::vector<std::unique_ptr<NodeSDF>> objects;
    objects.reserve(10);

    objects.push_back(std::make_unique<SphereSDF>(centers[0], 0.55f));
    objects.push_back(std::make_unique<BoxSDF>(
        centers[1], Vector(0.48, 0.58, 0.42), up));
    objects.push_back(std::make_unique<CylindreSDF>(
        centers[2] + Vector(0.0, -0.55, 0.0),
        centers[2] + Vector(0.0,  0.55, 0.0), 0.38f));
    objects.push_back(std::make_unique<CapsuleSDF>(
        centers[3] + Vector(-0.4, -0.3, 0.0),
        centers[3] + Vector( 0.4,  0.3, 0.0), 0.27f));
    objects.push_back(std::make_unique<ConeSDF>(
        centers[4] + Vector(0.0, -0.55, 0.0),
        centers[4] + Vector(0.0,  0.55, 0.0), 0.5f));
    objects.push_back(std::make_unique<DonutSDF>(
        centers[5], 0.42f, 0.17f, Vector(0.0, 0.0, 1.0)));

    std::unique_ptr<NodeSDF> unionLeft = std::make_unique<SphereSDF>(
        centers[6] + Vector(-0.28, 0.0, 0.0), 0.4f);
    std::unique_ptr<NodeSDF> unionRight = std::make_unique<SphereSDF>(
        centers[6] + Vector(0.28, 0.0, 0.0), 0.4f);
    objects.push_back(std::make_unique<UnionSDF>(unionLeft, unionRight));

    std::unique_ptr<NodeSDF> intersectionLeft =
        std::make_unique<SphereSDF>(centers[7], 0.58f);
    std::unique_ptr<NodeSDF> intersectionRight = std::make_unique<BoxSDF>(
        centers[7], Vector(0.45), up);
    objects.push_back(std::make_unique<IntersectionSDF>(
        intersectionLeft, intersectionRight));

    std::unique_ptr<NodeSDF> subtractionLeft =
        std::make_unique<SphereSDF>(centers[8], 0.58f);
    std::unique_ptr<NodeSDF> subtractionRight = std::make_unique<SphereSDF>(
        centers[8] + Vector(0.55, 0.0, 0.0), 0.48f);
    objects.push_back(std::make_unique<SubstractionSDF>(
        subtractionLeft, subtractionRight));

    std::unique_ptr<NodeSDF> blendLeft = std::make_unique<SphereSDF>(
        centers[9] + Vector(-0.3, 0.0, 0.0), 0.4f);
    std::unique_ptr<NodeSDF> blendRight = std::make_unique<SphereSDF>(
        centers[9] + Vector(0.3, 0.0, 0.0), 0.4f);
    objects.push_back(std::make_unique<BlendSDF>(blendLeft, blendRight, 0.2f));

    std::unique_ptr<NodeSDF> scene = std::move(objects[0]);
    for (std::size_t i = 1; i < objects.size(); ++i) {
        scene = std::make_unique<UnionSDF>(scene, objects[i]);
    }
    return scene;
}

inline std::unique_ptr<NodeSDF> BuildPerformanceScene(PrimitiveType type)
{
    constexpr int objectCount = 1000;
    const Vector center(0.0, 0.0, 0.0);
    constexpr float radius = 0.1f;
    constexpr double halfHeight = 0.1;

    std::vector<std::unique_ptr<NodeSDF>> objects;
    objects.reserve(objectCount);

    for (int i = 0; i < objectCount; ++i) {
        std::unique_ptr<NodeSDF> object;

        switch (type) {
        case PrimitiveType::Sphere:
            object = std::make_unique<SphereSDF>(center, radius);
            break;
        case PrimitiveType::Box:
            object = std::make_unique<BoxSDF>(
                center, Vector(0.1), Vector(0.0, 1.0, 0.0));
            break;
        case PrimitiveType::Plane:
            object = std::make_unique<PlaneSDF>(
                center, Vector(0.0, 1.0, 0.0));
            break;
        case PrimitiveType::Cylinder:
            object = std::make_unique<CylindreSDF>(
                center + Vector(0.0, -halfHeight, 0.0),
                center + Vector(0.0, halfHeight, 0.0), radius);
            break;
        case PrimitiveType::Capsule:
            object = std::make_unique<CapsuleSDF>(
                center + Vector(-halfHeight, 0.0, 0.0),
                center + Vector(halfHeight, 0.0, 0.0), radius);
            break;
        case PrimitiveType::Cone:
            object = std::make_unique<ConeSDF>(
                center + Vector(0.0, -halfHeight, 0.0),
                center + Vector(0.0, halfHeight, 0.0), radius);
            break;
        case PrimitiveType::Torus:
            object = std::make_unique<DonutSDF>(
                center, radius, radius * 0.35f, Vector(0.0, 0.0, 1.0));
            break;
        }

        objects.push_back(std::move(object));
    }

    while (objects.size() > 1) {
        std::vector<std::unique_ptr<NodeSDF>> nextLevel;
        nextLevel.reserve((objects.size() + 1) / 2);

        for (std::size_t i = 0; i < objects.size(); i += 2) {
            if (i + 1 < objects.size()) {
                nextLevel.push_back(
                    std::make_unique<UnionSDF>(objects[i], objects[i + 1]));
            } else {
                nextLevel.push_back(std::move(objects[i]));
            }
        }

        objects = std::move(nextLevel);
    }

    return std::move(objects.front());
}

} 
