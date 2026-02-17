#pragma once

#include <array>

#include "Packet.hpp"


class TextPacket : public Packet {
public:
    static constexpr uint8_t id = PacketIds::TEXT;

    struct Data {
        std::array<char, SerialHandler::MAX_PACKET_SIZE> text;
    };

    TextPacket(const std::array<char, SerialHandler::MAX_PACKET_SIZE>& arr) : Packet(Header{id}, Data{arr}) {};
};