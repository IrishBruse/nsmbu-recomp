

#pragma once

#include "../Foundation/Foundation.hpp"
#include "MTLDefines.hpp"
#include "MTLHeaderBridge.hpp"
#include "MTLPrivate.hpp"

namespace MTL4
{
class Compiler;
_MTL_ENUM(NS::Integer, CompilerTaskStatus) {
    CompilerTaskStatusNone = 0,
    CompilerTaskStatusScheduled = 1,
    CompilerTaskStatusCompiling = 2,
    CompilerTaskStatusFinished = 3,
};

class CompilerTask : public NS::Referencing<CompilerTask>
{
public:
    Compiler*          compiler() const;

    CompilerTaskStatus status() const;

    void               waitUntilCompleted();
};

}

_MTL_INLINE MTL4::Compiler* MTL4::CompilerTask::compiler() const
{
    return Object::sendMessage<MTL4::Compiler*>(this, _MTL_PRIVATE_SEL(compiler));
}

_MTL_INLINE MTL4::CompilerTaskStatus MTL4::CompilerTask::status() const
{
    return Object::sendMessage<MTL4::CompilerTaskStatus>(this, _MTL_PRIVATE_SEL(status));
}

_MTL_INLINE void MTL4::CompilerTask::waitUntilCompleted()
{
    Object::sendMessage<void>(this, _MTL_PRIVATE_SEL(waitUntilCompleted));
}
