

#pragma once

#include "../Foundation/Foundation.hpp"
#include "MTL4FunctionDescriptor.hpp"
#include "MTLDefines.hpp"
#include "MTLHeaderBridge.hpp"
#include "MTLPrivate.hpp"

namespace MTL4
{
class FunctionDescriptor;
class SpecializedFunctionDescriptor;
}

namespace MTL
{
class FunctionConstantValues;
}

namespace MTL4
{
class SpecializedFunctionDescriptor : public NS::Copying<SpecializedFunctionDescriptor, FunctionDescriptor>
{
public:
    static SpecializedFunctionDescriptor* alloc();

    MTL::FunctionConstantValues*          constantValues() const;

    FunctionDescriptor*                   functionDescriptor() const;

    SpecializedFunctionDescriptor*        init();

    void                                  setConstantValues(const MTL::FunctionConstantValues* constantValues);

    void                                  setFunctionDescriptor(const MTL4::FunctionDescriptor* functionDescriptor);

    void                                  setSpecializedName(const NS::String* specializedName);
    NS::String*                           specializedName() const;
};

}
_MTL_INLINE MTL4::SpecializedFunctionDescriptor* MTL4::SpecializedFunctionDescriptor::alloc()
{
    return NS::Object::alloc<MTL4::SpecializedFunctionDescriptor>(_MTL_PRIVATE_CLS(MTL4SpecializedFunctionDescriptor));
}

_MTL_INLINE MTL::FunctionConstantValues* MTL4::SpecializedFunctionDescriptor::constantValues() const
{
    return Object::sendMessage<MTL::FunctionConstantValues*>(this, _MTL_PRIVATE_SEL(constantValues));
}

_MTL_INLINE MTL4::FunctionDescriptor* MTL4::SpecializedFunctionDescriptor::functionDescriptor() const
{
    return Object::sendMessage<MTL4::FunctionDescriptor*>(this, _MTL_PRIVATE_SEL(functionDescriptor));
}

_MTL_INLINE MTL4::SpecializedFunctionDescriptor* MTL4::SpecializedFunctionDescriptor::init()
{
    return NS::Object::init<MTL4::SpecializedFunctionDescriptor>();
}

_MTL_INLINE void MTL4::SpecializedFunctionDescriptor::setConstantValues(const MTL::FunctionConstantValues* constantValues)
{
    Object::sendMessage<void>(this, _MTL_PRIVATE_SEL(setConstantValues_), constantValues);
}

_MTL_INLINE void MTL4::SpecializedFunctionDescriptor::setFunctionDescriptor(const MTL4::FunctionDescriptor* functionDescriptor)
{
    Object::sendMessage<void>(this, _MTL_PRIVATE_SEL(setFunctionDescriptor_), functionDescriptor);
}

_MTL_INLINE void MTL4::SpecializedFunctionDescriptor::setSpecializedName(const NS::String* specializedName)
{
    Object::sendMessage<void>(this, _MTL_PRIVATE_SEL(setSpecializedName_), specializedName);
}

_MTL_INLINE NS::String* MTL4::SpecializedFunctionDescriptor::specializedName() const
{
    return Object::sendMessage<NS::String*>(this, _MTL_PRIVATE_SEL(specializedName));
}
