

#pragma once

#define _NS_WEAK_IMPORT __attribute__((weak_import))
#ifdef METALCPP_SYMBOL_VISIBILITY_HIDDEN
#define _NS_EXPORT __attribute__((visibility("hidden")))
#else
#define _NS_EXPORT __attribute__((visibility("default")))
#endif
#define _NS_EXTERN extern "C" _NS_EXPORT
#define _NS_INLINE inline __attribute__((always_inline))
#define _NS_PACKED __attribute__((packed))

#define _NS_CONST(type, name) _NS_EXTERN type const name
#define _NS_ENUM(type, name) enum name : type
#define _NS_OPTIONS(type, name) \
    using name = type;          \
    enum : name

#define _NS_CAST_TO_UINT(value) static_cast<NS::UInteger>(value)
#define _NS_VALIDATE_SIZE(ns, name) static_assert(sizeof(ns::name) == sizeof(ns##name), "size mismatch " #ns "::" #name)
#define _NS_VALIDATE_ENUM(ns, name) static_assert(_NS_CAST_TO_UINT(ns::name) == _NS_CAST_TO_UINT(ns##name), "value mismatch " #ns "::" #name)

