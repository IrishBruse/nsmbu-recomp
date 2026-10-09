

#pragma once

#include "NSEnumerator.hpp"
#include "NSObject.hpp"
#include "NSTypes.hpp"

namespace NS
{
class Dictionary : public NS::Copying<Dictionary>
{
public:
    static Dictionary* dictionary();
    static Dictionary* dictionary(const Object* pObject, const Object* pKey);
    static Dictionary* dictionary(const Object* const* pObjects, const Object* const* pKeys, UInteger count);

    static Dictionary* alloc();

    Dictionary*        init();
    Dictionary*        init(const Object* const* pObjects, const Object* const* pKeys, UInteger count);
    Dictionary*        init(const class Coder* pCoder);

    template <class _KeyType = Object>
    Enumerator<_KeyType>* keyEnumerator() const;

    template <class _Object = Object>
    _Object* object(const Object* pKey) const;
    UInteger count() const;
};
}

_NS_INLINE NS::Dictionary* NS::Dictionary::dictionary()
{
    return Object::sendMessage<Dictionary*>(_NS_PRIVATE_CLS(NSDictionary), _NS_PRIVATE_SEL(dictionary));
}

_NS_INLINE NS::Dictionary* NS::Dictionary::dictionary(const Object* pObject, const Object* pKey)
{
    return Object::sendMessage<Dictionary*>(_NS_PRIVATE_CLS(NSDictionary), _NS_PRIVATE_SEL(dictionaryWithObject_forKey_), pObject, pKey);
}

_NS_INLINE NS::Dictionary* NS::Dictionary::dictionary(const Object* const* pObjects, const Object* const* pKeys, UInteger count)
{
    return Object::sendMessage<Dictionary*>(_NS_PRIVATE_CLS(NSDictionary), _NS_PRIVATE_SEL(dictionaryWithObjects_forKeys_count_),
        pObjects, pKeys, count);
}

_NS_INLINE NS::Dictionary* NS::Dictionary::alloc()
{
    return NS::Object::alloc<Dictionary>(_NS_PRIVATE_CLS(NSDictionary));
}

_NS_INLINE NS::Dictionary* NS::Dictionary::init()
{
    return NS::Object::init<Dictionary>();
}

_NS_INLINE NS::Dictionary* NS::Dictionary::init(const Object* const* pObjects, const Object* const* pKeys, UInteger count)
{
    return Object::sendMessage<Dictionary*>(this, _NS_PRIVATE_SEL(initWithObjects_forKeys_count_), pObjects, pKeys, count);
}

_NS_INLINE NS::Dictionary* NS::Dictionary::init(const class Coder* pCoder)
{
    return Object::sendMessage<Dictionary*>(this, _NS_PRIVATE_SEL(initWithCoder_), pCoder);
}

template <class _KeyType>
_NS_INLINE NS::Enumerator<_KeyType>* NS::Dictionary::keyEnumerator() const
{
    return Object::sendMessage<Enumerator<_KeyType>*>(this, _NS_PRIVATE_SEL(keyEnumerator));
}

template <class _Object>
_NS_INLINE _Object* NS::Dictionary::object(const Object* pKey) const
{
    return Object::sendMessage<_Object*>(this, _NS_PRIVATE_SEL(objectForKey_), pKey);
}

_NS_INLINE NS::UInteger NS::Dictionary::count() const
{
    return Object::sendMessage<UInteger>(this, _NS_PRIVATE_SEL(count));
}

