#pragma once

#include <cmath>

class DistributedLoadValue
{
public:
    double qxStart = 0.0;
    double qyStart = 0.0;
    double qxEnd = 0.0;
    double qyEnd = 0.0;

public:
    DistributedLoadValue() = default;

    DistributedLoadValue(
        double newQxStart,
        double newQyStart,
        double newQxEnd,
        double newQyEnd)
        : qxStart(newQxStart),
          qyStart(newQyStart),
          qxEnd(newQxEnd),
          qyEnd(newQyEnd)
    {
    }

    bool IsZero(double tolerance = 1.0e-9) const
    {
        return std::abs(qxStart) <= tolerance &&
               std::abs(qyStart) <= tolerance &&
               std::abs(qxEnd) <= tolerance &&
               std::abs(qyEnd) <= tolerance;
    }

    bool IsVariable(double tolerance = 1.0e-9) const
    {
        return std::abs(qxStart - qxEnd) > tolerance ||
               std::abs(qyStart - qyEnd) > tolerance;
    }
};

class BeamDistributedLoad
{
public:
    int id = -1;
    int beamId = -1;
    DistributedLoadValue value;

public:
    BeamDistributedLoad() = default;

    BeamDistributedLoad(int newId, int newBeamId, const DistributedLoadValue& newValue)
        : id(newId),
          beamId(newBeamId),
          value(newValue)
    {
    }
};
