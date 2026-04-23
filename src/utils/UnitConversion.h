#pragma once

#include "model/DisplayUnits.h"

namespace UnitConversion
{
    const char* GetLengthUnitLabel(LengthUnit unit);
    const char* GetForceUnitLabel(ForceUnit unit);
    const char* GetMomentUnitLabel(MomentUnit unit);
    const char* GetDistributedLoadUnitLabel(DistributedLoadUnit unit);
    const char* GetStressUnitLabel(StressUnit unit);
    const char* GetElasticModulusUnitLabel(ElasticModulusUnit unit);
    const char* GetAreaUnitLabel(AreaUnit unit);
    const char* GetInertiaUnitLabel(InertiaUnit unit);

    double LengthToDisplay(double valueInMeters, LengthUnit unit);
    double LengthToSI(double valueInDisplay, LengthUnit unit);

    double ForceToDisplay(double valueInNewtons, ForceUnit unit);
    double ForceToSI(double valueInDisplay, ForceUnit unit);

    double MomentToDisplay(double valueInNewtonMeters, MomentUnit unit);
    double MomentToSI(double valueInDisplay, MomentUnit unit);

    double DistributedLoadToDisplay(double valueInNewtonsPerMeter, DistributedLoadUnit unit);
    double DistributedLoadToSI(double valueInDisplay, DistributedLoadUnit unit);

    double StressToDisplay(double valueInPascals, StressUnit unit);
    double StressToSI(double valueInDisplay, StressUnit unit);

    double ElasticModulusToDisplay(double valueInPascals, ElasticModulusUnit unit);
    double ElasticModulusToSI(double valueInDisplay, ElasticModulusUnit unit);

    double AreaToDisplay(double valueInSquareMeters, AreaUnit unit);
    double AreaToSI(double valueInDisplay, AreaUnit unit);

    double InertiaToDisplay(double valueInMeterToFourth, InertiaUnit unit);
    double InertiaToSI(double valueInDisplay, InertiaUnit unit);
}
