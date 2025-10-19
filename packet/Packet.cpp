#include "Packet.hpp"

Packet::Packet(const Header header, const uint8_t* data, const size_t length) : header(header) {
    // Build the data vector
    std::vector<uint8_t> packet_data(length);
    packet_data.assign(data, data + length);
    this->data = packet_data;
}
