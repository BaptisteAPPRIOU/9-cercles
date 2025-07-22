#ifndef LPTF_PACKET_HPP
#define LPTF_PACKET_HPP

#include <vector>
#include <string>
#include <cstdint>
#include <stdexcept>


class LPTF_Packet {
public:
// enum class better for type safety
    // Packet types for communication
    enum class PacketType : uint8_t {
        GET_INFO = 0x01,
        KEYLOG = 0x02,
        PROCESS_LIST = 0x03,
        EXEC_COMMAND = 0x04,
        RESPONSE = 0xFF,
        ERROR = 0xFE
        // probably more types in the future
    };

    // Canonical form (rule of three)
        LPTF_Packet(
        uint8_t version,
        PacketType type,
        uint8_t flags,
        uint16_t packetId,
        uint32_t sessionId,
        const std::vector<uint8_t>& payload
    );

    LPTF_Packet(const LPTF_Packet& other);
    LPTF_Packet& operator=(const LPTF_Packet& other);
    ~LPTF_Packet();

    std::vector<uint8_t> serialize() const;
    static LPTF_Packet deserialize(const std::vector<uint8_t>& data);

    // Getters
    uint8_t getVersion() const;
    PacketType getType() const;
    uint8_t getFlags() const;
    uint16_t getPacketId() const;
    uint32_t getSessionId() const;
    const std::vector<uint8_t>& getPayload() const;

private:
    uint8_t version;
    PacketType type;
    uint8_t flags;
    uint16_t packetId;
    uint32_t sessionId;
    std::vector<uint8_t> payload;
};

#endif // LPTF_PACKET_H
