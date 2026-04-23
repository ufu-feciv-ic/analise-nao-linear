#include "editor/interaction/BeamSnapResolver.h"

#include <algorithm>
#include <cmath>

namespace
{
constexpr double kGeometryTolerance = 0.00001;
constexpr float kSnapDistancePixels = 10.0f;

Vector2 Subtract(Vector2 a, Vector2 b)
{
    return Vector2{a.x - b.x, a.y - b.y};
}

double DotProduct(Vector2 a, Vector2 b)
{
    return static_cast<double>(a.x) * static_cast<double>(b.x) +
           static_cast<double>(a.y) * static_cast<double>(b.y);
}

double LengthSquared(Vector2 vector)
{
    return DotProduct(vector, vector);
}

double ProjectPointParameter(Vector2 point, Vector2 start, Vector2 end)
{
    const Vector2 segment = Subtract(end, start);
    const double segmentLengthSquared = LengthSquared(segment);
    if (segmentLengthSquared <= kGeometryTolerance)
    {
        return 0.0;
    }

    return DotProduct(Subtract(point, start), segment) / segmentLengthSquared;
}

bool TryGetSegmentIntersection(Vector2 startA, Vector2 endA, Vector2 startB, Vector2 endB, Vector2& intersection)
{
    const double x1 = startA.x;
    const double y1 = startA.y;
    const double x2 = endA.x;
    const double y2 = endA.y;
    const double x3 = startB.x;
    const double y3 = startB.y;
    const double x4 = endB.x;
    const double y4 = endB.y;

    const double denominator = (x1 - x2) * (y3 - y4) - (y1 - y2) * (x3 - x4);
    if (std::abs(denominator) <= kGeometryTolerance)
    {
        return false;
    }

    const double t = ((x1 - x3) * (y3 - y4) - (y1 - y3) * (x3 - x4)) / denominator;
    const double u = ((x1 - x3) * (y1 - y2) - (y1 - y3) * (x1 - x2)) / denominator;

    if (t < -kGeometryTolerance || t > 1.0 + kGeometryTolerance ||
        u < -kGeometryTolerance || u > 1.0 + kGeometryTolerance)
    {
        return false;
    }

    intersection = Vector2{
        static_cast<float>(x1 + t * (x2 - x1)),
        static_cast<float>(y1 + t * (y2 - y1))};
    return true;
}
}

namespace BeamSnapResolver
{
bool TryGetBeamIntersectionSnap(
    const ProjectDocument& document,
    const Camera2D& camera,
    Vector2 mouseScreenPosition,
    Vector2& snappedPosition)
{
    bool foundIntersection = false;
    float closestDistanceSquared = kSnapDistancePixels * kSnapDistancePixels;

    for (std::size_t i = 0; i < document.beams.size(); ++i)
    {
        const Beam& firstBeam = document.beams[i];
        const Node* firstStartNode = document.FindNodeById(firstBeam.startNodeId);
        const Node* firstEndNode = document.FindNodeById(firstBeam.endNodeId);
        if (firstStartNode == nullptr || firstEndNode == nullptr)
        {
            continue;
        }

        const Vector2 firstStart = Vector2{
            static_cast<float>(firstStartNode->position.x),
            static_cast<float>(firstStartNode->position.y)};
        const Vector2 firstEnd = Vector2{
            static_cast<float>(firstEndNode->position.x),
            static_cast<float>(firstEndNode->position.y)};

        for (std::size_t j = i + 1; j < document.beams.size(); ++j)
        {
            const Beam& secondBeam = document.beams[j];
            const Node* secondStartNode = document.FindNodeById(secondBeam.startNodeId);
            const Node* secondEndNode = document.FindNodeById(secondBeam.endNodeId);
            if (secondStartNode == nullptr || secondEndNode == nullptr)
            {
                continue;
            }

            const Vector2 secondStart = Vector2{
                static_cast<float>(secondStartNode->position.x),
                static_cast<float>(secondStartNode->position.y)};
            const Vector2 secondEnd = Vector2{
                static_cast<float>(secondEndNode->position.x),
                static_cast<float>(secondEndNode->position.y)};

            Vector2 intersection{0.0f, 0.0f};
            if (!TryGetSegmentIntersection(firstStart, firstEnd, secondStart, secondEnd, intersection))
            {
                continue;
            }

            const Vector2 intersectionScreen = GetWorldToScreen2D(intersection, camera);
            const float dx = intersectionScreen.x - mouseScreenPosition.x;
            const float dy = intersectionScreen.y - mouseScreenPosition.y;
            const float distanceSquared = dx * dx + dy * dy;
            if (distanceSquared <= closestDistanceSquared)
            {
                closestDistanceSquared = distanceSquared;
                snappedPosition = intersection;
                foundIntersection = true;
            }
        }
    }

    return foundIntersection;
}

bool TryGetBeamInteriorSnap(
    const ProjectDocument& document,
    const Camera2D& camera,
    Vector2 mouseWorldPosition,
    Vector2 mouseScreenPosition,
    Vector2& snappedPosition,
    int& referenceBeamId)
{
    referenceBeamId = -1;

    bool foundSnap = false;
    float closestDistanceSquared = kSnapDistancePixels * kSnapDistancePixels;

    for (const Beam& beam : document.beams)
    {
        const Node* startNode = document.FindNodeById(beam.startNodeId);
        const Node* endNode = document.FindNodeById(beam.endNodeId);
        if (startNode == nullptr || endNode == nullptr)
        {
            continue;
        }

        const Vector2 startPosition = Vector2{
            static_cast<float>(startNode->position.x),
            static_cast<float>(startNode->position.y)};
        const Vector2 endPosition = Vector2{
            static_cast<float>(endNode->position.x),
            static_cast<float>(endNode->position.y)};

        const double parameter = std::clamp(
            ProjectPointParameter(mouseWorldPosition, startPosition, endPosition),
            0.0,
            1.0);
        if (parameter <= kGeometryTolerance || parameter >= 1.0 - kGeometryTolerance)
        {
            continue;
        }

        const Vector2 projectedPosition = Vector2{
            startPosition.x + (endPosition.x - startPosition.x) * static_cast<float>(parameter),
            startPosition.y + (endPosition.y - startPosition.y) * static_cast<float>(parameter)};
        const Vector2 projectedScreen = GetWorldToScreen2D(projectedPosition, camera);
        const float dx = projectedScreen.x - mouseScreenPosition.x;
        const float dy = projectedScreen.y - mouseScreenPosition.y;
        const float distanceSquared = dx * dx + dy * dy;
        if (distanceSquared <= closestDistanceSquared)
        {
            closestDistanceSquared = distanceSquared;
            snappedPosition = projectedPosition;
            referenceBeamId = beam.id;
            foundSnap = true;
        }
    }

    return foundSnap;
}
}
