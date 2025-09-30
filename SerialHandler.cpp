#include "SerialHandler.hpp"

#include <cmath>
#include <cstring>
#include <iostream>
#include <fcntl.h>
#include <unistd.h>

#include "packet/types/encoder.hpp"
#include "packet/types/optical.hpp"
#include "PacketId.hpp"

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

std::unique_ptr<Packet<std::any>> SerialHandler::receive() {
    if (fd == -1) return nullptr;

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
                return nullptr;
            }

            bytes.push_back(in);
        }

        // Cobs does not decode the last 0x00 byte, it is only used in encoding so we have a delimiter
        bytes.pop_back();

        const std::optional<std::vector<uint8_t>> decoded = cobs_decode(bytes);
        if (!decoded.has_value()) return nullptr; // If we fail to decode, ignore the packet

        // Decode the header
        Header received_header{};
        memcpy(&received_header, decoded->data(), sizeof(received_header));

        std::unique_ptr<Packet<std::any>> received_packet;

        // Using copilot for this. Not quite sure how it works. Alternate: use a macro
        auto make_packet = [&]<typename T0>(const T0& data_type) {
            using T = std::decay_t<T0>;
            T data{};
            memcpy(&data, decoded->data() + sizeof(received_header), sizeof(data));
            return std::make_unique<Packet<std::any>>(received_header, data);
        };

        switch (received_header.packet_id) {
        case PacketId::ENCODER:
            received_packet = make_packet(EncoderData{});
            break;
        case PacketId::OPTICAL:
            received_packet = make_packet(OpticalData{});
            break;
        default: // Unknown packet type
            return nullptr;
        }

        // Ensure the packet is valid by checking the checksum
        if (!received_packet->check_checksum()) {
            // If the checksum is invalid, ignore the packet
            return nullptr;
        }

        return received_packet;
}