#include "SerialHandler.hpp"

#include <cmath>
#include <cstring>
#include <iostream>
#include <fcntl.h>
#include <unistd.h>

#include "Buffer.hpp"
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

        // Decode the header
        Header received_header{};
        memcpy(&received_header, decoded->data(), sizeof(received_header));

        Packet received_packet{received_header, decoded.value()};

        // Ensure the packet is valid by checking the checksum
        if (!received_packet.check_checksum()) {
            // If the checksum is invalid, ignore the packet
            return;
        }


        this->buffers[received_header.packet_id].add(received_packet);

}

std::optional<Packet> SerialHandler::pop_latest(PacketId packet_id)
{
    return this->buffers[packet_id].pop_latest();
}
