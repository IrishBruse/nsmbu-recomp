

#pragma once

#include "../Foundation/Foundation.hpp"
#include "MTLCommandBuffer.hpp"
#include "MTLDefines.hpp"
#include "MTLHeaderBridge.hpp"
#include "MTLPrivate.hpp"

namespace MTL
{
class ComputePassDescriptor;
class ComputePassSampleBufferAttachmentDescriptor;
class ComputePassSampleBufferAttachmentDescriptorArray;
class CounterSampleBuffer;

class ComputePassSampleBufferAttachmentDescriptor : public NS::Copying<ComputePassSampleBufferAttachmentDescriptor>
{
public:
    static ComputePassSampleBufferAttachmentDescriptor* alloc();

    NS::UInteger                                        endOfEncoderSampleIndex() const;

    ComputePassSampleBufferAttachmentDescriptor*        init();

    CounterSampleBuffer*                                sampleBuffer() const;

    void                                                setEndOfEncoderSampleIndex(NS::UInteger endOfEncoderSampleIndex);

    void                                                setSampleBuffer(const MTL::CounterSampleBuffer* sampleBuffer);

    void                                                setStartOfEncoderSampleIndex(NS::UInteger startOfEncoderSampleIndex);
    NS::UInteger                                        startOfEncoderSampleIndex() const;
};
class ComputePassSampleBufferAttachmentDescriptorArray : public NS::Referencing<ComputePassSampleBufferAttachmentDescriptorArray>
{
public:
    static ComputePassSampleBufferAttachmentDescriptorArray* alloc();

    ComputePassSampleBufferAttachmentDescriptorArray*        init();

    ComputePassSampleBufferAttachmentDescriptor*             object(NS::UInteger attachmentIndex);
    void                                                     setObject(const MTL::ComputePassSampleBufferAttachmentDescriptor* attachment, NS::UInteger attachmentIndex);
};
class ComputePassDescriptor : public NS::Copying<ComputePassDescriptor>
{
public:
    static ComputePassDescriptor*                     alloc();

    static ComputePassDescriptor*                     computePassDescriptor();

    DispatchType                                      dispatchType() const;

    ComputePassDescriptor*                            init();

    ComputePassSampleBufferAttachmentDescriptorArray* sampleBufferAttachments() const;

    void                                              setDispatchType(MTL::DispatchType dispatchType);
};

}
_MTL_INLINE MTL::ComputePassSampleBufferAttachmentDescriptor* MTL::ComputePassSampleBufferAttachmentDescriptor::alloc()
{
    return NS::Object::alloc<MTL::ComputePassSampleBufferAttachmentDescriptor>(_MTL_PRIVATE_CLS(MTLComputePassSampleBufferAttachmentDescriptor));
}

_MTL_INLINE NS::UInteger MTL::ComputePassSampleBufferAttachmentDescriptor::endOfEncoderSampleIndex() const
{
    return Object::sendMessage<NS::UInteger>(this, _MTL_PRIVATE_SEL(endOfEncoderSampleIndex));
}

_MTL_INLINE MTL::ComputePassSampleBufferAttachmentDescriptor* MTL::ComputePassSampleBufferAttachmentDescriptor::init()
{
    return NS::Object::init<MTL::ComputePassSampleBufferAttachmentDescriptor>();
}

_MTL_INLINE MTL::CounterSampleBuffer* MTL::ComputePassSampleBufferAttachmentDescriptor::sampleBuffer() const
{
    return Object::sendMessage<MTL::CounterSampleBuffer*>(this, _MTL_PRIVATE_SEL(sampleBuffer));
}

_MTL_INLINE void MTL::ComputePassSampleBufferAttachmentDescriptor::setEndOfEncoderSampleIndex(NS::UInteger endOfEncoderSampleIndex)
{
    Object::sendMessage<void>(this, _MTL_PRIVATE_SEL(setEndOfEncoderSampleIndex_), endOfEncoderSampleIndex);
}

