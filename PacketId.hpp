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

    /** Struct to test sending packets > 512 bytes */
    LARGE,

    /** A packet sent from the brain to the pi to set up the optical sensor. This is an all-in-one packet that will
     * zero the readings, and run the calibration. */
    INITIALIZE_OPTICAL,

    /** A packet sent from the pi to the brain to signal that initialization and calibration of the optical sensor has been finished. */
    INITIALIZE_OPTICAL_COMPLETE
};
