

#pragma once

#include "../Foundation/Foundation.hpp"
#include "MTLDefines.hpp"
#include "MTLHeaderBridge.hpp"
#include "MTLPrivate.hpp"
#include <cstdint>

namespace MTL
{
class Device;
}

namespace MTL4
{

class CommandAllocatorDescriptor : public NS::Copying<CommandAllocatorDescriptor>
{
public:
    static CommandAllocatorDescriptor* alloc();

    CommandAllocatorDescriptor*        init();

    NS::String*                        label() const;
    void                               setLabel(const NS::String* label);
};

class CommandAllocator : public NS::Referencing<CommandAllocator>
{
public:
    uint64_t     allocatedSize();

    MTL::Device* device() const;

    NS::String*  label() const;

    void         reset();
};

}

_MTL_INLINE MTL4::CommandAllocatorDescriptor* MTL4::CommandAllocatorDescriptor::alloc()
{
    return NS::Object::alloc<MTL4::CommandAllocatorDescriptor>(_MTL_PRIVATE_CLS(MTL4CommandAllocatorDescriptor));
}

_MTL_INLINE MTL4::CommandAllocatorDescriptor* MTL4::CommandAllocatorDescriptor::init()
{
    return NS::Object::init<MTL4::CommandAllocatorDescriptor>();
}

_MTL_INLINE NS::String* MTL4::CommandAllocatorDescriptor::label() const
{
    return Object::sendMessage<NS::String*>(this, _MTL_PRIVATE_SEL(label));
}

_MTL_INLINE void MTL4::CommandAllocatorDescriptor::setLabel(const NS::String* label)
{
    Object::sendMessage<void>(this, _MTL_PRIVATE_SEL(setLabel_), label);
}

_MTL_INLINE uint64_t MTL4::CommandAllocator::allocatedSize()
{
    return Object::sendMessage<uint64_t>(this, _MTL_PRIVATE_SEL(allocatedSize));
}

_MTL_INLINE MTL::Device* MTL4::CommandAllocator::device() const
{
    return Object::sendMessage<MTL::Device*>(this, _MTL_PRIVATE_SEL(device));
}

_MTL_INLINE NS::String* MTL4::CommandAllocator::label() const
{
    return Object::sendMessage<NS::String*>(this, _MTL_PRIVATE_SEL(label));
}

_MTL_INLINE void MTL4::CommandAllocator::reset()
{
    Object::sendMessage<void>(this, _MTL_PRIVATE_SEL(reset));
}
