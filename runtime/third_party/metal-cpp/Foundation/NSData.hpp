

#pragma once

#include "NSObject.hpp"
#include "NSTypes.hpp"

namespace NS
{
class Data : public Copying<Data>
{
public:
    const void*    bytes() const;
    UInteger       length() const;
};
}

_NS_INLINE const void* NS::Data::bytes() const
{
    return Object::sendMessage<void*>(this, _NS_PRIVATE_SEL(bytes));
}

_NS_INLINE NS::UInteger NS::Data::length() const
{
    return Object::sendMessage<UInteger>(this, _NS_PRIVATE_SEL(length));
}

