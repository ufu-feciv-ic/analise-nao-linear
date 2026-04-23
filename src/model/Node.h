#pragma once

#include "model/Point2D.h"

enum class SupportType
{
    None,
    RestrainedX,
    RestrainedY,
    RestrainedXY,
    Fixed
};

class NodalLoad
{
public:
    double fx = 0.0;
    double fy = 0.0;
    double mz = 0.0;

public:
    NodalLoad() = default;

    NodalLoad(double fxValue, double fyValue, double mzValue)
        : fx(fxValue), fy(fyValue), mz(mzValue)
    {
    }
};

class Node
{
public:
    int id = -1;
    Point2D position;
    SupportType support = SupportType::None;
    NodalLoad load;

public:
    Node() = default;

    Node(int newId, double xValue, double yValue)
        : id(newId), position(xValue, yValue)
    {
    }

    Node(int newId, const Point2D& newPosition)
        : id(newId), position(newPosition)
    {
    }
};
