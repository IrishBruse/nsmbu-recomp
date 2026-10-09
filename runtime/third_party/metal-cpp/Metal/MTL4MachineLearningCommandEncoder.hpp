

#pragma once

#include "../Foundation/Foundation.hpp"
#include "MTL4CommandEncoder.hpp"
#include "MTLDefines.hpp"
#include "MTLHeaderBridge.hpp"
#include "MTLPrivate.hpp"

namespace MTL4
{
class ArgumentTable;
class MachineLearningPipelineState;
}

namespace MTL
{
class Heap;
}

namespace MTL4
{
class MachineLearningCommandEncoder : public NS::Referencing<MachineLearningCommandEncoder, CommandEncoder>
{
public:
    void dispatchNetwork(const MTL::Heap* heap);

    void setArgumentTable(const MTL4::ArgumentTable* argumentTable);

    void setPipelineState(const MTL4::MachineLearningPipelineState* pipelineState);
};

}
_MTL_INLINE void MTL4::MachineLearningCommandEncoder::dispatchNetwork(const MTL::Heap* heap)
{
    Object::sendMessage<void>(this, _MTL_PRIVATE_SEL(dispatchNetworkWithIntermediatesHeap_), heap);
}

_MTL_INLINE void MTL4::MachineLearningCommandEncoder::setArgumentTable(const MTL4::ArgumentTable* argumentTable)
{
    Object::sendMessage<void>(this, _MTL_PRIVATE_SEL(setArgumentTable_), argumentTable);
}

_MTL_INLINE void MTL4::MachineLearningCommandEncoder::setPipelineState(const MTL4::MachineLearningPipelineState* pipelineState)
{
    Object::sendMessage<void>(this, _MTL_PRIVATE_SEL(setPipelineState_), pipelineState);
}
