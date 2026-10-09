

#pragma once
#include <cstdint>
#include <filesystem>
#include <fstream>
#include <functional>
#include <string>
#include <vector>

namespace zarchive {

struct Error {
    bool damaged;
    std::string msg;
};

class Reader {
public:
    static constexpr uint64_t BLOCK = 64 * 1024;

    static bool detect(const std::filesystem::path& p);

    explicit Reader(const std::filesystem::path& p);

    struct Node {
        std::string name;
        bool is_file;
        uint64_t offset, size;
        uint32_t first, count;
    };
    const Node& node(uint32_t i) const { return nodes_.at(i); }
    uint32_t root() const { return 0; }
    std::vector<uint32_t> children(uint32_t dir) const;

    void read_file(uint32_t file, const std::function<void(const uint8_t*, uint64_t)>& out);

    uint64_t archive_size() const { return size_; }

    bool verify(const std::function<void(uint64_t, uint64_t)>& progress);

private:
    std::ifstream f_;
    uint64_t size_ = 0, data_off_ = 0, data_size_ = 0;
    std::vector<uint64_t> block_off_;
    std::vector<uint32_t> block_len_;
    std::vector<Node> nodes_;
    uint8_t hash_[32] = {};
    uint64_t cached_ = ~0ull;
    std::vector<uint8_t> cache_, packed_;

    void raw_read(uint64_t off, uint8_t* dst, uint64_t len);
    const uint8_t* block(uint64_t index);
};

}
