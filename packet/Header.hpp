#pragma once

#include <cstdint>

#include "../PacketId.hpp"

/** The standard definition for the header send in all packets. */
struct Header {
    /** The type of packet being sent. */
    PacketId packet_id;

   // TODO: Remove once brain code also stops sending checksum
    uint16_t checksum;
};
