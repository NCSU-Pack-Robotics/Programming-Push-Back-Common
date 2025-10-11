#include "Packet.hpp"

Packet::Packet(const PacketId packet_id, const uint8_t* data, const size_t length) {
    std::vector<uint8_t> packet_data(length);
    packet_data.assign(data, data + length);
    this->data = packet_data;

    // Build the header
    this->header.packet_id = packet_id;
    this->header.checksum = compute_checksum();
}

bool Packet::check_checksum() const {
    const uint16_t computed_checksum = compute_checksum();
    return computed_checksum == this->header.checksum;
}

uint16_t Packet::compute_checksum() const {
    uint16_t checksum = 0;

    // Compute checksum over header
    Header header = this->header;  // This is a copy to avoid modifying the original
    header.checksum = 0; // Zero out checksum field for calculation
    const auto header_bytes = reinterpret_cast<const uint8_t*>(&header);
    const uint16_t headerChecksum = compute_twos_sum(header_bytes, sizeof(Header));

    // Compute checksum over data
    const auto data_bytes = reinterpret_cast<const uint8_t*>(&this->data);
    const uint16_t dataChecksum = compute_twos_sum(data_bytes, data.size());

    // Combine the two checksums
    checksum = headerChecksum + dataChecksum;

    // Return the 2's complement of the sum
    return checksum;
}
