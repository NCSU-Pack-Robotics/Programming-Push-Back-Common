#include "Packet.hpp"

Packet::Packet(const PacketId packet_id, const uint8_t* data, const size_t length) {
    std::vector<uint8_t> packet_data(length);
    packet_data.assign(data, data + length);

    // Build the header
    this->header.packet_id = packet_id;
    this->data = std::move(packet_data);
}
