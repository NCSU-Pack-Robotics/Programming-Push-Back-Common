#pragma once

#include <utility>

#include "Header.hpp"
#include "../utils.hpp"

/**
 * A generic, lightweight packet structure that contains a header and data of type T.
 * @tparam T The type of data contained in the packet. Typically, this will be a type defined in packet/types.
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
     */
    explicit Packet(const PacketId packet_id, std::vector<uint8_t> data) : data(std::move(data)) {
        // Build the header
        this->header.packet_id = packet_id;
        this->header.checksum = compute_checksum();
    }

    /**
     * Constructs a packet from received header and data
     * @param header The received header of the packet
     * @param data The received data of the packet
     */
    explicit Packet(const Header header, std::vector<uint8_t> data) : header(header), data(std::move(data)) {}

    /**
     * Checks if the checksum in the header matches the computed checksum of the packet.
     * A useful method to call upon receipt of a packet to ensure data integrity.
     * @return True if the checksums match, false otherwise.
     */
    bool check_checksum() {
        const uint16_t computed_checksum = compute_checksum();
        return computed_checksum == this->header.checksum;
    }

private:
    /**
     * Computes a two's complement checksum of the given data for integrity checking.
     *
     * https://datatracker.ietf.org/doc/html/rfc1071
     * @return A 16 bit two's complement checksum of the entire packet
     */
    uint16_t compute_checksum() {
        uint16_t checksum = 0;

        // Compute checksum over header
        Header header = this->header;  // This is a copy to avoid modifying the original
        header.checksum = 0; // Zero out checksum field for calculation
        const auto header_bytes = reinterpret_cast<const uint8_t*>(&this->header);
        const uint16_t headerChecksum = compute_twos_sum(header_bytes, sizeof(Header));

        // Compute checksum over data
        const auto data_bytes = reinterpret_cast<const uint8_t*>(&this->data);
        const uint16_t dataChecksum = compute_twos_sum(data_bytes, data.size());

        // Combine the two checksums
        checksum = headerChecksum + dataChecksum;

        // Return the 2's complement of the sum
        return checksum;
    }
};
