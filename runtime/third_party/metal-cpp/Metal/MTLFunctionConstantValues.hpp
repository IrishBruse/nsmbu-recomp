

#pragma once

#include "../Foundation/Foundation.hpp"
#include "MTLDataType.hpp"
#include "MTLDefines.hpp"
#include "MTLHeaderBridge.hpp"
#include "MTLPrivate.hpp"

namespace MTL
{
class FunctionConstantValues;

class FunctionConstantValues : public NS::Copying<FunctionConstantValues>
{
public:
    static FunctionConstantValues* alloc();

    FunctionConstantValues*        init();

    void                           reset();

    void                           setConstantValue(const void* value, MTL::DataType type, NS::UInteger index);
    void                           setConstantValue(const void* value, MTL::DataType type, const NS::String* name);
    void                           setConstantValues(const void* values, MTL::DataType type, NS::Range range);
};

}
_MTL_INLINE MTL::FunctionConstantValues* MTL::FunctionConstantValues::alloc()
{
    return NS::Object::alloc<MTL::FunctionConstantValues>(_MTL_PRIVATE_CLS(MTLFunctionConstantValues));
}

_MTL_INLINE MTL::FunctionConstantValues* MTL::FunctionConstantValues::init()
{
    return NS::Object::init<MTL::FunctionConstantValues>();
}

_MTL_INLINE void MTL::FunctionConstantValues::reset()
{
    Object::sendMessage<void>(this, _MTL_PRIVATE_SEL(reset));
}

_MTL_INLINE void MTL::FunctionConstantValues::setConstantValue(const void* value, MTL::DataType type, NS::UInteger index)
{
    Object::sendMessage<void>(this, _MTL_PRIVATE_SEL(setConstantValue_type_atIndex_), value, type, index);
}

_MTL_INLINE void MTL::FunctionConstantValues::setConstantValue(const void* value, MTL::DataType type, const NS::String* name)
{
    Object::sendMessage<void>(this, _MTL_PRIVATE_SEL(setConstantValue_type_withName_), value, type, name);
}

_MTL_INLINE void MTL::FunctionConstantValues::setConstantValues(const void* values, MTL::DataType type, NS::Range range)
{
    Object::sendMessage<void>(this, _MTL_PRIVATE_SEL(setConstantValues_type_withRange_), values, type, range);
}
