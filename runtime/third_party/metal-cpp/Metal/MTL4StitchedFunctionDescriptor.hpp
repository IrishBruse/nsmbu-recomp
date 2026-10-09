

#pragma once

#include "../Foundation/Foundation.hpp"
#include "MTL4FunctionDescriptor.hpp"
#include "MTLDefines.hpp"
#include "MTLHeaderBridge.hpp"
#include "MTLPrivate.hpp"

namespace MTL4
{
class StitchedFunctionDescriptor;
}

namespace MTL
{
class FunctionStitchingGraph;
}

namespace MTL4
{
class StitchedFunctionDescriptor : public NS::Copying<StitchedFunctionDescriptor, FunctionDescriptor>
{
public:
    static StitchedFunctionDescriptor* alloc();

    NS::Array*                         functionDescriptors() const;

    MTL::FunctionStitchingGraph*       functionGraph() const;

    StitchedFunctionDescriptor*        init();

    void                               setFunctionDescriptors(const NS::Array* functionDescriptors);

    void                               setFunctionGraph(const MTL::FunctionStitchingGraph* functionGraph);
};

}
_MTL_INLINE MTL4::StitchedFunctionDescriptor* MTL4::StitchedFunctionDescriptor::alloc()
{
    return NS::Object::alloc<MTL4::StitchedFunctionDescriptor>(_MTL_PRIVATE_CLS(MTL4StitchedFunctionDescriptor));
}

_MTL_INLINE NS::Array* MTL4::StitchedFunctionDescriptor::functionDescriptors() const
{
    return Object::sendMessage<NS::Array*>(this, _MTL_PRIVATE_SEL(functionDescriptors));
}

_MTL_INLINE MTL::FunctionStitchingGraph* MTL4::StitchedFunctionDescriptor::functionGraph() const
{
    return Object::sendMessage<MTL::FunctionStitchingGraph*>(this, _MTL_PRIVATE_SEL(functionGraph));
}

_MTL_INLINE MTL4::StitchedFunctionDescriptor* MTL4::StitchedFunctionDescriptor::init()
{
    return NS::Object::init<MTL4::StitchedFunctionDescriptor>();
}

_MTL_INLINE void MTL4::StitchedFunctionDescriptor::setFunctionDescriptors(const NS::Array* functionDescriptors)
{
    Object::sendMessage<void>(this, _MTL_PRIVATE_SEL(setFunctionDescriptors_), functionDescriptors);
}

_MTL_INLINE void MTL4::StitchedFunctionDescriptor::setFunctionGraph(const MTL::FunctionStitchingGraph* functionGraph)
{
    Object::sendMessage<void>(this, _MTL_PRIVATE_SEL(setFunctionGraph_), functionGraph);
}
