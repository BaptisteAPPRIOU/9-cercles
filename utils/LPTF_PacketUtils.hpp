#pragma once
#include <string>
#include <vector>
#include <cstdint>
#include <stdexcept>

inline void appendStringToPayload(std::vector<uint8_t>& payload, const std::string& str) {
    uint32_t len = static_cast<uint32_t>(str.size());
    payload.insert(payload.end(), reinterpret_cast<uint8_t*>(&len), reinterpret_cast<uint8_t*>(&len) + sizeof(len));
    payload.insert(payload.end(), str.begin(), str.end());
}

inline std::string extractStringFromPayload(const std::vector<uint8_t>& payload, size_t& offset) {
    if (offset + 4 > payload.size()) throw std::runtime_error("Truncated string length");
    uint32_t len = *reinterpret_cast<const uint32_t*>(&payload[offset]);
    offset += 4;
    if (offset + len > payload.size()) throw std::runtime_error("Truncated string");
    std::string str(payload.begin() + offset, payload.begin() + offset + len);
    offset += len;
    return str;
}
