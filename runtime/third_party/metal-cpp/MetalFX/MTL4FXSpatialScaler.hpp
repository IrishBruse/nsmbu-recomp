

#pragma once

#include "MTLFXDefines.hpp"
#include "MTLFXPrivate.hpp"

#include "MTLFXSpatialScaler.hpp"
#include "../Metal/Metal.hpp"

namespace MTL4FX
{
    class SpatialScaler : public NS::Referencing< SpatialScaler, MTLFX::SpatialScalerBase >
    {
    public:
        void encodeToCommandBuffer( MTL4::CommandBuffer* pCommandBuffer );
    };
}

_MTLFX_INLINE void MTL4FX::SpatialScaler::encodeToCommandBuffer( MTL4::CommandBuffer* pCommandBuffer )
{
    Object::sendMessage< void >( this, _MTLFX_PRIVATE_SEL( encodeToCommandBuffer_ ), pCommandBuffer );
}

