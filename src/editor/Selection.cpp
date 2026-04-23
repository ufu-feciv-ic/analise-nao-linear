#include "editor/Selection.h"

#include <algorithm>

bool Selection::HasAny() const
{
    return !nodeIds.empty() || !beamIds.empty() || !dimensionIds.empty();
}

bool Selection::HasNodes() const
{
    return !nodeIds.empty();
}

bool Selection::HasBeams() const
{
    return !beamIds.empty();
}

bool Selection::HasDimensions() const
{
    return !dimensionIds.empty();
}

void Selection::Clear()
{
    nodeIds.clear();
    beamIds.clear();
    dimensionIds.clear();
}

void Selection::ClearNodes()
{
    nodeIds.clear();
}

void Selection::ClearBeams()
{
    beamIds.clear();
}

void Selection::ClearDimensions()
{
    dimensionIds.clear();
}

bool Selection::IsNodeSelected(int id) const
{
    return std::find(nodeIds.begin(), nodeIds.end(), id) != nodeIds.end();
}

bool Selection::IsBeamSelected(int id) const
{
    return std::find(beamIds.begin(), beamIds.end(), id) != beamIds.end();
}

bool Selection::IsDimensionSelected(int id) const
{
    return std::find(dimensionIds.begin(), dimensionIds.end(), id) != dimensionIds.end();
}

void Selection::SelectSingleNode(int id)
{
    Clear();
    nodeIds.push_back(id);
}

void Selection::AddNode(int id)
{
    if (!IsNodeSelected(id))
    {
        nodeIds.push_back(id);
    }
}

void Selection::RemoveNode(int id)
{
    nodeIds.erase(
        std::remove(nodeIds.begin(), nodeIds.end(), id),
        nodeIds.end());
}

void Selection::SelectSingleBeam(int id)
{
    Clear();
    beamIds.push_back(id);
}

void Selection::AddBeam(int id)
{
    if (!IsBeamSelected(id))
    {
        beamIds.push_back(id);
    }
}

void Selection::RemoveBeam(int id)
{
    beamIds.erase(
        std::remove(beamIds.begin(), beamIds.end(), id),
        beamIds.end());
}

void Selection::SelectSingleDimension(int id)
{
    Clear();
    dimensionIds.push_back(id);
}

void Selection::AddDimension(int id)
{
    if (!IsDimensionSelected(id))
    {
        dimensionIds.push_back(id);
    }
}

void Selection::RemoveDimension(int id)
{
    dimensionIds.erase(
        std::remove(dimensionIds.begin(), dimensionIds.end(), id),
        dimensionIds.end());
}
