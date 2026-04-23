#pragma once

#include <string>

class StructuralMaterial
{
public:
    int id = -1;
    std::string name = "New Material";

    double youngModulus = 0.0;
    double thermalExpansion = 0.0;

public:
    StructuralMaterial() = default;

    StructuralMaterial(int newId, const std::string& newName)
        : id(newId), name(newName)
    {
    }

    StructuralMaterial(
        int newId,
        const std::string& newName,
        double newYoungModulus,
        double newThermalExpansion)
        : id(newId),
          name(newName),
          youngModulus(newYoungModulus),
          thermalExpansion(newThermalExpansion)
    {
    }
};