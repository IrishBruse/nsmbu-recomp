

#pragma once

#include "../Foundation/Foundation.hpp"
#include "MTLDefines.hpp"
#include "MTLHeaderBridge.hpp"
#include "MTLPrivate.hpp"

namespace MTL
{
class Allocation : public NS::Referencing<Allocation>
{
public:
    NS::UInteger allocatedSize() const;
};

}
_MTL_INLINE NS::UInteger MTL::Allocation::allocatedSize() const
{
    return Object::sendMessage<NS::UInteger>(this, _MTL_PRIVATE_SEL(allocatedSize));
}
