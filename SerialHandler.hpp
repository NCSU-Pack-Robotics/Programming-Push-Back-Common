#pragma once

#include "PacketId.hpp"

#include <cstdint>
#include <functional>
#include <typeindex>


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
    void send(const T& packet);

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
