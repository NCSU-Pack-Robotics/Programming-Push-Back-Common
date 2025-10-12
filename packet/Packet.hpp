#pragma once

#include <memory>

#include "Header.hpp"
#include "../utils.hpp"

/**
 * A generic, lightweight packet structure that contains a header and data as an array of bytes.
 */
class Packet {
public:
    /** The header of the packet, containing metadata such as packet ID and checksum. */
    Header header;

    /** The data contained in the packet. This is a template because any type of data can be contained. */
    std::vector<uint8_t> data;

    /**
     * Constructs a Packet completely.
     * @param header The packet header information.
     * @param data A pointer to the beginning of the data in the packet.
     * @param length The length of the data in bytes.
     */
    explicit Packet(Header header, const uint8_t* data, size_t length);

    /**
     * Returns the data from the packet as the specified type. The specified type is likely a struct from
     * <code>common/packet/types</code>.
     * @tparam T The type of the data to return. This must match with the PacketID from the header.
     * @return The data.
     */
    template <typename T>
    T get_data() const {
        return *reinterpret_cast<const T*>(this->data.data());
    }
};
