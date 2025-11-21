#include "Packet.hpp"

Packet::Packet(Header header, const uint8_t* data, size_t length) : header(header), data(length) {
    memcpy(this->data.data(), data, length);
}
