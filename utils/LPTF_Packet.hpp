#ifndef LPTF_PACKET_H
#define LPTF_PACKET_H



#include "../utils/PacketType.hpp"
#include <vector>
#include <string>
#include <cstdint>
#include <stdexcept>
using namespace std;

class LPTF_Packet {
public:
// enum cla (rule of three)
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

    vector<uint8_t> serialize() const;
    static LPTF_Packet deserialize(const vector<uint8_t>& data);

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
    vector<uint8_t> payload;
};

#endif // LPTF_PACKET_H
