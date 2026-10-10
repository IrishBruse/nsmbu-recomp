set(NSMBU_LUAU_ROOT "${CMAKE_SOURCE_DIR}/runtime/third_party/luau")

foreach(NSMBU_LUAU_PART Common Ast Bytecode Compiler VM)
  if(NOT EXISTS "${NSMBU_LUAU_ROOT}/${NSMBU_LUAU_PART}/include")
    message(FATAL_ERROR "Luau sources missing at ${NSMBU_LUAU_ROOT}")
  endif()
endforeach()

file(GLOB NSMBU_LUAU_SOURCES CONFIGURE_DEPENDS
  "${NSMBU_LUAU_ROOT}/Common/src/*.cpp"
  "${NSMBU_LUAU_ROOT}/Ast/src/*.cpp"
  "${NSMBU_LUAU_ROOT}/Bytecode/src/*.cpp"
  "${NSMBU_LUAU_ROOT}/Compiler/src/*.cpp"
  "${NSMBU_LUAU_ROOT}/VM/src/*.cpp")

add_library(nsmbu_luau STATIC ${NSMBU_LUAU_SOURCES})
target_include_directories(nsmbu_luau SYSTEM PUBLIC
  "${NSMBU_LUAU_ROOT}/Common/include"
  "${NSMBU_LUAU_ROOT}/Ast/include"
  "${NSMBU_LUAU_ROOT}/Bytecode/include"
  "${NSMBU_LUAU_ROOT}/Compiler/include"
  "${NSMBU_LUAU_ROOT}/VM/include")
target_compile_features(nsmbu_luau PUBLIC cxx_std_17)
target_compile_options(nsmbu_luau PRIVATE -w -O2)
if(NOT MSVC)
  target_compile_options(nsmbu_luau PRIVATE -fno-math-errno)
endif()

if(UNIX)
  target_link_libraries(nsmbu_luau PUBLIC m ${CMAKE_DL_LIBS})
endif()

message(STATUS "Luau: vendored static (${NSMBU_LUAU_ROOT}, VM + Compiler)")
