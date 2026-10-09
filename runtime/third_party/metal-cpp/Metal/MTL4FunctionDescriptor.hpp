

#pragma once

#include "../Foundation/Foundation.hpp"
#include "MTLDefines.hpp"
#include "MTLHeaderBridge.hpp"
#include "MTLPrivate.hpp"

namespace MTL4
{
class FunctionDescriptor;

class FunctionDescriptor : public NS::Copying<FunctionDescriptor>
{
public:
    static FunctionDescriptor* alloc();

    FunctionDescriptor*        init();
};

}
_MTL_INLINE MTL4::FunctionDescriptor* MTL4::FunctionDescriptor::alloc()
{
    return NS::Object::alloc<MTL4::FunctionDescriptor>(_MTL_PRIVATE_CLS(MTL4FunctionDescriptor));
}

_MTL_INLINE MTL4::FunctionDescriptor* MTL4::FunctionDescriptor::init()
{
    return NS::Object::init<MTL4::FunctionDescriptor>();
}
