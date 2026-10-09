

#pragma once

#include "../Foundation/Foundation.hpp"
#include "MTL4FunctionDescriptor.hpp"
#include "MTLDefines.hpp"
#include "MTLHeaderBridge.hpp"
#include "MTLPrivate.hpp"

namespace MTL4
{
class LibraryFunctionDescriptor;
}

namespace MTL
{
class Library;
}

namespace MTL4
{
class LibraryFunctionDescriptor : public NS::Copying<LibraryFunctionDescriptor, FunctionDescriptor>
{
public:
    static LibraryFunctionDescriptor* alloc();

    LibraryFunctionDescriptor*        init();

    MTL::Library*                     library() const;

    NS::String*                       name() const;

    void                              setLibrary(const MTL::Library* library);

    void                              setName(const NS::String* name);
};

}
_MTL_INLINE MTL4::LibraryFunctionDescriptor* MTL4::LibraryFunctionDescriptor::alloc()
{
    return NS::Object::alloc<MTL4::LibraryFunctionDescriptor>(_MTL_PRIVATE_CLS(MTL4LibraryFunctionDescriptor));
}

_MTL_INLINE MTL4::LibraryFunctionDescriptor* MTL4::LibraryFunctionDescriptor::init()
{
    return NS::Object::init<MTL4::LibraryFunctionDescriptor>();
}

_MTL_INLINE MTL::Library* MTL4::LibraryFunctionDescriptor::library() const
{
    return Object::sendMessage<MTL::Library*>(this, _MTL_PRIVATE_SEL(library));
}

_MTL_INLINE NS::String* MTL4::LibraryFunctionDescriptor::name() const
{
    return Object::sendMessage<NS::String*>(this, _MTL_PRIVATE_SEL(name));
}

_MTL_INLINE void MTL4::LibraryFunctionDescriptor::setLibrary(const MTL::Library* library)
{
    Object::sendMessage<void>(this, _MTL_PRIVATE_SEL(setLibrary_), library);
}

_MTL_INLINE void MTL4::LibraryFunctionDescriptor::setName(const NS::String* name)
{
    Object::sendMessage<void>(this, _MTL_PRIVATE_SEL(setName_), name);
}
