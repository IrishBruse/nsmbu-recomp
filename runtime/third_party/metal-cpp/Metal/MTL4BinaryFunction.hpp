

#pragma once

#include "../Foundation/Foundation.hpp"
#include "MTLDefines.hpp"
#include "MTLHeaderBridge.hpp"
#include "MTLLibrary.hpp"
#include "MTLPrivate.hpp"

namespace MTL4
{

class BinaryFunction : public NS::Referencing<BinaryFunction>
{
public:
    MTL::FunctionType functionType() const;

    NS::String*       name() const;
};

}

_MTL_INLINE MTL::FunctionType MTL4::BinaryFunction::functionType() const
{
    return Object::sendMessage<MTL::FunctionType>(this, _MTL_PRIVATE_SEL(functionType));
}

_MTL_INLINE NS::String* MTL4::BinaryFunction::name() const
{
    return Object::sendMessage<NS::String*>(this, _MTL_PRIVATE_SEL(name));
}
