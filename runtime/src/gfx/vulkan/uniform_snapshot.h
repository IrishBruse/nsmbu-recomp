#pragma once
#include <array>
#include <cstddef>
#include <cstdint>
#include <cstring>
#include <utility>
#include <vector>

namespace gfxvk {

template<class Slice, class Device>
class UniformSnapshotCache {
public:
  static constexpr size_t slotCount = 34;
  struct Counters {
    uint64_t lookups = 0, checks = 0, comparisons = 0, hits = 0;

    uint64_t reusedBytes = 0;
  } counters;

  template<class Factory>
  Slice get(Device device, uint64_t generation, size_t slot,
            const void* bytes, size_t size, bool directReads, Factory&& factory) {
    if (!initialized_ || device_ != device || generation_ != generation) {
      for (auto& entry : entries_) entry.clear();
      device_ = device;
      generation_ = generation;
      initialized_ = true;
    }
    ++counters.lookups;
    if (slot >= slotCount)
      return std::forward<Factory>(factory)(bytes, size);
    auto& last = entries_[slot];
    if (last.valid && last.size == size) {
      ++counters.checks;
      bool equal;
      if (!size) equal = true;
      else if (!bytes) equal = last.knownZero;
      else {
        ++counters.comparisons;

        equal = last.knownZero ? all_zero(bytes, size)
              : std::memcmp(bytes, last.direct ? last.slice.mapped : last.bytes.data(), size) == 0;
      }
      if (equal) {
        ++counters.hits;
        counters.reusedBytes += last.slice.size;
        return last.slice;
      }
    }
    const void* source = nullptr;
    if (directReads) {
      last.bytes.clear();
      source = bytes;
    } else if (bytes && size) {
      const auto* b = static_cast<const uint8_t*>(bytes);
      last.bytes.assign(b, b + size);
      source = last.bytes.data();
    }
    auto slice = std::forward<Factory>(factory)(source, size);

    if (slice.buffer && slice.mapped && slice.size == (size < 16 ? 16 : size)) {
      last.slice = slice;
      last.size = size;
      last.valid = true;
      last.knownZero = !bytes || !size;
      last.direct = directReads;
    } else {
      last.clear();
    }
    return slice;
  }

  void reset() {
    for (auto& entry : entries_) entry.clear();
    initialized_ = false;
  }

private:
  static bool all_zero(const void* bytes, size_t size) {
    const auto* b = static_cast<const uint8_t*>(bytes);
    for (size_t i = 0; i < size; ++i)
      if (b[i]) return false;
    return true;
  }
  struct Entry {
    Slice slice{};
    size_t size = 0;
    bool valid = false, knownZero = false;
    bool direct = false;
    std::vector<uint8_t> bytes;
    void clear() { slice = {}; size = 0; valid = knownZero = direct = false; bytes.clear(); }
  };
  std::array<Entry, slotCount> entries_{};
  Device device_{};
  uint64_t generation_ = 0;
  bool initialized_ = false;
};
}
