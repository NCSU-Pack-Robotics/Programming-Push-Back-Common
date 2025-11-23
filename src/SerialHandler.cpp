#include "../SerialHandler.hpp"

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
    if (const int error = libusb_init_context(nullptr, nullptr, 0) != LIBUSB_SUCCESS) {
        printf("Failed to initialize libusb context: %s\n", libusb_error_name(error));
        return;
    }

    // Get the list of devices on the system
    libusb_device** devices = nullptr;
    const ssize_t device_count = libusb_get_device_list(nullptr, &devices);
    if (device_count < 0) {
        printf("Failed to get device list: %s\n", libusb_error_name(static_cast<int>(device_count)));
        return;
    }

    // TODO: We should probably retry initializing the context and getting the device list if they ever fail.

    // Loop the connected devices to find the VEX V5 Brain
    for (int i = 0; i < device_count; i++) {
        libusb_device_descriptor device_descriptor{};
        // Get the device descriptor. If LIBUSBX_API_VERSION >= 0x01000102 then this function will never fail.
        if (libusb_get_device_descriptor(devices[i], &device_descriptor) != LIBUSB_SUCCESS)
            continue;

        if (device_descriptor.idVendor == VEX_USB_VENDOR_ID) {
            libusb_device_handle* handle = nullptr;
            if (const int error = libusb_open(devices[i], &handle) != LIBUSB_SUCCESS) {
                printf("Failed to open VEX Brain: %s\n", libusb_error_name(error));
                // TODO: If we find it can error, we should make a loop to try to open multiple times
                return;
            }
            this->device_handle = handle;

            libusb_detach_kernel_driver(handle, VEX_USB_USER_INTERFACE_NUMBER);
            libusb_detach_kernel_driver(handle, VEX_USB_USER_DATA_INTERFACE_NUMBER);

            // Since this is output, line_coding_bytes will not be modified by libusb_control_transfer
            libusb_control_transfer(
                handle, LIBUSB_RECIPIENT_INTERFACE | LIBUSB_REQUEST_TYPE_CLASS | LIBUSB_ENDPOINT_OUT,
                SET_LINE_CODING, 0, VEX_USB_COMMUNICATIONS_INTERFACE_NUMBER, line_coding_bytes,
                sizeof(line_coding_bytes), 0);

            libusb_control_transfer(
                handle, LIBUSB_RECIPIENT_INTERFACE | LIBUSB_REQUEST_TYPE_CLASS | LIBUSB_ENDPOINT_OUT,
                SET_LINE_CODING, 0, VEX_USB_USER_INTERFACE_NUMBER, line_coding_bytes, sizeof(line_coding_bytes), 0);
            break;

        }
    }

    // All devices start with ref count of 1, this subtracts 1 so it dereferences them all
    libusb_free_device_list(devices, 1);

    if (!this->device_handle)
        printf("Failed to find vex brain!\n");
#endif
}

SerialHandler::~SerialHandler() {
#if PI
    if (this->device_handle) {
        libusb_close(this->device_handle);
        libusb_exit(nullptr);
    }
#endif
}

void SerialHandler::send(const Packet& packet) {
    // Create enough space to store the entire packet
    std::vector<uint8_t> data_to_send(sizeof(packet.header) + packet.data.size());

    // Copy header and data into the byte array
    memcpy(data_to_send.data(), &packet.header, sizeof(packet.header));
    memcpy(data_to_send.data() + sizeof(packet.header), packet.data.data(), packet.data.size());

    // Frame the packet using COBS
    std::optional<std::vector<uint8_t>> encoded = Utils::cobs_encode(data_to_send);
    if (!encoded.has_value())
        return;

    // Write the data to the serial connection
    // TODO: Both of these functions have a possibility to not fully send all of the data, we would need to resend the part not sent
    #if BRAIN
    write(STDOUT_FILENO, encoded->data(), encoded->size());
    #elif PI
    libusb_bulk_transfer(this->device_handle, VEX_USB_USER_DATA_ENDPOINT_OUT, encoded->data(),
                         static_cast<int>(encoded->size()), nullptr, 0);
    #endif
}

void SerialHandler::receive() {
    // Get a pointer to the first null byte in the buffer
    auto zero_ptr = reinterpret_cast<unsigned char*>(strchr(reinterpret_cast<char*>(this->buffer), '\0'));
    while (!zero_ptr // If a null byte is not found in the buffer
        || zero_ptr - this->buffer >= this->next_write_index) { // If the null byte that gets found is past the amount of data we actually read

        int num_read = 0;
        // Reading the MAX_PACKET_SIZE is important so that libusb does not throw an error for not having enough room for the data.
        // We read to buffer + an offset in case the packet we are reading spans multiple libusb packets

        #if PI
        const int res = libusb_bulk_transfer(this->device_handle, VEX_USB_USER_DATA_ENDPOINT_IN,
                                       this->buffer + this->next_write_index, MAX_LIBUSB_PACKET_SIZE, &num_read, 0);
        if (res)
            printf("Error: %s\n", libusb_error_name(res));

        #elif BRAIN
        num_read = read(STDIN_FILENO, this->buffer + this->next_write_index, MAX_LIBUSB_PACKET_SIZE);
        #endif

        this->next_write_index += num_read;

        // TODO: Handle EOF or other errors

        zero_ptr = reinterpret_cast<unsigned char*>(strchr(reinterpret_cast<char*>(this->buffer), '\0'));
    }

    const int packet_length = zero_ptr - this->buffer + 1;
    std::vector<uint8_t> bytes(packet_length);

    // Copy the bytes into the bytes vector, excluding the null delimiter
    memcpy(bytes.data(), this->buffer, packet_length - 1);

    // In case we read multiple packets in 1 libusb packet, move the data between the end of our current packet, and the total bytes read, to the beginning of the buffer
    memmove(this->buffer, this->buffer + packet_length, this->next_write_index - packet_length);
    this->next_write_index -= packet_length;

    const std::optional<std::vector<uint8_t>> decoded = Utils::cobs_decode(bytes);
    if (!decoded.has_value()) return; // If we fail to decode, ignore the packet

    // Decode the header
    Header received_header{};
    memcpy(&received_header, decoded->data(), sizeof(received_header));
    const uint8_t* ptr = decoded->data();
    const Packet received_packet{received_header, ptr + sizeof(received_header),
                                 decoded->size() - sizeof(received_header)};

    this->buffers[received_header.packet_id].add(received_packet);

    auto it = this->listeners.find(received_header.packet_id);
    if (it != this->listeners.end()) {
        it->second(*this, received_packet);
    }
}

std::optional<Packet> SerialHandler::pop_latest(const PacketId packet_id) {
    return this->buffers[packet_id].pop_latest();
}

bool SerialHandler::add_listener(PacketId packet_id, const std::function<void(SerialHandler& serial_handler, const Packet&)>& listener) {
    if (this->listeners.contains(packet_id)) return false;
    this->listeners[packet_id] = listener;
    return true;
}

bool SerialHandler::remove_listener(PacketId packet_id) {
    auto it = this->listeners.find(packet_id);
    if (it == this->listeners.end()) return false;
    this->listeners.erase(it);
    return true;
}
