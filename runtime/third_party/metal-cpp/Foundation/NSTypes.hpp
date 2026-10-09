

#pragma once

#include "NSDefines.hpp"

#include <CoreFoundation/CoreFoundation.h>
#include <cstdint>

namespace NS
{
using TimeInterval = double;

using Integer = std::intptr_t;
using UInteger = std::uintptr_t;

const Integer  IntegerMax = INTPTR_MAX;
const Integer  IntegerMin = INTPTR_MIN;
const UInteger UIntegerMax = UINTPTR_MAX;

struct OperatingSystemVersion
{
    Integer majorVersion;
    Integer minorVersion;
    Integer patchVersion;
} _NS_PACKED;
}

