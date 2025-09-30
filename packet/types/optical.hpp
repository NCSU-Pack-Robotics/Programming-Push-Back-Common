#pragma once

#include <cstdint>

/**
 * A packet containing data from the optical sensor.
 * TODO: This is a placeholder structure. Update when the optical data is known.
 */
struct OpticalData {
    /** Coordinate of the robot as reported by the optical sensor. */
    int32_t x, y, heading;
};
