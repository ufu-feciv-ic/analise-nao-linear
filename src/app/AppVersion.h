#pragma once

namespace AppVersion
{
    constexpr int Major = 0;
    constexpr int Minor = 3;
    constexpr int Patch = 1;

    constexpr char String[] = "0.3.1";

    constexpr int CurrentProjectFileFormatVersion = 1;

    constexpr bool IsSupportedProjectFileFormatVersion(int version)
    {
        return version == CurrentProjectFileFormatVersion;
    }
}
