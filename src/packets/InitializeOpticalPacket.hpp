#pragma once
#include "Packet.hpp"

/** A packet sent from the brain to the pi to set up the optical sensor. This is an all-in-one packet that will
* zero the readings, and run the calibration. */
class InitializeOpticalPacket : public Packet
{
public:
    static constexpr uint8_t id = PacketIds::INITIALIZE_OPTICAL;

    struct Data
    {

    };

    explicit InitializeOpticalPacket(const Data& data = {}) : Packet(Header{id}, data) {};
};
