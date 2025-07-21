#ifndef LPTF_PACKET_H
#define LPTF_PACKET_H

#include <vector>
#include <string>
#include <cstdint>
#include <stdexcept>
using namespace std;

class LPTF_Packet {
public:
    enum PacketType : uint8_t {
        GET_INFO = 0x01,
        KEYLOG = 0x02,
        PROCESS_LIST = 0x03,
        EXEC_COMMAND = 0x04,
        RESPONSE = 0xFF
    };

    LPTF_Packet(uint8_t version, PacketType type, const vector<uint8_t>& payload);

    vector<uint8_t> serialize() const;
    static LPTF_Packet deserialize(const vector<uint8_t>& data);

    uint8_t getVersion() const;
    PacketType getType() const;
    const vector<uint8_t>& getPayload() const;

private:
    uint8_t version;
    PacketType type;
    vector<uint8_t> payload;
};

#endif // LPTF_PACKET_H
