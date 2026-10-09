

#pragma once

#include "NSObject.hpp"
#include "NSTypes.hpp"

namespace NS
{
struct FastEnumerationState
{
    unsigned long  state;
    Object**       itemsPtr;
    unsigned long* mutationsPtr;
    unsigned long  extra[5];
} _NS_PACKED;

class FastEnumeration : public Referencing<FastEnumeration>
{
public:
    NS::UInteger countByEnumerating(FastEnumerationState* pState, Object** pBuffer, NS::UInteger len);
};

template <class _ObjectType>
class Enumerator : public Referencing<Enumerator<_ObjectType>, FastEnumeration>
{
public:
    _ObjectType* nextObject();
    class Array* allObjects();
};
}

_NS_INLINE NS::UInteger NS::FastEnumeration::countByEnumerating(FastEnumerationState* pState, Object** pBuffer, NS::UInteger len)
{
    return Object::sendMessage<UInteger>(this, _NS_PRIVATE_SEL(countByEnumeratingWithState_objects_count_), pState, pBuffer, len);
}

template <class _ObjectType>
_NS_INLINE _ObjectType* NS::Enumerator<_ObjectType>::nextObject()
{
    return Object::sendMessage<_ObjectType*>(this, _NS_PRIVATE_SEL(nextObject));
}

template <class _ObjectType>
_NS_INLINE NS::Array* NS::Enumerator<_ObjectType>::allObjects()
{
    return Object::sendMessage<Array*>(this, _NS_PRIVATE_SEL(allObjects));
}

