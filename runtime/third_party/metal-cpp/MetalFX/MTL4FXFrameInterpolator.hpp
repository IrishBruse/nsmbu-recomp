

#pragma once

#include "MTLFXDefines.hpp"
#include "MTLFXPrivate.hpp"

#include "MTLFXFrameInterpolator.hpp"
#include "../Metal/Metal.hpp"

namespace MTL4FX
{
    class FrameInterpolator : public NS::Referencing<FrameInterpolator, MTLFX::FrameInterpolatorBase>
    {
    public:
        void encodeToCommandBuffer(MTL4::CommandBuffer* commandBuffer);
    };
}

_MTLFX_INLINE void MTL4FX::FrameInterpolator::encodeToCommandBuffer(MTL4::CommandBuffer* commandBuffer)
{
    return NS::Object::sendMessage< void >( this, _MTLFX_PRIVATE_SEL( encodeToCommandBuffer_ ), commandBuffer );
}
