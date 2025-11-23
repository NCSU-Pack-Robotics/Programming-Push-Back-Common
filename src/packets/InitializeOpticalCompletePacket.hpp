#pragma once
#include "Packet.hpp"
#include "PacketTypes.hpp"

/** A packet sent from the pi to the brain to signal that initialization and calibration of the optical sensor has been finished. */
class InitializeOpticalCompletePacket : public Packet
{
public:
    static constexpr uint8_t id = PacketIds::INITIALIZE_OPTICAL_COMPLETE;

    struct Data
    {

    };

    explicit InitializeOpticalCompletePacket(const Data& data = {}) : Packet(Header{id}, data) {};
};
