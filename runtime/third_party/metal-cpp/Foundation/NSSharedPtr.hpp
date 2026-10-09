//-------------------------------------------------------------------------------------------------------------------------------------------------------------
//
// Foundation/NSSharedPtr.hpp
//
// Copyright 2020-2024 Apple Inc.
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//     http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.
//
//-------------------------------------------------------------------------------------------------------------------------------------------------------------
#pragma once
#include <cstddef>
#include "NSDefines.hpp"
#pragma once

#include <cstddef>
#include "NSDefines.hpp"

namespace NS
{
template <class _Class>
class SharedPtr
{
public:

    SharedPtr();

    ~SharedPtr();

    SharedPtr(std::nullptr_t) noexcept;

    SharedPtr(const SharedPtr<_Class>& other) noexcept;

    template <class _OtherClass>
    SharedPtr(const SharedPtr<_OtherClass>& other, typename std::enable_if_t<std::is_convertible_v<_OtherClass *, _Class *>> * = nullptr) noexcept;

    SharedPtr(SharedPtr<_Class>&& other) noexcept;

    template <class _OtherClass>
    SharedPtr(SharedPtr<_OtherClass>&& other, typename std::enable_if_t<std::is_convertible_v<_OtherClass *, _Class *>> * = nullptr) noexcept;

    SharedPtr& operator=(const SharedPtr<_Class>& other);

    template <class _OtherClass>
    typename std::enable_if_t<std::is_convertible_v<_OtherClass *, _Class *>, SharedPtr &>
    operator=(const SharedPtr<_OtherClass>& other);

    SharedPtr& operator=(SharedPtr<_Class>&& other);

    template <class _OtherClass>
    typename std::enable_if_t<std::is_convertible_v<_OtherClass *, _Class *>, SharedPtr &>
    operator=(SharedPtr<_OtherClass>&& other);

    _Class* get() const;

    _Class* operator->() const;

    explicit operator bool() const;

    void reset();

    void detach();

    template <class _OtherClass>
    friend SharedPtr<_OtherClass> RetainPtr(_OtherClass* ptr);

    template <class _OtherClass>
    friend SharedPtr<_OtherClass> TransferPtr(_OtherClass* ptr);

private:
    _Class* m_pObject;
};

template <class _Class>
_NS_INLINE NS::SharedPtr<_Class> RetainPtr(_Class* pObject)
{
    NS::SharedPtr<_Class> ret;
    ret.m_pObject = pObject->retain();
    return ret;
}

template <class _Class>
_NS_INLINE NS::SharedPtr<_Class> TransferPtr(_Class* pObject)
{
    NS::SharedPtr<_Class> ret;
    ret.m_pObject = pObject;
    return ret;
}

}

template <class _Class>
_NS_INLINE NS::SharedPtr<_Class>::SharedPtr()
    : m_pObject(nullptr)
{
}

template <class _Class>
_NS_INLINE NS::SharedPtr<_Class>::~SharedPtr<_Class>() __attribute__((no_sanitize("undefined")))
{
    m_pObject->release();
}

template <class _Class>
_NS_INLINE NS::SharedPtr<_Class>::SharedPtr(std::nullptr_t) noexcept
    : m_pObject(nullptr)
{
}

template <class _Class>
_NS_INLINE NS::SharedPtr<_Class>::SharedPtr(const SharedPtr<_Class>& other) noexcept
    : m_pObject(other.m_pObject->retain())
{
}

template <class _Class>
template <class _OtherClass>
_NS_INLINE NS::SharedPtr<_Class>::SharedPtr(const SharedPtr<_OtherClass>& other, typename std::enable_if_t<std::is_convertible_v<_OtherClass *, _Class *>> *) noexcept
    : m_pObject(reinterpret_cast<_Class*>(other.get()->retain()))
{
}

template <class _Class>
_NS_INLINE NS::SharedPtr<_Class>::SharedPtr(SharedPtr<_Class>&& other) noexcept
    : m_pObject(other.m_pObject)
{
    other.m_pObject = nullptr;
}

