

#pragma once

#include "../Foundation/Foundation.hpp"
#include "MTLDefines.hpp"
#include "MTLHeaderBridge.hpp"
#include "MTLPrivate.hpp"
#include "MTLResourceViewPool.hpp"
#include "MTLTypes.hpp"

namespace MTL
{
class Buffer;
class Texture;
class TextureDescriptor;
class TextureViewDescriptor;

class TextureViewPool : public NS::Referencing<TextureViewPool, ResourceViewPool>
{
public:
    ResourceID setTextureView(const MTL::Texture* texture, NS::UInteger index);
    ResourceID setTextureView(const MTL::Texture* texture, const MTL::TextureViewDescriptor* descriptor, NS::UInteger index);
    ResourceID setTextureViewFromBuffer(const MTL::Buffer* buffer, const MTL::TextureDescriptor* descriptor, NS::UInteger offset, NS::UInteger bytesPerRow, NS::UInteger index);
};

}
_MTL_INLINE MTL::ResourceID MTL::TextureViewPool::setTextureView(const MTL::Texture* texture, NS::UInteger index)
{
    return Object::sendMessage<MTL::ResourceID>(this, _MTL_PRIVATE_SEL(setTextureView_atIndex_), texture, index);
}

_MTL_INLINE MTL::ResourceID MTL::TextureViewPool::setTextureView(const MTL::Texture* texture, const MTL::TextureViewDescriptor* descriptor, NS::UInteger index)
{
    return Object::sendMessage<MTL::ResourceID>(this, _MTL_PRIVATE_SEL(setTextureView_descriptor_atIndex_), texture, descriptor, index);
}

_MTL_INLINE MTL::ResourceID MTL::TextureViewPool::setTextureViewFromBuffer(const MTL::Buffer* buffer, const MTL::TextureDescriptor* descriptor, NS::UInteger offset, NS::UInteger bytesPerRow, NS::UInteger index)
{
    return Object::sendMessage<MTL::ResourceID>(this, _MTL_PRIVATE_SEL(setTextureViewFromBuffer_descriptor_offset_bytesPerRow_atIndex_), buffer, descriptor, offset, bytesPerRow, index);
}
