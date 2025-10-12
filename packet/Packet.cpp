#include "Packet.hpp"

#include <cstring>

Packet::Packet(Header header, const uint8_t* data, size_t length) {
    this->data.resize(length);
    memcpy(this->data.data(), data, length);

    this->header = header;
}
