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
    /** The header of the packet, containing metadata such as packet ID and checksum. */
    Header header{};

    /** The data contained in the packet. This is template because any type of data can be contained. */
    std::vector<uint8_t> data;

    /**
     * Constructs a Packet completely.
     * @param packet_id The ID of the packet type.
     * @param data The data to be sent in the packet.
     * @param length The length of the data in bytes.
     */
    explicit Packet(PacketId packet_id, const uint8_t* data, size_t length);

    /**
     * Constructs a packet from received header and data
     * @param header The received header of the packet
     * @param data The received data of the packet
     */
    explicit Packet(const Header header, std::vector<uint8_t> data) : header(header), data(std::move(data)) {}

    /**
     * Returns the data from the packet as the specified type. The specified type is likely a struct from
     * <code>common/packet/types</code>.
     * @tparam T The type of the data to return. This must match with the PacketID from the header.
     * @return A pointer to the memory stored in the packet reinterpreted to be of type T.
     */
    template<typename T>
    T get_data() const;

    /**
     * Checks if the checksum in the header matches the computed checksum of the packet.
     * A useful method to call upon receipt of a packet to ensure data integrity.
     * @return True if the checksums match, false otherwise.
     */
    [[nodiscard]] bool check_checksum() const;

private:
    /**
     * Computes a two's complement checksum of the given data for integrity checking.
     *
     * https://datatracker.ietf.org/doc/html/rfc1071
     * @return A 16 bit two's complement checksum of the entire packet
     */
    [[nodiscard]] uint16_t compute_checksum() const;
};

template <typename T>
T Packet::get_data() const {
    return *reinterpret_cast<T>(this->data.data());
}
