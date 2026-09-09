#pragma once

#include <cstdint>

/** The standard definition for the header send in all packets. */
struct Header {
private:
    /** A bitset of flags, currently only the LSB is used, a full byte is reserved for future use.
     * Bit 0 - is_request: If set, this packet is a request for a response packet, and should not contain data.
     * Bit 1 ...
     */
    uint8_t flags;
public:
    /** The type of packet being sent/requested. */
    uint8_t packet_id;

    bool is_request() const
    {
        return flags & 0x1;
    }

    void set_is_request(bool is_request)
    {
        is_request ? flags |= 0x1 : flags &= ~0x1;
    }

    Header() = default;

    Header(uint8_t packet_id, bool is_request)
    {
        this->packet_id = packet_id;
        set_is_request(is_request);
    }
};
