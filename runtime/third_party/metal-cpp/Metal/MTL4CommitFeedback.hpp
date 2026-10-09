

#pragma once

#include "../Foundation/Foundation.hpp"
#include "MTLDefines.hpp"
#include "MTLHeaderBridge.hpp"
#include "MTLPrivate.hpp"
#include <CoreFoundation/CoreFoundation.h>

#include <functional>

namespace MTL4
{
class CommitFeedback;

using CommitFeedbackHandler = void (^)(MTL4::CommitFeedback*);
using CommitFeedbackHandlerFunction = std::function<void(MTL4::CommitFeedback*)>;

class CommitFeedback : public NS::Referencing<CommitFeedback>
{
public:
    CFTimeInterval GPUEndTime() const;

    CFTimeInterval GPUStartTime() const;

    NS::Error*     error() const;
};

}
_MTL_INLINE CFTimeInterval MTL4::CommitFeedback::GPUEndTime() const
{
    return Object::sendMessage<CFTimeInterval>(this, _MTL_PRIVATE_SEL(GPUEndTime));
}

_MTL_INLINE CFTimeInterval MTL4::CommitFeedback::GPUStartTime() const
{
    return Object::sendMessage<CFTimeInterval>(this, _MTL_PRIVATE_SEL(GPUStartTime));
}

_MTL_INLINE NS::Error* MTL4::CommitFeedback::error() const
{
    return Object::sendMessage<NS::Error*>(this, _MTL_PRIVATE_SEL(error));
}
