

#pragma once

#include "NSDefines.hpp"
#include "NSObject.hpp"
#include "NSPrivate.hpp"
#include "NSTypes.hpp"

namespace NS
{
class URL : public Copying<URL>
{
public:
    static URL* fileURLWithPath(const class String* pPath);

    static URL* alloc();
    URL*        init();
    URL*        init(const class String* pString);
    URL*        initFileURLWithPath(const class String* pPath);

    const char* fileSystemRepresentation() const;
};
}

_NS_INLINE NS::URL* NS::URL::fileURLWithPath(const String* pPath)
{
    return Object::sendMessage<URL*>(_NS_PRIVATE_CLS(NSURL), _NS_PRIVATE_SEL(fileURLWithPath_), pPath);
}

_NS_INLINE NS::URL* NS::URL::alloc()
{
    return Object::alloc<URL>(_NS_PRIVATE_CLS(NSURL));
}

_NS_INLINE NS::URL* NS::URL::init()
{
    return Object::init<URL>();
}

_NS_INLINE NS::URL* NS::URL::init(const String* pString)
{
    return Object::sendMessage<URL*>(this, _NS_PRIVATE_SEL(initWithString_), pString);
}

_NS_INLINE NS::URL* NS::URL::initFileURLWithPath(const String* pPath)
{
    return Object::sendMessage<URL*>(this, _NS_PRIVATE_SEL(initFileURLWithPath_), pPath);
}

_NS_INLINE const char* NS::URL::fileSystemRepresentation() const
{
    return Object::sendMessage<const char*>(this, _NS_PRIVATE_SEL(fileSystemRepresentation));
}

