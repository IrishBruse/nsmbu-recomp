

#pragma once

#include "NSObject.hpp"
#include "NSEnumerator.hpp"

namespace NS
{
    class Set : public NS::Copying <Set>
    {
        public:
            UInteger count() const;
            Enumerator<Object>* objectEnumerator() const;

            static Set* alloc();

            Set* init();
            Set* init(const Object* const* pObjects, UInteger count);
            Set* init(const class Coder* pCoder);

    };
}

_NS_INLINE NS::UInteger NS::Set::count() const
{
    return NS::Object::sendMessage<NS::UInteger>(this, _NS_PRIVATE_SEL(count));
}

_NS_INLINE NS::Enumerator<NS::Object>* NS::Set::objectEnumerator() const
{
    return NS::Object::sendMessage<Enumerator<NS::Object>*>(this, _NS_PRIVATE_SEL(objectEnumerator));
}

_NS_INLINE NS::Set* NS::Set::alloc()
{
    return NS::Object::alloc<Set>(_NS_PRIVATE_CLS(NSSet));
}

_NS_INLINE NS::Set* NS::Set::init()
{
    return NS::Object::init<Set>();
}

_NS_INLINE NS::Set* NS::Set::init(const Object* const* pObjects, NS::UInteger count)
{
    return NS::Object::sendMessage<Set*>(this, _NS_PRIVATE_SEL(initWithObjects_count_), pObjects, count);
}

_NS_INLINE NS::Set* NS::Set::init(const class Coder* pCoder)
{
    return Object::sendMessage<Set*>(this, _NS_PRIVATE_SEL(initWithCoder_), pCoder);
}
