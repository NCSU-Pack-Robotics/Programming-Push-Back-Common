#pragma once
#include <cstdint>
#include <cstdio>
#include <cstring>
#include <functional>
#include <typeindex>
#include <vector>

#include "COBS.hpp"
#include "PacketId.hpp"

enum class DeviceType
{
    BRAIN,
    PI
};

class SerialHandler
{
public:
    explicit SerialHandler(DeviceType device_type);


    template <typename T>
    void send(const T& packet)
    {
        // std::optional<std::vector<uint8_t>> encoded = cobs_encode(reinterpret_cast<const uint8_t*>(&packet), sizeof(packet));

        // if (!encoded.has_value()) return;
        auto it = structs_to_packet_ids.find(std::type_index(typeid(T)));
        if (it == structs_to_packet_ids.end()) return;
        int32_t packet_id = static_cast<int32_t>(it->second);

        // Create enough space to store 2, 4 byte values in the header: ID and hash, and the packet struct body
        std::vector<uint8_t> data(2 * sizeof(int32_t) + sizeof(packet));

        memcpy(data.data(), &packet_id, sizeof(packet_id)); // Copy ID into vector
        // memcpy(data.data() + sizeof(int32_t), &hash, sizeof(hash)); // TODO: Compute hash

        // Write the struct body after the header bytes
        memcpy(data.data() + 2 * sizeof(int32_t), &packet, sizeof(packet));

        std::optional<std::vector<uint8_t>> encoded = cobs_encode(data);
        if (!encoded.has_value()) return;

        if (this->device_type == DeviceType::BRAIN)
        {
            fwrite(encoded->data(), encoded->size(), sizeof(uint8_t), stdout);
        }
        else if (this->device_type == DeviceType::PI)
        {

        }

    }

    /** A map of packet struct identifiers to packet IDs */
    std::unordered_map<std::type_index, PacketId> structs_to_packet_ids;
    /** A map of packet IDs to handler methods that decode and respond to the packet */
    std::unordered_map<PacketId, std::function<void(std::vector<uint8_t>)>> handlers;

private:
    DeviceType device_type;
};
