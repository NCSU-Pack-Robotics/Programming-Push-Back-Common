#pragma once

#include <cstdint>

#include "../PacketId.hpp"

/** The standard definition for the header send in all packets. */
struct Header {
    /** The type of packet being sent. */
    PacketId packet_id;

    /**
     * The one's complement checksum of the packet data.
     * Should be computed using <code>utils.compute_checksum</code> and verified on receipt.
     */
    uint16_t checksum;
};
