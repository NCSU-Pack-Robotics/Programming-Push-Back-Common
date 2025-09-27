#pragma once

#include "COBS.hpp"
#include "PacketId.hpp"

#include <cstdint>
#include <cstdio>
#include <cstring>
#include <functional>
#include <typeindex>
#include <vector>
#include <unistd.h>



enum class DeviceType
{
    BRAIN,
    PI
};

class SerialHandler
{
public:
    explicit SerialHandler(DeviceType device_type);
    ~SerialHandler();

    template <typename T>
    void send(const T& packet)
    {
        auto it = structs_to_packet_ids.find(std::type_index(typeid(T)));
        if (it == structs_to_packet_ids.end()) return;
        int32_t packet_id = static_cast<uint32_t>(it->second);

        // Create enough space to store 2, 4 byte values in the header: ID and hash, and the packet struct body
        std::vector<uint8_t> data(2 * sizeof(uint32_t) + sizeof(packet));

        memcpy(data.data(), &packet_id, sizeof(packet_id)); // Copy ID into vector
        // Hash is computed for packet data before cobs encoding
        uint32_t hash = SerialHandler::get_struct_hash(reinterpret_cast<const uint8_t*>(&packet), sizeof(packet));
        memcpy(data.data() + sizeof(uint32_t), &hash, sizeof(hash));

        // Write the struct body after the header bytes
        memcpy(data.data() + 2 * sizeof(uint32_t), &packet, sizeof(packet));

        std::optional<std::vector<uint8_t>> encoded = cobs_encode(data);
        if (!encoded.has_value()) return;

        if (this->device_type == DeviceType::BRAIN)
        {
            write(STDOUT_FILENO, encoded->data(), encoded->size());
        }
        else if (this->device_type == DeviceType::PI)
        {
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
    DeviceType device_type;

    static uint32_t get_struct_hash(const uint8_t* start, int length);

    int fd = -1;
};
