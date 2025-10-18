#pragma once

#include <memory>
#include <utility>

#include "Header.hpp"
#include "../utils.hpp"

/**
 * A generic, lightweight packet structure that contains a header and data as an array of bytes.
 */
class Packet {
public:
    /** The header of the packet, containing metadata such as packet ID. */
    Header header;

    /** The data bytes contained in the packet. */
    std::vector<uint8_t> data;

    /**
     * @param packet_id The ID of the packet type.
     * @param data The data to be sent in the packet.
     * @param length The length of the data in bytes.
     */
    Packet(PacketId packet_id, const uint8_t* data, size_t length);

    /**
     * @param header The received header of the packet
     * @param data The received data of the packet
     * @param length The length of the data in bytes
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
