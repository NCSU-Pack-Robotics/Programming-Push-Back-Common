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
     * @param data A pointer to the data to include in the packet. The data is copied into the packet.
     */
    template <typename T>
    Packet(const PacketId packet_id, const T* data) {
        // Build the header
        const Header h = {packet_id};

        // Use other constructor to construct packet
        *this = Packet(h, data);
    }

    /**
     * Leaving this constructor explicit to avoid accidental misuse of the header.
     * @param header The received header of the packet
     * @param data The received data of the packet The data is copied into the packet.
     */
    template<typename T>
    explicit Packet(const Header header, const T* data) : header(header) {
        // Cast data pointer to a byte pointer
        const auto data_bytes = reinterpret_cast<const uint8_t*>(data);

        // Use other constructor to construct packet
        *this = Packet(header, data_bytes, sizeof(T));
    }

    /**
     * This constructor is more convenient to use when the data is already in byte form or when the type of the data is
     * unknown (such as when receiving data over serial).
     * Leaving this constructor explicit to avoid accidental misuse of the header.
     * @param header The received header of the packet
     * @param data The received data of the packet in bytes
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
        return std::bit_cast<T>(this->data.data());
    }
};
