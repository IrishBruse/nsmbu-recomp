

#pragma once

#include "../Foundation/Foundation.hpp"
#include "MTLDefines.hpp"
#include "MTLHeaderBridge.hpp"
#include "MTLLibrary.hpp"
#include "MTLPrivate.hpp"
#include "MTLTypes.hpp"

namespace MTL
{
class Device;

class FunctionHandle : public NS::Referencing<FunctionHandle>
{
public:
    Device*      device() const;

    FunctionType functionType() const;

    ResourceID   gpuResourceID() const;

    NS::String*  name() const;
};

}
_MTL_INLINE MTL::Device* MTL::FunctionHandle::device() const
{
    return Object::sendMessage<MTL::Device*>(this, _MTL_PRIVATE_SEL(device));
}

_MTL_INLINE MTL::FunctionType MTL::FunctionHandle::functionType() const
{
    return Object::sendMessage<MTL::FunctionType>(this, _MTL_PRIVATE_SEL(functionType));
}

_MTL_INLINE MTL::ResourceID MTL::FunctionHandle::gpuResourceID() const
{
    return Object::sendMessage<MTL::ResourceID>(this, _MTL_PRIVATE_SEL(gpuResourceID));
}

_MTL_INLINE NS::String* MTL::FunctionHandle::name() const
{
    return Object::sendMessage<NS::String*>(this, _MTL_PRIVATE_SEL(name));
}
