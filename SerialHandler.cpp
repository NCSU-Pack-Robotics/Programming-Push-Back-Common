#include "SerialHandler.hpp"

#include <array>
#include <cmath>
#include <cstring>
#include <iostream>
#include <fcntl.h>
#include <optional>
#include <unistd.h>
#include <vector>

#include "COBS.hpp"

SerialHandler::SerialHandler(const DeviceType device_type) : device_type(device_type) {
    if (device_type == DeviceType::PI) {

        // TODO: Should implement something to scan for the correct /dev/ttyACMx port
        // Open with read/write and O_NOCTTY is so we don't become the process's controlling terminal
        this->fd = open("/dev/ttyACM1", O_RDWR | O_NOCTTY);

        if (this->fd < 0) {
            perror("Error opening file: ");
        }

        // TODO: Better error handling here, maybe continuously try to connect until it works
    }
}

SerialHandler::~SerialHandler() {
    if (this->fd != -1) {
        close(fd);
    }
}

template <typename T>
void SerialHandler::send(const T& packet) {
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


void SerialHandler::receive() {
    if (fd == -1) return;

    uint8_t in = ' ';  // Current byte being read
    std::vector<uint8_t> bytes;

    // Read until we get a null byte (0x00)—end of packet
    while (in != '\0') {
        ssize_t num_read = 0;
        if (this->device_type == DeviceType::PI) {
            num_read = read(this->fd, &in, 1);
        } else if (this->device_type == DeviceType::BRAIN) {
            num_read = read(STDIN_FILENO, &in, 1);
        }

        if (num_read != 1) {
            /* Error occurred or EOF */
            return;
        }

        bytes.push_back(in);
    }

    // Cobs does not decode the last 0x00 byte, it is only used in encoding so we have a delimiter
    bytes.pop_back();

    const std::optional<std::vector<uint8_t>> decoded = cobs_decode(bytes);
    if (!decoded.has_value()) return; // If we fail to decode, ignore the packet

    uint32_t packet_id;
    uint32_t packet_hash;

    // Yoink the ID and hash from the start of the decoded packet
    memcpy(&packet_id, decoded->data(), sizeof(uint32_t));
    memcpy(&packet_hash, decoded->data() + sizeof(uint32_t), sizeof(uint32_t));

    // Ensure hash from packet matches the incoming data
    const uint32_t actual_hash = compute_hash(decoded->data() + 2 * sizeof(uint32_t),
                                                          decoded->size() - 2 * sizeof(uint32_t));
    if (actual_hash != packet_hash) return;  // If not match, something was corrupted—drop the packet

    const auto it = this->handlers.find(static_cast<PacketId>(packet_id));
    if (it == this->handlers.end()) return; // If the packet ID is not in the decoding map, ignore it

    // Grab the data from the packet TODO: how to handle the data
    it->second(decoded->data() + 2 * sizeof(uint32_t));
}


uint32_t SerialHandler::compute_hash(const uint8_t* start, int length) {
    uint32_t hash = 0;
    for (int i = 0; i < length; i++) {
        hash ^= static_cast<uint32_t>(*(start + i)) << 5;
    }

    return hash;
}
