#pragma once

#include "PacketId.hpp"

#include <cstdint>
#include <cstring>
#include <functional>
#include <typeindex>
#include <unistd.h>

#include "COBS.hpp"


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
     * Sends the given data over the serial connection.
     * @tparam T The type of the packet struct to send.
     * @param packet The packet struct to send.
     */
    template <typename T>
    void send(const T& packet) {
        const auto it = structs_to_packet_ids.find(std::type_index(typeid(T)));
        if (it == structs_to_packet_ids.end()) return;

        const auto packet_id = static_cast<uint32_t>(it->second);

        // Create enough space to store 2, 4 byte values in the header: ID and hash, and the packet struct body
        std::vector<uint8_t> data(2 * sizeof(uint32_t) + sizeof(packet));

        // Copy ID into vector
        memcpy(data.data(), &packet_id, sizeof(packet_id));

        // Copy Hash into vector
        const uint32_t hash = compute_hash(reinterpret_cast<const uint8_t*>(&packet), sizeof(packet));
        memcpy(data.data() + sizeof(uint32_t), &hash, sizeof(hash));

        // Write the struct body after the header bytes
        memcpy(data.data() + 2 * sizeof(uint32_t), &packet, sizeof(packet));

        // Frame the packet using COBS
        const std::optional<std::vector<uint8_t>> encoded = cobs_encode(data);
        if (!encoded.has_value()) return;

        // Write the data to the serial connection
        if (this->device_type == DeviceType::BRAIN) {
            write(STDOUT_FILENO, encoded->data(), encoded->size());
        } else if (this->device_type == DeviceType::PI) {
            write(this->fd, encoded->data(), encoded->size());
        }
    }

    /** Blocking call that reads and handles a single packet */
    void receive();

    /** A map of packet struct identifiers to packet IDs */
    std::unordered_map<std::type_index, PacketId> structs_to_packet_ids;

    /** A map of packet IDs to handler methods that decode and respond to the packet */
    std::unordered_map<PacketId, std::function<void(const uint8_t* data)>> handlers;

private:
    /** The type of device this SerialHandler is running on. */
    DeviceType device_type;

    /**
     * Computes a simple hash of the given struct data for integrity checking.
     * @param start A pointer to the start of the struct data.
     * @param length The length of the struct data in bytes.
     * @return A 32-bit hash of the struct data.
     */
    static uint32_t compute_hash(const uint8_t* start, int length);

    /** The file descriptor for the serial connection. */
    int fd = -1;
};
