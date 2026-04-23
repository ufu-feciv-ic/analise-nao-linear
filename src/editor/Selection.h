#pragma once

#include <vector>

struct Selection
{
    std::vector<int> nodeIds;
    std::vector<int> beamIds;
    std::vector<int> dimensionIds;

    bool HasAny() const;
    bool HasNodes() const;
    bool HasBeams() const;
    bool HasDimensions() const;

    void Clear();
    void ClearNodes();
    void ClearBeams();
    void ClearDimensions();

    bool IsNodeSelected(int id) const;
    bool IsBeamSelected(int id) const;
    bool IsDimensionSelected(int id) const;

    void SelectSingleNode(int id);
    void AddNode(int id);
    void RemoveNode(int id);

    void SelectSingleBeam(int id);
    void AddBeam(int id);
    void RemoveBeam(int id);

    void SelectSingleDimension(int id);
    void AddDimension(int id);
    void RemoveDimension(int id);
};
