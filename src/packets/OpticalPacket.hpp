#pragma once
#include <stdfloat>

#include "Packet.hpp"

/**
 * A packet containing data from the optical sensor.
 */
class OpticalPacket : public Packet {
public:
    static constexpr uint8_t id = PacketIds::OPTICAL;

    struct Data
    {
        std::float64_t x;
        std::float64_t y;
        std::float64_t heading;
    };

    OpticalPacket(std::float64_t x, std::float64_t y, std::float64_t heading) : Packet(Header{id}, Data{x, y, heading}) {};
};
