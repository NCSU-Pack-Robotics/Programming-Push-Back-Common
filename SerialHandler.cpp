#include "SerialHandler.hpp"

#include <array>
#include <cmath>
#include <iostream>
#include <fcntl.h>
#include <vector>

SerialHandler::SerialHandler(DeviceType device_type) : device_type(device_type)
{
    if (device_type == DeviceType::PI)
    {
        // Open with read/write and O_NOCTTY is so we don't become the process's controlling terminal
        this->fd = open("/dev/ttyACM1", O_RDWR | O_NOCTTY);
        if (this->fd < 0) {
            perror("Error opening file: ");
        }
        // TODO: Better error handling here, maybe continuously try to connect until it works
    }
    else if (device_type == DeviceType::BRAIN)
    {
        // Turn off buffering
        setvbuf(stdout, NULL, _IONBF, 0);
        setvbuf(stdin, NULL, _IONBF, 0);
    }
}

SerialHandler::~SerialHandler()
{
    if (this->fd != -1)
    {
        close(fd);
    }
}

void SerialHandler::receive()
{
    if (fd == -1) return;
    std::vector<uint8_t> bytes;
    uint8_t in = ' ';
    while (in != '\0')
    {
        int num_read;
        if (this->device_type == DeviceType::PI)
        {
            num_read = read(this->fd, &in, 1);
        }
        else if (this->device_type == DeviceType::BRAIN)
        {
            num_read = fread(&in, 1, 1, stdin);
        }
        if (num_read != 1) // Error occurred or EOF
        {
            return;
        }
        bytes.push_back(in);
    }

    bytes.pop_back(); // Cobs does not decode the last 0x00 byte, it is only used in encoding so we have a delimiter

    std::optional<std::vector<uint8_t>> decoded = cobs_decode(bytes);
    if (!decoded.has_value()) return; // If we fail to decode, ignore the packet

    uint32_t packet_id;
    uint32_t packet_hash;

    memcpy(&packet_id, decoded->data(), sizeof(uint32_t)); // Read the first 4 bytes into packet_id
    memcpy(&packet_hash, decoded->data() + sizeof(uint32_t), sizeof(uint32_t)); // Read the next 4 bytes into packet_hash

    uint32_t actual_hash = SerialHandler::get_struct_hash(decoded->data() + 2 * sizeof(uint32_t), decoded->size() - 2 * sizeof(uint32_t));
    if (actual_hash != packet_hash) return; // If the packet somehow got messed up, ignore it

    auto it = this->handlers.find(static_cast<PacketId>(packet_id));
    if (it == this->handlers.end()) return; // If the packet ID is not in the decoding map, ignore it

    it->second(decoded->data() + 2 * sizeof(uint32_t));
}


uint32_t SerialHandler::get_struct_hash(const uint8_t* start, int length)
{
    uint32_t hash = 0;
    for (int i = 0; i < length; i++)
    {
        hash ^= static_cast<uint32_t>(*(start + i)) << 5;
    }

    return hash;
}
