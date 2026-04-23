#include "utils/UnitConversion.h"

namespace UnitConversion
{
    const char* GetLengthUnitLabel(LengthUnit unit)
    {
        switch (unit)
        {
        case LengthUnit::Meter:      return "m";
        case LengthUnit::Centimeter: return "cm";
        case LengthUnit::Millimeter: return "mm";
        default:                     return "";
        }
    }

    const char* GetForceUnitLabel(ForceUnit unit)
    {
        switch (unit)
        {
        case ForceUnit::Newton:     return "N";
        case ForceUnit::Kilonewton: return "kN";
        default:                    return "";
        }
    }

    const char* GetMomentUnitLabel(MomentUnit unit)
    {
        switch (unit)
        {
        case MomentUnit::NewtonMeter:     return "N.m";
        case MomentUnit::KilonewtonMeter: return "kN.m";
        default:                          return "";
        }
    }

    const char* GetDistributedLoadUnitLabel(DistributedLoadUnit unit)
    {
        switch (unit)
        {
        case DistributedLoadUnit::NewtonPerMeter:     return "N/m";
        case DistributedLoadUnit::KilonewtonPerMeter: return "kN/m";
        default:                                      return "";
        }
    }

    const char* GetStressUnitLabel(StressUnit unit)
    {
        switch (unit)
        {
        case StressUnit::Pascal:     return "Pa";
        case StressUnit::Megapascal: return "MPa";
        default:                     return "";
        }
    }

    const char* GetElasticModulusUnitLabel(ElasticModulusUnit unit)
    {
        switch (unit)
        {
        case ElasticModulusUnit::Pascal:     return "Pa";
        case ElasticModulusUnit::Megapascal: return "MPa";
        case ElasticModulusUnit::Gigapascal: return "GPa";
        default:                             return "";
        }
    }

    const char* GetAreaUnitLabel(AreaUnit unit)
    {
        switch (unit)
        {
        case AreaUnit::SquareMeter:       return "m2";
        case AreaUnit::SquareCentimeter:  return "cm2";
        case AreaUnit::SquareMillimeter:  return "mm2";
        default:                          return "";
        }
    }

    const char* GetInertiaUnitLabel(InertiaUnit unit)
    {
        switch (unit)
        {
        case InertiaUnit::MeterToFourth:      return "m4";
        case InertiaUnit::CentimeterToFourth: return "cm4";
        case InertiaUnit::MillimeterToFourth: return "mm4";
        default:                              return "";
        }
    }

    double LengthToDisplay(double valueInMeters, LengthUnit unit)
    {
        switch (unit)
        {
        case LengthUnit::Meter:      return valueInMeters;
        case LengthUnit::Centimeter: return valueInMeters * 100.0;
        case LengthUnit::Millimeter: return valueInMeters * 1000.0;
        default:                     return valueInMeters;
        }
    }

    double LengthToSI(double valueInDisplay, LengthUnit unit)
    {
        switch (unit)
        {
        case LengthUnit::Meter:      return valueInDisplay;
        case LengthUnit::Centimeter: return valueInDisplay / 100.0;
        case LengthUnit::Millimeter: return valueInDisplay / 1000.0;
        default:                     return valueInDisplay;
        }
    }

    double ForceToDisplay(double valueInNewtons, ForceUnit unit)
    {
        switch (unit)
        {
        case ForceUnit::Newton:     return valueInNewtons;
        case ForceUnit::Kilonewton: return valueInNewtons / 1000.0;
        default:                    return valueInNewtons;
        }
    }

    double ForceToSI(double valueInDisplay, ForceUnit unit)
    {
        switch (unit)
        {
        case ForceUnit::Newton:     return valueInDisplay;
        case ForceUnit::Kilonewton: return valueInDisplay * 1000.0;
        default:                    return valueInDisplay;
        }
    }

    double MomentToDisplay(double valueInNewtonMeters, MomentUnit unit)
    {
        switch (unit)
        {
        case MomentUnit::NewtonMeter:     return valueInNewtonMeters;
        case MomentUnit::KilonewtonMeter: return valueInNewtonMeters / 1000.0;
        default:                          return valueInNewtonMeters;
        }
    }

