

#pragma once

#include "NSDefines.hpp"
#include "NSObject.hpp"
#include "NSPrivate.hpp"
#include "NSTypes.hpp"

namespace NS
{
class AutoreleasePool : public Object
{
public:
    static AutoreleasePool* alloc();
    AutoreleasePool*        init();

    void                    drain();

    void                    addObject(Object* pObject);

    static void             showPools();
};
}

_NS_INLINE NS::AutoreleasePool* NS::AutoreleasePool::alloc()
{
    return NS::Object::alloc<AutoreleasePool>(_NS_PRIVATE_CLS(NSAutoreleasePool));
}

_NS_INLINE NS::AutoreleasePool* NS::AutoreleasePool::init()
{
    return NS::Object::init<AutoreleasePool>();
}

_NS_INLINE void NS::AutoreleasePool::drain()
{
    Object::sendMessage<void>(this, _NS_PRIVATE_SEL(drain));
}

_NS_INLINE void NS::AutoreleasePool::addObject(Object* pObject)
{
    Object::sendMessage<void>(this, _NS_PRIVATE_SEL(addObject_), pObject);
}

_NS_INLINE void NS::AutoreleasePool::showPools()
{
    Object::sendMessage<void>(_NS_PRIVATE_CLS(NSAutoreleasePool), _NS_PRIVATE_SEL(showPools));
}

