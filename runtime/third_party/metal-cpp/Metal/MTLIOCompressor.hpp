

#pragma once

#include "MTLDefines.hpp"
#include "MTLHeaderBridge.hpp"
#include "MTLPrivate.hpp"
#include "MTLDevice.hpp"

#include "../Foundation/Foundation.hpp"

namespace MTL
{
using IOCompressionContext=void*;

_MTL_ENUM(NS::Integer, IOCompressionStatus) {
    IOCompressionStatusComplete = 0,
    IOCompressionStatusError = 1,
};

size_t IOCompressionContextDefaultChunkSize();

IOCompressionContext IOCreateCompressionContext(const char* path, IOCompressionMethod type, size_t chunkSize);

void IOCompressionContextAppendData(IOCompressionContext context, const void* data, size_t size);

IOCompressionStatus IOFlushAndDestroyCompressionContext(IOCompressionContext context);

}

#if defined(MTL_PRIVATE_IMPLEMENTATION)

namespace MTL::Private {

MTL_DEF_FUNC(MTLIOCompressionContextDefaultChunkSize, size_t (*)(void));

MTL_DEF_FUNC( MTLIOCreateCompressionContext, void* (*)(const char*, MTL::IOCompressionMethod, size_t) );

MTL_DEF_FUNC( MTLIOCompressionContextAppendData, void (*)(void*, const void*, size_t) );

MTL_DEF_FUNC( MTLIOFlushAndDestroyCompressionContext, MTL::IOCompressionStatus (*)(void*) );

}

_NS_EXPORT size_t MTL::IOCompressionContextDefaultChunkSize()
{
    return MTL::Private::MTLIOCompressionContextDefaultChunkSize();
}

_NS_EXPORT void* MTL::IOCreateCompressionContext(const char* path, IOCompressionMethod type, size_t chunkSize)
{
    if ( MTL::Private::MTLIOCreateCompressionContext )
    {
        return MTL::Private::MTLIOCreateCompressionContext( path, type, chunkSize );
    }
    return nullptr;
}

_NS_EXPORT void MTL::IOCompressionContextAppendData(void* context, const void* data, size_t size)
{
    if ( MTL::Private::MTLIOCompressionContextAppendData )
    {
        MTL::Private::MTLIOCompressionContextAppendData( context, data, size );
    }
}

_NS_EXPORT MTL::IOCompressionStatus MTL::IOFlushAndDestroyCompressionContext(void* context)
{
    if ( MTL::Private::MTLIOFlushAndDestroyCompressionContext )
    {
        return MTL::Private::MTLIOFlushAndDestroyCompressionContext( context );
    }
    return MTL::IOCompressionStatusError;
}

#endif
