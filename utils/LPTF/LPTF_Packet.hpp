#pragma once

#include "LPTF_PacketType.hpp"
#include <vector>
#include <cstdint>
#include <stdexcept>

/**
 * @class LPTF_Packet
 * @brief Represents a packet in the LPTF protocol, with serialization and deserialization support.
 */
class LPTF_Packet
{
public:
    /**
     * @brief Constructs a packet with the given parameters.
     * @param version Protocol version.
     * @param type Packet type (see PacketType).
     * @param flags Packet flags.
     * @param packetId Packet identifier.
     * @param sessionId Session identifier.
     * @param payload Packet payload data.
     */
    LPTF_Packet(
        uint8_t version,
        PacketType type,
        uint8_t flags,
        uint16_t packetId,
        uint32_t sessionId,
        const std::vector<uint8_t> &payload);

    /**
     * @brief Copy constructor.
     * @param other Packet to copy from.
     */
    LPTF_Packet(const LPTF_Packet &other);

    /**
     * @brief Move constructor.
     * @param other Packet to move from.
     */
    LPTF_Packet(LPTF_Packet &&other) noexcept;

    /**
     * @brief Copy assignment operator.
     * @param other Packet to copy from.
     * @return Reference to this packet.
     */
    LPTF_Packet &operator=(const LPTF_Packet &other);

    /**
     * @brief Move assignment operator.
     * @param other Packet to move from.
     * @return Reference to this packet.
     */
    LPTF_Packet &operator=(LPTF_Packet &&other) noexcept;

    /**
     * @brief Destructor.
     */
    virtual ~LPTF_Packet();

    /**
     * @brief Serializes the packet to a byte vector.
     * @return Serialized packet as a vector of bytes.
     */
    std::vector<uint8_t> serialize() const;

    /**
     * @brief Deserializes a packet from a byte vector.
     * @param data Byte vector containing the serialized packet.
     * @return Deserialized LPTF_Packet object.
     * @throws std::runtime_error if the data is invalid.
     */
    static LPTF_Packet deserialize(const std::vector<uint8_t> &data);

    /**
     * @brief Gets the protocol version.
     * @return Protocol version.
     */
    uint8_t getVersion() const;

    /**
     * @brief Gets the packet type.
     * @return Packet type.
     */
    PacketType getType() const;

    /**
     * @brief Gets the packet flags.
     * @return Packet flags.
     */
    uint8_t getFlags() const;

    /**
     * @brief Gets the packet identifier.
     * @return Packet identifier.
     */
    uint16_t getPacketId() const;

    /**
     * @brief Gets the session identifier.
     * @return Session identifier.
     */
    uint32_t getSessionId() const;

    /**
     * @brief Gets the packet payload.
     * @return Reference to the payload vector.
     */
    const std::vector<uint8_t> &getPayload() const;

private:
    uint8_t version;
    PacketType type;
    uint8_t flags;
    uint16_t packetId;
    uint32_t sessionId;
    std::vector<uint8_t> payload;
};
