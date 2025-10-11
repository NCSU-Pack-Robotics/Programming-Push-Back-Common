#pragma once

/**
 * An enumeration of all packet IDs.
 * These IDs are used to identify the type of packet being sent or received.
 */
enum class PacketId : uint8_t {
    /** Packet containing data from the optical sensor. */
    OPTICAL,

    /** Packet containing data from the custom encoders. */
    ENCODER,
};
