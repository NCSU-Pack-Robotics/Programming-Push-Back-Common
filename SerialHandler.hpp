#pragma once

#include <any>

#include <cstdint>
#include <cstring>
#include <deque>
#include <functional>
#include <memory>
#include <typeindex>
#include <unistd.h>

#include "Buffer.hpp"
#include "packet/Packet.hpp"


/** An enumeration for the type of device this SerialHandler is running on. */
enum class DeviceType {
    /** The VEX Brain */
    BRAIN,

    /** A RaspberryPi (Gen. 4 at the time of writing). */
    PI
};

class SerialHandler {
public:
    /**
     * Constructs a SerialHandler for the given device type.
     * @param device_type The type of device this SerialHandler is running on.
     */
    explicit SerialHandler(DeviceType device_type);

    /** Cleans up the SerialHandler, closing any open file descriptors. */
    ~SerialHandler();

    /**
     * Sends the given packet over the serial connection.
     *
     * This method takes an entire packet as opposed to the data in order to focus the scope to transmission logic and
     * allow flexibility in packet structure later.
     *
     * @param packet The packet struct to send.
     * This is a reference to avoid unnecessary copying.
     * @tparam T The type of the packet struct to send.
     */
    template <typename T>
    void send(const Packet& packet) {
        // Create enough space to store the entire packet
        std::vector<uint8_t> data_to_send (2 * sizeof(packet.header) + packet.data.size());

        // Copy header and data into the byte array
        memcpy(data_to_send.data(), &packet.header, sizeof(packet.header));
        memcpy(data_to_send.data() + sizeof(packet.header), packet.data.data(), packet.data.size());

        // Frame the packet using COBS
        const std::optional<std::vector<uint8_t>> encoded = cobs_encode(data_to_send);
        if (!encoded.has_value()) return;

        // Write the data to the serial connection
        if (this->device_type == DeviceType::BRAIN) {
            write(STDOUT_FILENO, encoded->data(), encoded->size());
        } else if (this->device_type == DeviceType::PI) {
            write(this->fd, encoded->data(), encoded->size());
        }
    }

    /**
     * Blocking call that reads and handles a single packet
     * @return A packet of any data type. Use the ID from the header to decode which data the packet uses.
     */
    void receive();

    std::optional<Packet> pop_latest(PacketId packet_id);

    /** A map of packet ids to their Buffer */
    std::unordered_map<PacketId, Buffer> buffers;

private:
    /** The type of device this SerialHandler is running on. */
    DeviceType device_type;

    /** The file descriptor for the serial connection. */
    int fd = -1;
};
