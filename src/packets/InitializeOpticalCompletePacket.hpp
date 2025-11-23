#pragma once
#include "Packet.hpp"

/** A packet sent from the pi to the brain to signal that initialization and calibration of the optical sensor has been finished. */
class InitializeOpticalCompletePacket : public Packet
{
public:
    static constexpr int id = 0x02;

    struct Data
    {

    };

    explicit InitializeOpticalCompletePacket(const Data& data = {}) : Packet(Header{id}, data) {};
};