template <class _Class>
template <class _OtherClass>
_NS_INLINE NS::SharedPtr<_Class>::SharedPtr(SharedPtr<_OtherClass>&& other, typename std::enable_if_t<std::is_convertible_v<_OtherClass *, _Class *>> *) noexcept
    : m_pObject(reinterpret_cast<_Class*>(other.get()))
{
    other.detach();
}

template <class _Class>
_NS_INLINE _Class* NS::SharedPtr<_Class>::get() const
{
    return m_pObject;
}

template <class _Class>
_NS_INLINE _Class* NS::SharedPtr<_Class>::operator->() const
{
    return m_pObject;
}

template <class _Class>
_NS_INLINE NS::SharedPtr<_Class>::operator bool() const
{
    return nullptr != m_pObject;
}

template <class _Class>
_NS_INLINE void NS::SharedPtr<_Class>::reset() __attribute__((no_sanitize("undefined")))
{
    m_pObject->release();
    m_pObject = nullptr;
}

template <class _Class>
_NS_INLINE void NS::SharedPtr<_Class>::detach()
{
    m_pObject = nullptr;
}

template <class _Class>
_NS_INLINE NS::SharedPtr<_Class>& NS::SharedPtr<_Class>::operator=(const SharedPtr<_Class>& other) __attribute__((no_sanitize("undefined")))
{
    _Class* pOldObject = m_pObject;

    m_pObject = other.m_pObject->retain();

    pOldObject->release();

    return *this;
}

template <class _Class>
template <class _OtherClass>
typename std::enable_if_t<std::is_convertible_v<_OtherClass *, _Class *>, NS::SharedPtr<_Class> &>
_NS_INLINE NS::SharedPtr<_Class>::operator=(const SharedPtr<_OtherClass>& other) __attribute__((no_sanitize("undefined")))
{
    _Class* pOldObject = m_pObject;

    m_pObject = reinterpret_cast<_Class*>(other.get()->retain());

    pOldObject->release();

    return *this;
}

template <class _Class>
_NS_INLINE NS::SharedPtr<_Class>& NS::SharedPtr<_Class>::operator=(SharedPtr<_Class>&& other) __attribute__((no_sanitize("undefined")))
{
    if (m_pObject != other.m_pObject)
    {
        m_pObject->release();
        m_pObject = other.m_pObject;
    }
    else
    {
        m_pObject = other.m_pObject;
        other.m_pObject->release();
    }
    other.m_pObject = nullptr;
    return *this;
}

template <class _Class>
template <class _OtherClass>
typename std::enable_if_t<std::is_convertible_v<_OtherClass *, _Class *>, NS::SharedPtr<_Class> &>
_NS_INLINE NS::SharedPtr<_Class>::operator=(SharedPtr<_OtherClass>&& other) __attribute__((no_sanitize("undefined")))
{
    if (m_pObject != other.get())
    {
        m_pObject->release();
        m_pObject = reinterpret_cast<_Class*>(other.get());
        other.detach();
    }
    else
    {
        m_pObject = other.get();
        other.reset();
    }
    return *this;
}

template <class _ClassLhs, class _ClassRhs>
_NS_INLINE bool operator==(const NS::SharedPtr<_ClassLhs>& lhs, const NS::SharedPtr<_ClassRhs>& rhs)
{
    return lhs.get() == rhs.get();
}

template <class _ClassLhs, class _ClassRhs>
_NS_INLINE bool operator!=(const NS::SharedPtr<_ClassLhs>& lhs, const NS::SharedPtr<_ClassRhs>& rhs)
{
    return lhs.get() != rhs.get();
}

namespace std
{

template <class T>
struct hash<NS::SharedPtr<T>>
{
    size_t operator()(const NS::SharedPtr<T>& p) const
    {
        return std::hash<T*>{}(p.get());
    }
};

}
