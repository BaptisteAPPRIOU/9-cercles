#include "LPTF_Packet.h"

LPTF_Packet::LPTF_Packet(uint8_t version, PacketType type, const vector<uint8_t>& payload)
    : version(version), type(type), payload(payload) {}

vector<uint8_t> LPTF_Packet::serialize() const {
    vector<uint8_t> data;
    data.push_back(version);
    data.push_back(type);

    uint16_t size = payload.size();
    data.push_back((size >> 8) & 0xFF);  // High byte
    data.push_back(size & 0xFF);         // Low byte

    data.insert(data.end(), payload.begin(), payload.end());
    return data;
}

LPTF_Packet LPTF_Packet::deserialize(const vector<uint8_t>& data) {
    if (data.size() < 4) throw runtime_error("Packet too short");

    uint8_t version = data[0];
    PacketType type = static_cast<PacketType>(data[1]);

    uint16_t size = (data[2] << 8) | data[3];
    if (data.size() < 4 + size) throw runtime_error("Payload size mismatch");

    vector<uint8_t> payload(data.begin() + 4, data.begin() + 4 + size);
    return LPTF_Packet(version, type, payload);
}

uint8_t LPTF_Packet::getVersion() const { return version; }
LPTF_Packet::PacketType LPTF_Packet::getType() const { return type; }
const vector<uint8_t>& LPTF_Packet::getPayload() const { return payload; }
