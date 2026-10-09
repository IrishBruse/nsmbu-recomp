

#pragma once

#include "../Foundation/Foundation.hpp"
#include "MTLDefines.hpp"
#include "MTLHeaderBridge.hpp"
#include "MTLPrivate.hpp"

namespace MTL
{
class CounterSampleBuffer;
class ResourceStatePassDescriptor;
class ResourceStatePassSampleBufferAttachmentDescriptor;
class ResourceStatePassSampleBufferAttachmentDescriptorArray;

class ResourceStatePassSampleBufferAttachmentDescriptor : public NS::Copying<ResourceStatePassSampleBufferAttachmentDescriptor>
{
public:
    static ResourceStatePassSampleBufferAttachmentDescriptor* alloc();

    NS::UInteger                                              endOfEncoderSampleIndex() const;

    ResourceStatePassSampleBufferAttachmentDescriptor*        init();

    CounterSampleBuffer*                                      sampleBuffer() const;

    void                                                      setEndOfEncoderSampleIndex(NS::UInteger endOfEncoderSampleIndex);

    void                                                      setSampleBuffer(const MTL::CounterSampleBuffer* sampleBuffer);

    void                                                      setStartOfEncoderSampleIndex(NS::UInteger startOfEncoderSampleIndex);
    NS::UInteger                                              startOfEncoderSampleIndex() const;
};
class ResourceStatePassSampleBufferAttachmentDescriptorArray : public NS::Referencing<ResourceStatePassSampleBufferAttachmentDescriptorArray>
{
public:
    static ResourceStatePassSampleBufferAttachmentDescriptorArray* alloc();

    ResourceStatePassSampleBufferAttachmentDescriptorArray*        init();

    ResourceStatePassSampleBufferAttachmentDescriptor*             object(NS::UInteger attachmentIndex);
    void                                                           setObject(const MTL::ResourceStatePassSampleBufferAttachmentDescriptor* attachment, NS::UInteger attachmentIndex);
};
class ResourceStatePassDescriptor : public NS::Copying<ResourceStatePassDescriptor>
{
public:
    static ResourceStatePassDescriptor*                     alloc();

    ResourceStatePassDescriptor*                            init();

    static ResourceStatePassDescriptor*                     resourceStatePassDescriptor();

    ResourceStatePassSampleBufferAttachmentDescriptorArray* sampleBufferAttachments() const;
};

}
_MTL_INLINE MTL::ResourceStatePassSampleBufferAttachmentDescriptor* MTL::ResourceStatePassSampleBufferAttachmentDescriptor::alloc()
{
    return NS::Object::alloc<MTL::ResourceStatePassSampleBufferAttachmentDescriptor>(_MTL_PRIVATE_CLS(MTLResourceStatePassSampleBufferAttachmentDescriptor));
}

_MTL_INLINE NS::UInteger MTL::ResourceStatePassSampleBufferAttachmentDescriptor::endOfEncoderSampleIndex() const
{
    return Object::sendMessage<NS::UInteger>(this, _MTL_PRIVATE_SEL(endOfEncoderSampleIndex));
}

_MTL_INLINE MTL::ResourceStatePassSampleBufferAttachmentDescriptor* MTL::ResourceStatePassSampleBufferAttachmentDescriptor::init()
{
    return NS::Object::init<MTL::ResourceStatePassSampleBufferAttachmentDescriptor>();
}

_MTL_INLINE MTL::CounterSampleBuffer* MTL::ResourceStatePassSampleBufferAttachmentDescriptor::sampleBuffer() const
{
    return Object::sendMessage<MTL::CounterSampleBuffer*>(this, _MTL_PRIVATE_SEL(sampleBuffer));
}

_MTL_INLINE void MTL::ResourceStatePassSampleBufferAttachmentDescriptor::setEndOfEncoderSampleIndex(NS::UInteger endOfEncoderSampleIndex)
{
    Object::sendMessage<void>(this, _MTL_PRIVATE_SEL(setEndOfEncoderSampleIndex_), endOfEncoderSampleIndex);
}

