

#pragma once

#include "NSDefines.hpp"
#include "NSTypes.hpp"

namespace NS
{

_NS_ENUM(Integer, ComparisonResult) {
    OrderedAscending = -1L,
    OrderedSame,
    OrderedDescending
};

const Integer NotFound = IntegerMax;

}

