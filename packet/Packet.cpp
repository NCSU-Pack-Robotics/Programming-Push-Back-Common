#include "Packet.hpp"

Packet::Packet(Header header, const uint8_t* data, size_t length) {
    std::vector<uint8_t> packet_data(length);
    packet_data.assign(data, data + length);

    // Build the header
    this->header = header;
    this->data = std::move(packet_data);
}
