#ifndef PACKETTYPE_HPP
#define PACKETTYPE_HPP

#include <cstdint>

/**
 * @enum PacketType
 * @brief Types of packets supported by the LPTF protocol.
 *
 * - GET_INFO: Request or send system information.
 * - KEYLOG: Start or stop keylogger.
 * - PROCESS_LIST: Request or send process list.
 * - EXEC_COMMAND: Execute a command on the remote system.
 * - RESPONSE: Generic response packet.
 * - PACKET_ERROR: Error packet.
 */
enum class PacketType : uint8_t
{
    GET_INFO = 0x01,     ///< Request or send system information
    KEYLOG = 0x02,       ///< Start or stop keylogger
    PROCESS_LIST = 0x03, ///< Request or send process list
    EXEC_COMMAND = 0x04, ///< Execute a command on the remote system
    RESPONSE = 0xFF,     ///< Generic response packet
    PACKET_ERROR = 0xFE, ///< Error packet
};

#endif