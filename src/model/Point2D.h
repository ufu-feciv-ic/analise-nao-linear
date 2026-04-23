#pragma once

class Point2D
{
public:
    double x = 0.0;
    double y = 0.0;

public:
    Point2D() = default;

    Point2D(double xValue, double yValue)
        : x(xValue), y(yValue)
    {
    }
};