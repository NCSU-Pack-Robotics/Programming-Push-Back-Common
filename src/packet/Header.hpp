#pragma once

#include "PacketId.hpp"

/** The standard definition for the header send in all packets. */
struct Header {
    /** The type of packet being sent. */
    PacketId packet_id;
};
