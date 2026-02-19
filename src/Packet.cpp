#include "Packet.hpp"

#include <cassert>

#include "SerialHandler.hpp"

Packet::Packet(Header header, const uint8_t* data, size_t length) : data(length), header(header) {
    memcpy(this->data.data(), data, length);
    assert(length <= SerialHandler::MAX_PACKET_DATA_SIZE);
}

std::vector<uint8_t> Packet::serialize() const {
    // Create enough space to store the entire packet
    std::vector<uint8_t> data_to_send(sizeof(Header) + this->data.size());

    // Copy header and data into the byte array
    memcpy(data_to_send.data(), &this->header, sizeof(Header));
    memcpy(data_to_send.data() + sizeof(Header), this->data.data(), this->data.size());

    return data_to_send;
}

uint8_t Packet::get_id() const {
    return this->header.packet_id;
}
