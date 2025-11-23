#pragma once

/** The standard definition for the header send in all packets. */
struct Header {
    /** The type of packet being sent. */
    uint8_t packet_id;
};
