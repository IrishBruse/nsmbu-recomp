

#pragma once

#include "NSDefines.hpp"
#include "NSTypes.hpp"

namespace NS
{
struct Range
{
    static Range Make(UInteger loc, UInteger len);

    Range(UInteger loc, UInteger len);

    bool     Equal(const Range& range) const;
    bool     LocationInRange(UInteger loc) const;
    UInteger Max() const;

    UInteger location;
    UInteger length;
} _NS_PACKED;
}

_NS_INLINE NS::Range::Range(UInteger loc, UInteger len)
    : location(loc)
    , length(len)
{
}

_NS_INLINE NS::Range NS::Range::Make(UInteger loc, UInteger len)
{
    return Range(loc, len);
}

_NS_INLINE bool NS::Range::Equal(const Range& range) const
{
    return (location == range.location) && (length == range.length);
}

_NS_INLINE bool NS::Range::LocationInRange(UInteger loc) const
{
    return (!(loc < location)) && ((loc - location) < length);
}

_NS_INLINE NS::UInteger NS::Range::Max() const
{
    return location + length;
}