    double MomentToSI(double valueInDisplay, MomentUnit unit)
    {
        switch (unit)
        {
        case MomentUnit::NewtonMeter:     return valueInDisplay;
        case MomentUnit::KilonewtonMeter: return valueInDisplay * 1000.0;
        default:                          return valueInDisplay;
        }
    }

    double DistributedLoadToDisplay(double valueInNewtonsPerMeter, DistributedLoadUnit unit)
    {
        switch (unit)
        {
        case DistributedLoadUnit::NewtonPerMeter:     return valueInNewtonsPerMeter;
        case DistributedLoadUnit::KilonewtonPerMeter: return valueInNewtonsPerMeter / 1000.0;
        default:                                      return valueInNewtonsPerMeter;
        }
    }

    double DistributedLoadToSI(double valueInDisplay, DistributedLoadUnit unit)
    {
        switch (unit)
        {
        case DistributedLoadUnit::NewtonPerMeter:     return valueInDisplay;
        case DistributedLoadUnit::KilonewtonPerMeter: return valueInDisplay * 1000.0;
        default:                                      return valueInDisplay;
        }
    }

    double StressToDisplay(double valueInPascals, StressUnit unit)
    {
        switch (unit)
        {
        case StressUnit::Pascal:     return valueInPascals;
        case StressUnit::Megapascal: return valueInPascals / 1.0e6;
        default:                     return valueInPascals;
        }
    }

    double StressToSI(double valueInDisplay, StressUnit unit)
    {
        switch (unit)
        {
        case StressUnit::Pascal:     return valueInDisplay;
        case StressUnit::Megapascal: return valueInDisplay * 1.0e6;
        default:                     return valueInDisplay;
        }
    }

    double ElasticModulusToDisplay(double valueInPascals, ElasticModulusUnit unit)
    {
        switch (unit)
        {
        case ElasticModulusUnit::Pascal:     return valueInPascals;
        case ElasticModulusUnit::Megapascal: return valueInPascals / 1.0e6;
        case ElasticModulusUnit::Gigapascal: return valueInPascals / 1.0e9;
        default:                             return valueInPascals;
        }
    }

    double ElasticModulusToSI(double valueInDisplay, ElasticModulusUnit unit)
    {
        switch (unit)
        {
        case ElasticModulusUnit::Pascal:     return valueInDisplay;
        case ElasticModulusUnit::Megapascal: return valueInDisplay * 1.0e6;
        case ElasticModulusUnit::Gigapascal: return valueInDisplay * 1.0e9;
        default:                             return valueInDisplay;
        }
    }

    double AreaToDisplay(double valueInSquareMeters, AreaUnit unit)
    {
        switch (unit)
        {
        case AreaUnit::SquareMeter:      return valueInSquareMeters;
        case AreaUnit::SquareCentimeter: return valueInSquareMeters * 1.0e4;
        case AreaUnit::SquareMillimeter: return valueInSquareMeters * 1.0e6;
        default:                         return valueInSquareMeters;
        }
    }

    double AreaToSI(double valueInDisplay, AreaUnit unit)
    {
        switch (unit)
        {
        case AreaUnit::SquareMeter:      return valueInDisplay;
        case AreaUnit::SquareCentimeter: return valueInDisplay / 1.0e4;
        case AreaUnit::SquareMillimeter: return valueInDisplay / 1.0e6;
        default:                         return valueInDisplay;
        }
    }

    double InertiaToDisplay(double valueInMeterToFourth, InertiaUnit unit)
    {
        switch (unit)
        {
        case InertiaUnit::MeterToFourth:      return valueInMeterToFourth;
        case InertiaUnit::CentimeterToFourth: return valueInMeterToFourth * 1.0e8;
        case InertiaUnit::MillimeterToFourth: return valueInMeterToFourth * 1.0e12;
        default:                              return valueInMeterToFourth;
        }
    }

    double InertiaToSI(double valueInDisplay, InertiaUnit unit)
    {
        switch (unit)
        {
        case InertiaUnit::MeterToFourth:      return valueInDisplay;
        case InertiaUnit::CentimeterToFourth: return valueInDisplay / 1.0e8;
        case InertiaUnit::MillimeterToFourth: return valueInDisplay / 1.0e12;
        default:                              return valueInDisplay;
        }
    }
}
