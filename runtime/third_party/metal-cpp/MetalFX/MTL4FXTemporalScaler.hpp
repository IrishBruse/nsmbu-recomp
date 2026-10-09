

#pragma once

#include "MTLFXDefines.hpp"
#include "MTLFXPrivate.hpp"

#include "MTLFXTemporalScaler.hpp"
#include "../Metal/Metal.hpp"

namespace MTL4FX
{
    class TemporalScaler : public NS::Referencing< TemporalScaler, MTLFX::TemporalScalerBase >
    {
    public:
        void encodeToCommandBuffer( MTL4::CommandBuffer* pCommandBuffer );
    };
}

_MTLFX_INLINE void MTL4FX::TemporalScaler::encodeToCommandBuffer( MTL4::CommandBuffer* pCommandBuffer )
{
    Object::sendMessage< void >( this, _MTLFX_PRIVATE_SEL( encodeToCommandBuffer_ ), pCommandBuffer );
}

