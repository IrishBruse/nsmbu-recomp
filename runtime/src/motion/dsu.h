

#pragma once
#include <atomic>
#include <cstddef>
#include <cstdint>
#include <functional>
#include <mutex>
#include <string>
#include <thread>
#include <vector>

namespace dsu {

constexpr uint16_t kProtocolVersion = 1001;
constexpr uint16_t kDefaultPort = 26760;
enum MessageType : uint32_t { kVersion = 0x100000, kPortInfo = 0x100001, kPadData = 0x100002 };
constexpr size_t kHeaderSize = 16;
constexpr size_t kPortInfoSize = 32;
constexpr size_t kPadDataSize = 100;

uint32_t crc32(const uint8_t* data, size_t size);

std::vector<uint8_t> encode_port_info_request(uint32_t client_id, const std::vector<uint8_t>& slots);

std::vector<uint8_t> encode_pad_data_request(uint32_t client_id, uint8_t slot);

struct PadData {
    uint8_t slot = 0;
    uint8_t state = 0;
    uint8_t model = 0;
    bool connected = false;
    uint32_t packet = 0;
    uint64_t timestamp_us = 0;
    float accel[3] = {};
    float gyro[3] = {};
};

bool parse_pad_data(const uint8_t* data, size_t size, PadData& out);

uint32_t message_type(const uint8_t* data, size_t size);

std::vector<uint8_t> encode_pad_data(uint32_t server_id, const PadData& d);

class Client {
public:
    using Sink = std::function<void(const PadData&)>;
    explicit Client(Sink sink) : sink_(std::move(sink)) {}
    ~Client() { stop(); }

    void start(const std::string& host, uint16_t port, uint8_t slot);
    void stop();
    bool running() const { return thread_.joinable(); }

    bool receiving() const;
    std::string status() const;

private:
    void run(std::string host, uint16_t port, uint8_t slot);
    Sink sink_;
    std::thread thread_;
    std::atomic<bool> quit_{false};
    std::atomic<int64_t> last_data_ms_{0};
    mutable std::mutex mu_;
    std::string status_;
};

}
