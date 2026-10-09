

#pragma once

#include "MTLFXDefines.hpp"
#include "MTLFXPrivate.hpp"

#include "MTLFXTemporalDenoisedScaler.hpp"
#include "../Metal/Metal.hpp"

#include <simd/simd.h>

namespace MTL4FX
{
    class TemporalDenoisedScaler : public NS::Referencing< TemporalDenoisedScaler, MTLFX::TemporalDenoisedScalerBase >
    {
    public:
        void encodeToCommandBuffer(MTL4::CommandBuffer* commandBuffer);
    };
}

_MTLFX_INLINE void MTL4FX::TemporalDenoisedScaler::encodeToCommandBuffer( MTL4::CommandBuffer* commandBuffer )
{
    return NS::Object::sendMessage< void >( this, _MTLFX_PRIVATE_SEL( encodeToCommandBuffer_ ), commandBuffer );
}
