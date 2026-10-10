set(NSMBU_LUAJIT_ROOT "${CMAKE_SOURCE_DIR}/runtime/third_party/luajit")
set(NSMBU_LUAJIT_SRC "${NSMBU_LUAJIT_ROOT}/src")
set(NSMBU_LUAJIT_LIB "${NSMBU_LUAJIT_SRC}/libluajit.a")

if(NOT EXISTS "${NSMBU_LUAJIT_SRC}/lua.h")
  message(FATAL_ERROR "LuaJIT sources missing at ${NSMBU_LUAJIT_SRC}")
endif()

if(WIN32 AND MSVC)
  message(FATAL_ERROR "LuaJIT static build uses the POSIX Makefile; MSVC is not wired yet")
endif()

find_program(NSMBU_LUAJIT_MAKE NAMES gmake make REQUIRED)

set(NSMBU_LUAJIT_MAKE_ARGS
  BUILDMODE=static
  "XCFLAGS=-DLUAJIT_DISABLE_FFI"
  "CC=${CMAKE_C_COMPILER}"
  libluajit.a)

if(APPLE)
  if(CMAKE_OSX_DEPLOYMENT_TARGET)
    list(APPEND NSMBU_LUAJIT_MAKE_ARGS "MACOSX_DEPLOYMENT_TARGET=${CMAKE_OSX_DEPLOYMENT_TARGET}")
  endif()
endif()

include(ExternalProject)
ExternalProject_Add(nsmbu_luajit_ep
  SOURCE_DIR "${NSMBU_LUAJIT_ROOT}"
  CONFIGURE_COMMAND ""
  BUILD_IN_SOURCE TRUE
  BUILD_COMMAND "${NSMBU_LUAJIT_MAKE}" -C "${NSMBU_LUAJIT_SRC}" ${NSMBU_LUAJIT_MAKE_ARGS}
  INSTALL_COMMAND ""
  BUILD_BYPRODUCTS "${NSMBU_LUAJIT_LIB}"
  EXCLUDE_FROM_ALL FALSE)

add_library(nsmbu_luajit STATIC IMPORTED GLOBAL)
add_dependencies(nsmbu_luajit nsmbu_luajit_ep)
set_target_properties(nsmbu_luajit PROPERTIES
  IMPORTED_LOCATION "${NSMBU_LUAJIT_LIB}"
  INTERFACE_INCLUDE_DIRECTORIES "${NSMBU_LUAJIT_SRC}")

if(UNIX)
  target_link_libraries(nsmbu_luajit INTERFACE m ${CMAKE_DL_LIBS})
endif()

message(STATUS "LuaJIT: vendored static (${NSMBU_LUAJIT_ROOT}, BUILDMODE=static, LUAJIT_DISABLE_FFI)")
