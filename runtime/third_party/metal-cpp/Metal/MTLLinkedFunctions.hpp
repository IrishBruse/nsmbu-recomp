

#pragma once

#include "../Foundation/Foundation.hpp"
#include "MTLDefines.hpp"
#include "MTLHeaderBridge.hpp"
#include "MTLPrivate.hpp"

namespace MTL
{

class LinkedFunctions : public NS::Copying<LinkedFunctions>
{
public:
    static LinkedFunctions* alloc();

    NS::Array*              binaryFunctions() const;
    NS::Array*              functions() const;

    NS::Dictionary*         groups() const;

    LinkedFunctions*        init();

    static LinkedFunctions* linkedFunctions();

    NS::Array*              privateFunctions() const;

    void                    setBinaryFunctions(const NS::Array* binaryFunctions);

    void                    setFunctions(const NS::Array* functions);

    void                    setGroups(const NS::Dictionary* groups);

    void                    setPrivateFunctions(const NS::Array* privateFunctions);
};

}
_MTL_INLINE MTL::LinkedFunctions* MTL::LinkedFunctions::alloc()
{
    return NS::Object::alloc<MTL::LinkedFunctions>(_MTL_PRIVATE_CLS(MTLLinkedFunctions));
}

_MTL_INLINE NS::Array* MTL::LinkedFunctions::binaryFunctions() const
{
    return Object::sendMessage<NS::Array*>(this, _MTL_PRIVATE_SEL(binaryFunctions));
}

_MTL_INLINE NS::Array* MTL::LinkedFunctions::functions() const
{
    return Object::sendMessage<NS::Array*>(this, _MTL_PRIVATE_SEL(functions));
}

_MTL_INLINE NS::Dictionary* MTL::LinkedFunctions::groups() const
{
    return Object::sendMessage<NS::Dictionary*>(this, _MTL_PRIVATE_SEL(groups));
}

_MTL_INLINE MTL::LinkedFunctions* MTL::LinkedFunctions::init()
{
    return NS::Object::init<MTL::LinkedFunctions>();
}

_MTL_INLINE MTL::LinkedFunctions* MTL::LinkedFunctions::linkedFunctions()
{
    return Object::sendMessage<MTL::LinkedFunctions*>(_MTL_PRIVATE_CLS(MTLLinkedFunctions), _MTL_PRIVATE_SEL(linkedFunctions));
}

_MTL_INLINE NS::Array* MTL::LinkedFunctions::privateFunctions() const
{
    return Object::sendMessage<NS::Array*>(this, _MTL_PRIVATE_SEL(privateFunctions));
}

_MTL_INLINE void MTL::LinkedFunctions::setBinaryFunctions(const NS::Array* binaryFunctions)
{
    Object::sendMessage<void>(this, _MTL_PRIVATE_SEL(setBinaryFunctions_), binaryFunctions);
}

_MTL_INLINE void MTL::LinkedFunctions::setFunctions(const NS::Array* functions)
{
    Object::sendMessage<void>(this, _MTL_PRIVATE_SEL(setFunctions_), functions);
}

_MTL_INLINE void MTL::LinkedFunctions::setGroups(const NS::Dictionary* groups)
{
    Object::sendMessage<void>(this, _MTL_PRIVATE_SEL(setGroups_), groups);
}

_MTL_INLINE void MTL::LinkedFunctions::setPrivateFunctions(const NS::Array* privateFunctions)
{
    Object::sendMessage<void>(this, _MTL_PRIVATE_SEL(setPrivateFunctions_), privateFunctions);
}
