#pragma once
#include <stdfloat>

#include "Packet.hpp"

/**
 * A packet containing data from the optical sensor.
 */
class OpticalPacket : public Packet
{
public:
    static constexpr uint8_t id = PacketIds::OPTICAL;

    struct Data
    {
        std::float64_t x;
        std::float64_t y;
        std::float64_t heading;
    };

    explicit OpticalPacket(const Data& data) : Packet(Header{id}, data) {};
};