_MTL_INLINE void MTL::ComputePassSampleBufferAttachmentDescriptor::setSampleBuffer(const MTL::CounterSampleBuffer* sampleBuffer)
{
    Object::sendMessage<void>(this, _MTL_PRIVATE_SEL(setSampleBuffer_), sampleBuffer);
}

_MTL_INLINE void MTL::ComputePassSampleBufferAttachmentDescriptor::setStartOfEncoderSampleIndex(NS::UInteger startOfEncoderSampleIndex)
{
    Object::sendMessage<void>(this, _MTL_PRIVATE_SEL(setStartOfEncoderSampleIndex_), startOfEncoderSampleIndex);
}

_MTL_INLINE NS::UInteger MTL::ComputePassSampleBufferAttachmentDescriptor::startOfEncoderSampleIndex() const
{
    return Object::sendMessage<NS::UInteger>(this, _MTL_PRIVATE_SEL(startOfEncoderSampleIndex));
}

_MTL_INLINE MTL::ComputePassSampleBufferAttachmentDescriptorArray* MTL::ComputePassSampleBufferAttachmentDescriptorArray::alloc()
{
    return NS::Object::alloc<MTL::ComputePassSampleBufferAttachmentDescriptorArray>(_MTL_PRIVATE_CLS(MTLComputePassSampleBufferAttachmentDescriptorArray));
}

_MTL_INLINE MTL::ComputePassSampleBufferAttachmentDescriptorArray* MTL::ComputePassSampleBufferAttachmentDescriptorArray::init()
{
    return NS::Object::init<MTL::ComputePassSampleBufferAttachmentDescriptorArray>();
}

_MTL_INLINE MTL::ComputePassSampleBufferAttachmentDescriptor* MTL::ComputePassSampleBufferAttachmentDescriptorArray::object(NS::UInteger attachmentIndex)
{
    return Object::sendMessage<MTL::ComputePassSampleBufferAttachmentDescriptor*>(this, _MTL_PRIVATE_SEL(objectAtIndexedSubscript_), attachmentIndex);
}

_MTL_INLINE void MTL::ComputePassSampleBufferAttachmentDescriptorArray::setObject(const MTL::ComputePassSampleBufferAttachmentDescriptor* attachment, NS::UInteger attachmentIndex)
{
    Object::sendMessage<void>(this, _MTL_PRIVATE_SEL(setObject_atIndexedSubscript_), attachment, attachmentIndex);
}

_MTL_INLINE MTL::ComputePassDescriptor* MTL::ComputePassDescriptor::alloc()
{
    return NS::Object::alloc<MTL::ComputePassDescriptor>(_MTL_PRIVATE_CLS(MTLComputePassDescriptor));
}

_MTL_INLINE MTL::ComputePassDescriptor* MTL::ComputePassDescriptor::computePassDescriptor()
{
    return Object::sendMessage<MTL::ComputePassDescriptor*>(_MTL_PRIVATE_CLS(MTLComputePassDescriptor), _MTL_PRIVATE_SEL(computePassDescriptor));
}

_MTL_INLINE MTL::DispatchType MTL::ComputePassDescriptor::dispatchType() const
{
    return Object::sendMessage<MTL::DispatchType>(this, _MTL_PRIVATE_SEL(dispatchType));
}

_MTL_INLINE MTL::ComputePassDescriptor* MTL::ComputePassDescriptor::init()
{
    return NS::Object::init<MTL::ComputePassDescriptor>();
}

_MTL_INLINE MTL::ComputePassSampleBufferAttachmentDescriptorArray* MTL::ComputePassDescriptor::sampleBufferAttachments() const
{
    return Object::sendMessage<MTL::ComputePassSampleBufferAttachmentDescriptorArray*>(this, _MTL_PRIVATE_SEL(sampleBufferAttachments));
}

_MTL_INLINE void MTL::ComputePassDescriptor::setDispatchType(MTL::DispatchType dispatchType)
{
    Object::sendMessage<void>(this, _MTL_PRIVATE_SEL(setDispatchType_), dispatchType);
}
