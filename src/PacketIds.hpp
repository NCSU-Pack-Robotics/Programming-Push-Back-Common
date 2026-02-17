#pragma once
#include <cstdint>

namespace PacketIds {
    enum PacketIds : uint8_t {
        OPTICAL,
        INITIALIZE_OPTICAL,
        INITIALIZE_OPTICAL_COMPLETE,
        TEXT,
        LENGTH // Used to get the amount of packet ids at compile time
    };
}
