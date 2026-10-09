

#pragma once

#include "../Metal/MTLDrawable.hpp"
#include "../Metal/MTLTexture.hpp"

#include "CADefines.hpp"
#include "CAPrivate.hpp"

namespace CA
{
class MetalDrawable : public NS::Referencing<MetalDrawable, MTL::Drawable>
{
public:
    class MetalLayer* layer() const;
    MTL::Texture*     texture() const;
};
}

_CA_INLINE CA::MetalLayer* CA::MetalDrawable::layer() const
{
    return Object::sendMessage<MetalLayer*>(this, _CA_PRIVATE_SEL(layer));
}

_CA_INLINE MTL::Texture* CA::MetalDrawable::texture() const
{
    return Object::sendMessage<MTL::Texture*>(this, _CA_PRIVATE_SEL(texture));
}

