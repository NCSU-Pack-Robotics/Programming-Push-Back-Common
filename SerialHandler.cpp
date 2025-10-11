#include "SerialHandler.hpp"

#include <array>
#include <cmath>
#include <cstring>
#include <iostream>
#include <fcntl.h>
#include <optional>
#include <unistd.h>
#include <vector>

SerialHandler::SerialHandler()
#if PI
: device_handle(nullptr)
#endif
{
#if PI
        // Initialize the libusb context. We pass nullptr as the context to use the global default one.
        if (int error = libusb_init_context(nullptr, nullptr, 0) != LIBUSB_SUCCESS)
        {
            printf("Failed to initialize libusb context: %s\n", libusb_error_name(error));
            return;
        }

        libusb_device** devices = nullptr;
        // Get the list of devices on the system
        ssize_t device_count = libusb_get_device_list(nullptr, &devices);
        if (device_count < 0)
        {
            printf("Failed to get device list: %s\n", libusb_error_name(static_cast<int>(device_count)));
            return;
        }

        // TODO: We should probably retry initializing the context and getting the device list if they ever fail.

        // Loop the connected devices to find the VEX V5 Brain
        for (int i = 0; i < device_count; i++)
        {
            libusb_device_descriptor device_descriptor{};
            // Get the device descriptor. If LIBUSBX_API_VERSION >= 0x01000102 then this function will never fail.
            if (libusb_get_device_descriptor(devices[i], &device_descriptor) != LIBUSB_SUCCESS) continue;

            if (device_descriptor.idVendor == VEX_USB_VENDOR_ID)
            {
                libusb_device_handle* handle = nullptr;
                if (int error = libusb_open(devices[i], &handle) != LIBUSB_SUCCESS)
                {
                    printf("Failed to open VEX Brain: %s\n", libusb_error_name(error));
                    // TODO: If we find it can error, we should make a loop to try to open multiple times
                    return;
                }
                this->device_handle = handle;
                printf("Got device handle!\n");
                break;
            }
        }

        // All devices start with ref count of 1, this subtracts 1 so it dereferences them all
        libusb_free_device_list(devices, 1);

        if (!this->device_handle) printf("Failed to find vex brain!\n");
#endif
}

SerialHandler::~SerialHandler() {
#if PI
    if (this->device_handle)
    {
        libusb_close(this->device_handle);
        libusb_exit(nullptr);
    }
#endif
}

void SerialHandler::receive() {
    uint8_t in = ' ';  // Current byte being read
    std::vector<uint8_t> bytes;

    // Read until we get a null byte (0x00)—end of packet
    // TODO: It would be faster to read more than 1 byte at a time, but I'm not sure if the benefit outweighs the cost to search the data for the null byte.
    while (in != '\0') {
        ssize_t num_read = 0;

        #if PI
                libusb_bulk_transfer(this->device_handle, VEX_USB_USER_DATA_ENDPOINT_IN, &in, 1, reinterpret_cast<int*>(&num_read), 0);
        #endif
        #if BRAIN
                num_read = read(STDIN_FILENO, &in, 1);
        #endif

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
