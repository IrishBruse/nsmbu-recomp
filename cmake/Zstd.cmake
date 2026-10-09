# zstd (BSD-3-Clause, https:
# needs it to read Cemu's Wii U archives (.wua). Defines the target nsmbu_zstd.
#
# Developer builds use a system zstd when there is one (its CMake package, else pkg-config libzstd),
# so they need no network. Release builds compile the pinned source below (URL + SHA-256, like the
# other dependencies in cmake/WindowsDependencies.cmake) into a static library with the project's
# own compiler, so the shipped nsmbu-extract is one self-contained program: NSMBU_BUNDLED_ZSTD, on by
# default with NSMBU_BUNDLED_DEPS (the Linux and Windows release builds) and set explicitly by the
# macOS release build (its runner has a Homebrew libzstd.dylib that must not end up in the release).
# Without a system zstd the pinned source is downloaded as well; offline:
# -DFETCHCONTENT_SOURCE_DIR_ZSTD=DIR with the unpacked zstd-1.5.7 release.
option(NSMBU_BUNDLED_ZSTD "build zstd from its pinned source (static) instead of using a system zstd" ${NSMBU_BUNDLED_DEPS})

if(NOT NSMBU_BUNDLED_ZSTD)
  find_package(zstd CONFIG QUIET)
  foreach(t zstd::libzstd zstd::libzstd_shared zstd::libzstd_static)
    if(TARGET ${t})
      set(NSMBU_ZSTD_TARGET ${t})
      break()
    endif()
  endforeach()
  if(NOT NSMBU_ZSTD_TARGET)
    find_package(PkgConfig QUIET)
    if(PKG_CONFIG_FOUND)
      pkg_check_modules(NSMBU_SYSTEM_ZSTD QUIET IMPORTED_TARGET libzstd)
      if(NSMBU_SYSTEM_ZSTD_FOUND)
        set(NSMBU_ZSTD_TARGET PkgConfig::NSMBU_SYSTEM_ZSTD)
      endif()
    endif()
  endif()
endif()

add_library(nsmbu_zstd INTERFACE)
if(NSMBU_ZSTD_TARGET)
  message(STATUS "zstd: system (${NSMBU_ZSTD_TARGET})")
  target_link_libraries(nsmbu_zstd INTERFACE ${NSMBU_ZSTD_TARGET})
  # tools/release/package.py refuses this build: the extractor would depend on the system's zstd
  file(WRITE ${CMAKE_BINARY_DIR}/nsmbu-zstd.txt "system\n")
else()
  message(STATUS "zstd: pinned source 1.5.7, static")
  include(FetchContent)
  if(POLICY CMP0135)
    cmake_policy(SET CMP0135 NEW)
  endif()
  FetchContent_Declare(zstd
    URL https://github.com/facebook/zstd/releases/download/v1.5.7/zstd-1.5.7.tar.gz
    URL_HASH SHA256=eb33e51f49a15e023950cd7825ca74a4a2b43db8354825ac24fc1b7ee09e6fa3
    SOURCE_SUBDIR lib)  # no CMakeLists.txt there: only downloaded and unpacked, the library is defined below
  FetchContent_MakeAvailable(zstd)
  # lib/common, lib/decompress and lib/compress (no CLI, no multithreading, no assembly); compression is
  # used by the extractor's tests only (a synthetic archive), the linker leaves it out of nsmbu-extract
  file(GLOB NSMBU_ZSTD_SOURCES ${zstd_SOURCE_DIR}/lib/common

