#pragma once
#include <cstdint>
#include <cstring>
#include <utility>
#include <vector>

namespace gfxvk {

template<class Slice>
struct VertexSnapshotHistory {
  struct Entry {
    uint32_t address = 0, size = 0;
    Slice slice{};
    std::vector<uint8_t> bytes;
    bool direct = false;
    void clear() { address = size = 0; slice = {}; bytes.clear(); direct = false; }
  };
  Entry last{}, previous{};
  static bool matches(const Entry& entry, uint32_t address, uint32_t size) {
    return entry.slice.buffer && entry.address == address && entry.size == size;
  }

  static bool equal(const Entry& entry, const void* fresh) {
    return !entry.size ||
           std::memcmp(fresh, entry.direct ? entry.slice.mapped : entry.bytes.data(), entry.size) == 0;
  }
  Entry* secondary(uint32_t address, uint32_t size) {
    return matches(previous, address, size) ? &previous : nullptr;
  }
  void promote() { std::swap(last, previous); }
  void reset() { last.clear(); previous.clear(); }

  template<class Make>
  Slice remember(uint32_t address, uint32_t size, const void* fresh, bool keepHistory, bool direct,
                 Make&& make) {
    if (!keepHistory) previous.clear();

    else if (!matches(last, address, size)) std::swap(previous, last);
    last.address = address;
    last.size = size;
    last.direct = direct;
    if (direct) {
      last.bytes.clear();
      last.slice = std::forward<Make>(make)(fresh, size_t(size));
      return last.slice;
    }
    const auto* source = static_cast<const uint8_t*>(fresh);
    last.bytes.assign(source, source + size);
    last.slice = std::forward<Make>(make)(static_cast<const void*>(last.bytes.data()), size_t(size));
    return last.slice;
  }
};

template<class Slice>
struct VertexWindowEntry {
  uint32_t address = 0, reservation = 0, begin = 0, length = 0;
  Slice slice{};
  std::vector<uint8_t> bytes;
  bool direct = false;
  bool matches(uint32_t a, uint32_t r, uint32_t b, uint32_t l) const {
    return slice.buffer && address == a && reservation == r && begin == b && length == l;
  }

  bool equal(const void* fresh) const {
    if (!length) return true;
    const void* kept = direct ? static_cast<const void*>(static_cast<const uint8_t*>(slice.mapped) + begin)
                              : static_cast<const void*>(bytes.data());
    return !std::memcmp(fresh, kept, length);
  }
  void clear() { address = reservation = begin = length = 0; slice = {}; bytes.clear(); direct = false; }

  const uint8_t* remember(uint32_t a, uint32_t r, uint32_t b, uint32_t l, const void* fresh,
                          bool directReads) {
    address = a; reservation = r; begin = b; length = l;
    slice = {};
    direct = directReads;
    const auto* source = static_cast<const uint8_t*>(fresh);
    if (direct) {
      bytes.clear();
      return source;
    }
    bytes.assign(source, source + l);
    return bytes.data();
  }
};
}
