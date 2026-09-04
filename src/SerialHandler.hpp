#pragma once

#include <functional>

#include "Buffer.hpp"
#include "Header.hpp"
#include "Packet.hpp"
#include "comms/AbstractComm.hpp"

class SerialHandler {
    std::unique_ptr<AbstractComm> comm;

public:
    SerialHandler(std::unique_ptr<AbstractComm> comm) : comm(std::move(comm)) {}
    virtual ~SerialHandler() = default;

    /** The maximum packet size in bytes that is supported by the Vex Brain. This is a hardware limitation. It is
     * important to read at least this amount in bulk transfers to avoid errors. */
    static constexpr int MAX_LIBUSB_PACKET_SIZE = 512;

    /** The maximum packet size that we can use for our packets. */
    static constexpr size_t MAX_PACKET_SIZE = 1024;
    /** The maximum packet that can be sent is MAX_PACKET_SIZE, but the total is higher
    * to account for the increased size from cobs encoding
    * - +2 bytes from the start and end byte
    * - +5 bytes from ceil(1024 / 254). See Utils::cobs_encode for more details
    * TODO: In the future it may be better to make the buffer dynamically sized to avoid this confusion
    */
    static constexpr size_t MAX_ENCODED_PACKET_SIZE = 1024 + 2 + 5;
    /** The max size in bytes that the data of a packet can be so that once its encoded it doesn't go over MAX_PACKET_SIZE */
    static constexpr size_t MAX_PACKET_DATA_SIZE = MAX_PACKET_SIZE - sizeof(Header);

    /**
     * Sends the given packet over the serial connection.
     *
     * @param packet A reference to the packet to transmit.
     */
    void send(const Packet& packet);

    /**
     * Blocking call that reads a single packet.
     * If the packet has listeners registered to it, they will execute before this function returns.
     * Note: It is possible that a packet fails to decode after being read, this function will return
     * regardless of the success of decoding.
     */
    template <std::derived_from<Packet> T>
    std::optional<typename T::Data> get_packet_data() {

        auto packet = get_packet();
        if (packet.has_value()) {
            return packet->get_data<T>();
        }
        return std::nullopt;
    }

    std::optional<Packet> get_packet();

private:
    std::optional<Packet> decode_packet(const unsigned char* packet_end);

    /** An array of bytes that stores the data from receiving packets. Used temporarily between calls to libusb_block_transfer when receiving
     * This buffer needs to be large enough to store (MAX_ENCODED_PACKET_SIZE - 1) bytes + the amount of bytes being read in each IO call.
     * If a full MAX_ENCODED_PACKET_SIZE is read, it will be decoded and removed from the buffer before the next receive call.
     * In the worst case, there will be 1 fewer bytes sent, meaning we need to store them until we read IO again, and on each IO call there needs to
     * be room for however many bytes we read. */
    unsigned char buffer[MAX_ENCODED_PACKET_SIZE - 1 + MAX_LIBUSB_PACKET_SIZE]{};
    /** The index in the buffer array where the next read data should be placed. */
    size_t next_write_index = 0;
};
