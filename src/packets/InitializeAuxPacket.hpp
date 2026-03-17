#pragma once
#include "Packet.hpp"

/** A packet sent from the brain to the pi, which should be sent before sending any other packets.
 * It will destroy all existing threads and initialize again. */
class InitializeAuxPacket : public Packet {
public:
    static constexpr uint8_t id = PacketIds::INITIALIZE_AUX;

    InitializeAuxPacket() : Packet(Header{id}, nullptr, 0) {};
};