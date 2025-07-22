#ifndef LPTF_PACKET_HPP
#define LPTF_PACKET_HPP

#include <vector>
#include <string>
#include <cstdint>
#include <stdexcept>

class LPTF_Packet {
public:
    enum PacketType : uint8_t {
        GET_INFO = 0x01,
        KEYLOG = 0x02,
        PROCESS_LIST = 0x03,
        EXEC_COMMAND = 0x04,
        RESPONSE = 0xFF
    };

    LPTF_Packet(uint8_t version, PacketType type, const std::vector<uint8_t>& payload);

    std::vector<uint8_t> serialize() const;
    static LPTF_Packet deserialize(const std::vector<uint8_t>& data);

    uint8_t getVersion() const;
    PacketType getType() const;
    const std::vector<uint8_t>& getPayload() const;

private:
    uint8_t version;
    PacketType type;
    std::vector<uint8_t> payload;
};

#endif // LPTF_PACKET_HPP
