#pragma once

enum class LengthUnit
{
    Meter,
    Centimeter,
    Millimeter
};

enum class ForceUnit
{
    Newton,
    Kilonewton
};

enum class MomentUnit
{
    NewtonMeter,
    KilonewtonMeter
};

enum class DistributedLoadUnit
{
    NewtonPerMeter,
    KilonewtonPerMeter
};

enum class StressUnit
{
    Pascal,
    Megapascal
};

enum class ElasticModulusUnit
{
    Pascal,
    Megapascal,
    Gigapascal
};

enum class AreaUnit
{
    SquareMeter,
    SquareCentimeter,
    SquareMillimeter
};

enum class InertiaUnit
{
    MeterToFourth,
    CentimeterToFourth,
    MillimeterToFourth
};

class DisplayUnits
{
public:
    LengthUnit length = LengthUnit::Centimeter;
    ForceUnit force = ForceUnit::Kilonewton;
    MomentUnit moment = MomentUnit::KilonewtonMeter;
    DistributedLoadUnit distributedLoad = DistributedLoadUnit::KilonewtonPerMeter;

    StressUnit stress = StressUnit::Megapascal;
    ElasticModulusUnit elasticModulus = ElasticModulusUnit::Gigapascal;

    AreaUnit area = AreaUnit::SquareCentimeter;
    InertiaUnit inertia = InertiaUnit::CentimeterToFourth;
};
