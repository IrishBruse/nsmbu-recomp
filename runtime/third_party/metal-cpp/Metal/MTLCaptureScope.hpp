

#pragma once

#include "MTLDefines.hpp"
#include "MTLPrivate.hpp"

#include "../Foundation/Foundation.hpp"

namespace MTL
{
class CaptureScope : public NS::Referencing<CaptureScope>
{
public:
    class Device*       device() const;

    NS::String*         label() const;
    void                setLabel(const NS::String* pLabel);

    class CommandQueue* commandQueue() const;

    void                beginScope();
    void                endScope();
};
}

_MTL_INLINE MTL::Device* MTL::CaptureScope::device() const
{
    return Object::sendMessage<Device*>(this, _MTL_PRIVATE_SEL(device));
}

_MTL_INLINE NS::String* MTL::CaptureScope::label() const
{
    return Object::sendMessage<NS::String*>(this, _MTL_PRIVATE_SEL(label));
}

_MTL_INLINE void MTL::CaptureScope::setLabel(const NS::String* pLabel)
{
    Object::sendMessage<void>(this, _MTL_PRIVATE_SEL(setLabel_), pLabel);
}

_MTL_INLINE MTL::CommandQueue* MTL::CaptureScope::commandQueue() const
{
    return Object::sendMessage<CommandQueue*>(this, _MTL_PRIVATE_SEL(commandQueue));
}

_MTL_INLINE void MTL::CaptureScope::beginScope()
{
    return Object::sendMessage<void>(this, _MTL_PRIVATE_SEL(beginScope));
}

_MTL_INLINE void MTL::CaptureScope::endScope()
{
    return Object::sendMessage<void>(this, _MTL_PRIVATE_SEL(endScope));
}

