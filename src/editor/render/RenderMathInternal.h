#pragma once

namespace
{
    float Clamp01(float value)
    {
        if (value < 0.0f) return 0.0f;
        if (value > 1.0f) return 1.0f;
        return value;
    }

    float SmoothStep(float t)
    {
        t = Clamp01(t);
        return t * t * (3.0f - 2.0f * t);
    }

    Color WithScaledAlpha(Color color, float factor)
    {
        color.a = static_cast<unsigned char>(roundf(color.a * Clamp01(factor)));
        return color;
    }

    float VectorLength(Vector2 vector)
    {
        return sqrtf(vector.x * vector.x + vector.y * vector.y);
    }

    Vector2 NormalizeVector(Vector2 vector)
    {
        const float length = VectorLength(vector);
        if (length <= 1.0e-6f)
        {
            return Vector2{0.0f, 0.0f};
        }

        return Vector2{vector.x / length, vector.y / length};
    }

    float SignedTriangleArea2(Vector2 a, Vector2 b, Vector2 c)
    {
        return (b.x - a.x) * (c.y - a.y) - (b.y - a.y) * (c.x - a.x);
    }

    void DrawFilledTriangleSafe(Vector2 a, Vector2 b, Vector2 c, Color color)
    {
        if (SignedTriangleArea2(a, b, c) < 0.0f)
        {
            std::swap(b, c);
        }

        DrawTriangle(a, b, c, color);
        DrawTriangle(a, c, b, color);
    }

    Vector2 NormalizeVectorSafe(Vector2 vector)
    {
        const float length = sqrtf(vector.x * vector.x + vector.y * vector.y);
        if (length <= 1.0e-6f)
        {
            return Vector2{0.0f, 0.0f};
        }

        return Vector2{vector.x / length, vector.y / length};
    }

    Vector2 RotatePointAround(Vector2 point, Vector2 origin, float angleRadians)
    {
        const float rotatedAngle = -angleRadians;
        const float x = point.x - origin.x;
        const float y = point.y - origin.y;

        return Vector2{
            x * cosf(rotatedAngle) + y * sinf(rotatedAngle) + origin.x,
            -x * sinf(rotatedAngle) + y * cosf(rotatedAngle) + origin.y};
    }

    float NormalizeReadableAngle(float angleDegrees)
    {
        constexpr float verticalTolerance = 1.0f;
        constexpr float horizontalTolerance = 1.0f;

        while (angleDegrees <= -180.0f)
        {
            angleDegrees += 360.0f;
        }

        while (angleDegrees > 180.0f)
        {
            angleDegrees -= 360.0f;
        }

        if (angleDegrees < -90.0f)
        {
            return angleDegrees + 180.0f;
        }

        if (angleDegrees >= 90.0f - verticalTolerance)
        {
            return angleDegrees - 180.0f;
        }

        if (fabsf(fabsf(angleDegrees) - 180.0f) <= horizontalTolerance)
        {
            return 0.0f;
        }

        return angleDegrees;
    }
}

