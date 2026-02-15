#pragma once

#include <cstring>
#include <memory>
#include <utility>

#include "Header.hpp"
#include "Utils.hpp"

/**
 * Base class for packets to inherit from, that defines the data stored and available methods
 */
class Packet {
protected:
    /** The data bytes contained in the packet. */
    std::vector<uint8_t> data;

    /** The header of the packet, containing metadata such as packet ID. */
    Header header;

    /**
     * @param header The header of the packet.
     * @param data The data which is copied into the packets internal data buffer.
     */
    template <typename T>
    Packet(Header header, const T& data) : data(sizeof(T)), header(header)
    {
        memcpy(this->data.data(), &data, sizeof(T));
    }
public:
    /**
     * This constructor is more convenient to use when the data is already in byte form or when the type of the data is
     * unknown (such as when receiving data over serial).
     * Leaving this constructor explicit to avoid accidental misuse of the header.
     * @param header The received header of the packet
     * @param data The received data of the packet in bytes
     * @param length The length of the data in bytes
     */
    explicit Packet(Header header, const uint8_t* data, size_t length);

    /** @returns The header and the data combined into one vector of bytes */
    std::vector<uint8_t> serialize() const;

    uint8_t get_id() const;

    /**
     * @returns The data from the packet. The packet must have data or this will not compile.
     */
    template <typename T>
    requires std::derived_from<T, Packet>
    T::Data get_data() const {
        std::array<uint8_t, sizeof(typename T::Data)> bytes;
        memcpy(&bytes, this->data.data(), sizeof(typename T::Data));
        return std::bit_cast<typename T::Data>(bytes);
    }
};