_MTL_INLINE void MTL::ResourceStatePassSampleBufferAttachmentDescriptor::setSampleBuffer(const MTL::CounterSampleBuffer* sampleBuffer)
{
    Object::sendMessage<void>(this, _MTL_PRIVATE_SEL(setSampleBuffer_), sampleBuffer);
}

_MTL_INLINE void MTL::ResourceStatePassSampleBufferAttachmentDescriptor::setStartOfEncoderSampleIndex(NS::UInteger startOfEncoderSampleIndex)
{
    Object::sendMessage<void>(this, _MTL_PRIVATE_SEL(setStartOfEncoderSampleIndex_), startOfEncoderSampleIndex);
}

_MTL_INLINE NS::UInteger MTL::ResourceStatePassSampleBufferAttachmentDescriptor::startOfEncoderSampleIndex() const
{
    return Object::sendMessage<NS::UInteger>(this, _MTL_PRIVATE_SEL(startOfEncoderSampleIndex));
}

_MTL_INLINE MTL::ResourceStatePassSampleBufferAttachmentDescriptorArray* MTL::ResourceStatePassSampleBufferAttachmentDescriptorArray::alloc()
{
    return NS::Object::alloc<MTL::ResourceStatePassSampleBufferAttachmentDescriptorArray>(_MTL_PRIVATE_CLS(MTLResourceStatePassSampleBufferAttachmentDescriptorArray));
}

_MTL_INLINE MTL::ResourceStatePassSampleBufferAttachmentDescriptorArray* MTL::ResourceStatePassSampleBufferAttachmentDescriptorArray::init()
{
    return NS::Object::init<MTL::ResourceStatePassSampleBufferAttachmentDescriptorArray>();
}

_MTL_INLINE MTL::ResourceStatePassSampleBufferAttachmentDescriptor* MTL::ResourceStatePassSampleBufferAttachmentDescriptorArray::object(NS::UInteger attachmentIndex)
{
    return Object::sendMessage<MTL::ResourceStatePassSampleBufferAttachmentDescriptor*>(this, _MTL_PRIVATE_SEL(objectAtIndexedSubscript_), attachmentIndex);
}

_MTL_INLINE void MTL::ResourceStatePassSampleBufferAttachmentDescriptorArray::setObject(const MTL::ResourceStatePassSampleBufferAttachmentDescriptor* attachment, NS::UInteger attachmentIndex)
{
    Object::sendMessage<void>(this, _MTL_PRIVATE_SEL(setObject_atIndexedSubscript_), attachment, attachmentIndex);
}

_MTL_INLINE MTL::ResourceStatePassDescriptor* MTL::ResourceStatePassDescriptor::alloc()
{
    return NS::Object::alloc<MTL::ResourceStatePassDescriptor>(_MTL_PRIVATE_CLS(MTLResourceStatePassDescriptor));
}

_MTL_INLINE MTL::ResourceStatePassDescriptor* MTL::ResourceStatePassDescriptor::init()
{
    return NS::Object::init<MTL::ResourceStatePassDescriptor>();
}

_MTL_INLINE MTL::ResourceStatePassDescriptor* MTL::ResourceStatePassDescriptor::resourceStatePassDescriptor()
{
    return Object::sendMessage<MTL::ResourceStatePassDescriptor*>(_MTL_PRIVATE_CLS(MTLResourceStatePassDescriptor), _MTL_PRIVATE_SEL(resourceStatePassDescriptor));
}

_MTL_INLINE MTL::ResourceStatePassSampleBufferAttachmentDescriptorArray* MTL::ResourceStatePassDescriptor::sampleBufferAttachments() const
{
    return Object::sendMessage<MTL::ResourceStatePassSampleBufferAttachmentDescriptorArray*>(this, _MTL_PRIVATE_SEL(sampleBufferAttachments));
}
