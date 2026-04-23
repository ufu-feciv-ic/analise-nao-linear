#pragma once

#include <string>

class Section
{
public:
    int id = -1;
    std::string name = "New Section";

    double area = 0.0;
    double inertia = 0.0;

public:
    Section() = default;

    Section(int newId, const std::string& newName)
        : id(newId), name(newName)
    {
    }

    Section(
        int newId,
        const std::string& newName,
        double newArea,
        double newInertia)
        : id(newId),
          name(newName),
          area(newArea),
          inertia(newInertia)
    {
    }
};