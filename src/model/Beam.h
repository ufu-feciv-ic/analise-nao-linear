#pragma once

class Beam
{
public:
    int id = -1;

    int startNodeId = -1;
    int endNodeId = -1;

    int materialId = -1;
    int sectionId = -1;

public:
    Beam() = default;

    Beam(int newId, int newStartNodeId, int newEndNodeId)
        : id(newId),
          startNodeId(newStartNodeId),
          endNodeId(newEndNodeId)
    {
    }

    Beam(
        int newId,
        int newStartNodeId,
        int newEndNodeId,
        int newMaterialId,
        int newSectionId)
        : id(newId),
          startNodeId(newStartNodeId),
          endNodeId(newEndNodeId),
          materialId(newMaterialId),
          sectionId(newSectionId)
    {
    }
};