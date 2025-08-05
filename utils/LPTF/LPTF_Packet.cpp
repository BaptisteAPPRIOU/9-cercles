#include "LPTF_Packet.hpp"
#include "LPTF_PacketType.hpp"
#include <utility> // pour std::move

LPTF_Packet::LPTF_Packet(
    uint8_t version,
    PacketType type,
    uint8_t flags,
    uint16_t packetId,
    uint32_t sessionId,
    const std::vector<uint8_t> &payload)
    : version(version), type(type), flags(flags),
      packetId(packetId), sessionId(sessionId), payload(payload) {}

LPTF_Packet::LPTF_Packet(const LPTF_Packet &other)
    : version(other.version), type(other.type), flags(other.flags),
      packetId(other.packetId), sessionId(other.sessionId), payload(other.payload) {}

LPTF_Packet::LPTF_Packet(LPTF_Packet &&other) noexcept
    : version(other.version), type(other.type), flags(other.flags),
      packetId(other.packetId), sessionId(other.sessionId), payload(std::move(other.payload)) {}

LPTF_Packet &LPTF_Packet::operator=(const LPTF_Packet &other)
{
    if (this != &other)
    {
        version = other.version;
        type = other.type;
        flags = other.flags;
        packetId = other.packetId;
        sessionId = other.sessionId;
        payload = other.payload;
    }
    return *this;
}

LPTF_Packet &LPTF_Packet::operator=(LPTF_Packet &&other) noexcept
{
    if (this != &other)
    {
        version = other.version;
        type = other.type;
        flags = other.flags;
        packetId = other.packetId;
        sessionId = other.sessionId;
        payload = std::move(other.payload);
    }
    return *this;
}

LPTF_Packet::~LPTF_Packet() = default;

std::vector<uint8_t> LPTF_Packet::serialize() const
{
    std::vector<uint8_t> data;
    data.push_back(version);
    data.push_back(static_cast<uint8_t>(type));
    data.push_back(flags);
    data.push_back((packetId >> 8) & 0xFF);
    data.push_back(packetId & 0xFF);
    data.push_back((sessionId >> 24) & 0xFF);
    data.push_back((sessionId >> 16) & 0xFF);
    data.push_back((sessionId >> 8) & 0xFF);
    data.push_back(sessionId & 0xFF);
    uint16_t size = static_cast<uint16_t>(payload.size());
    data.push_back((size >> 8) & 0xFF);
    data.push_back(size & 0xFF);

    data.insert(data.end(), payload.begin(), payload.end());
    return data;
}

LPTF_Packet LPTF_Packet::deserialize(const std::vector<uint8_t> &data)
{
    if (data.size() < 11)
        throw std::runtime_error("Packet too short");
    uint8_t version = data[0];
    PacketType type = static_cast<PacketType>(data[1]);
    uint8_t flags = data[2];
    uint16_t packetId = (static_cast<uint16_t>(data[3]) << 8) | data[4];
    uint32_t sessionId = (static_cast<uint32_t>(data[5]) << 24) | (static_cast<uint32_t>(data[6]) << 16) | (static_cast<uint32_t>(data[7]) << 8) | data[8];
    uint16_t size = (static_cast<uint16_t>(data[9]) << 8) | data[10];
    if (data.size() < 11 + size)
        throw std::runtime_error("Payload size mismatch");
    std::vector<uint8_t> payload(data.begin() + 11, data.begin() + 11 + size);
    return LPTF_Packet(version, type, flags, packetId, sessionId, payload);
}

uint8_t LPTF_Packet::getVersion() const { return version; }
PacketType LPTF_Packet::getType() const { return type; }
uint8_t LPTF_Packet::getFlags() const { return flags; }
uint16_t LPTF_Packet::getPacketId() const { return packetId; }
uint32_t LPTF_Packet::getSessionId() const { return sessionId; }
const std::vector<uint8_t> &LPTF_Packet::getPayload() const { return payload; }
