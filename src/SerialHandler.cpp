#include "SerialHandler.hpp"

#include <cstring>
#include <optional>
#include <unistd.h>
#include <vector>
#include <cassert>

SerialHandler::SerialHandler(
#if PI
    const UsbTransferWrapper* usb_wrapper
#endif
    )
#if PI
    :
    device_handle(nullptr),
    usb_wrapper(usb_wrapper)
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
                SET_LINE_CODING, 0, VEX_USB_COMMUNICATIONS_INTERFACE_NUMBER, const_cast<unsigned char*>(line_coding_bytes),
                sizeof(line_coding_bytes), 0);

            libusb_control_transfer(
                handle, LIBUSB_RECIPIENT_INTERFACE | LIBUSB_REQUEST_TYPE_CLASS | LIBUSB_ENDPOINT_OUT,
                SET_LINE_CODING, 0, VEX_USB_USER_INTERFACE_NUMBER, const_cast<unsigned char*>(line_coding_bytes), sizeof(line_coding_bytes), 0);
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
    if (this->device_handle) {
        libusb_close(this->device_handle);
        libusb_exit(nullptr);
    }
#endif
}


// TODO: To be safe, sent packets should begin with a null byte to end the previous data, in the case tha theres unknown data
// before it. It also couldn't hurt to add a small set of signature bytes to prefix a packet, to prevent junk data
// having the chance to be a valid packet id and pollute the buffers
void SerialHandler::send(const Packet& packet) {
    std::vector<uint8_t> data_to_send = packet.serialize();

    assert(data_to_send.size() <= MAX_PACKET_SIZE && "Cannot send a packet with size greater than max packet size!");

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


#if BRAIN
bool SerialHandler::try_receive() {
    ssize_t num_read = read(STDIN_FILENO, this->buffer + this->next_write_index, MAX_LIBUSB_PACKET_SIZE);

    if (num_read <= 0) {
        // handle errors
        return false;
    }

    this->next_write_index += num_read;
    // if the next write index is > the size then something has gone wrong on the senders side. the entire message will
    // be discarded, because if index keeps growing we will exceed the length of the buffer. this is not an issue if the sender
    // doesnt send data > max_packer_size
    if (this->next_write_index >= MAX_PACKET_SIZE) {
        this->next_write_index = 0;
        return false;
    }

    auto it = std::ranges::find(this->buffer, '\0');
    // make sure the packet has ended
    if (it == std::ranges::end(this->buffer) || it - this->buffer >= this->next_write_index)
        return false;

    this->decode_packet(it);
    return true;
}
#endif

void SerialHandler::receive() {
    // Get a pointer to the first null byte in the buffer
    auto it = std::ranges::find(this->buffer, '\0');
    while (it == std::ranges::end(this->buffer) // If a null byte is not found in the buffer
        || it - this->buffer >= this->next_write_index) { // If the null byte that gets found is past the amount of data we actually read

        int num_read = 0;
        // Reading the MAX_PACKET_SIZE is important so that libusb does not throw an error for not having enough room for the data (and cause undefined behavior)
        // https://libusb.sourceforge.io/api-1.0/libusb_packetoverflow.html
        // We read to buffer + an offset in case the packet we are reading spans multiple libusb packets

        #if PI
        const int res = usb_wrapper->libusb_bulk_transfer(this->device_handle, VEX_USB_USER_DATA_ENDPOINT_IN,
                                       this->buffer + this->next_write_index, MAX_LIBUSB_PACKET_SIZE, &num_read, 0);
        if (res)
            printf("Error: %s\n", libusb_error_name(res));

        #elif BRAIN
        num_read = read(STDIN_FILENO, this->buffer + this->next_write_index, MAX_LIBUSB_PACKET_SIZE);
        if (num_read <= 0) {
            // possibly handle error if its -1
            continue;
            // continue, otherwise it could be -1 and then -1 gets added to next_write_index which messes up the buffer
            // this error handling might not be ideal because in some error cases the buffer might be written to partially before having an error occur
        }
        #endif



        // Since we have to read 512 bytes each libusb call, we need to make sure there is always 512 bytes available in the buffer
        this->next_write_index += num_read;
        if (this->next_write_index >= MAX_ENCODED_PACKET_SIZE) {
            this->next_write_index = 0;
            continue;
        }

        // TODO: Handle EOF or other errors

        it = std::ranges::find(this->buffer, '\0');
    }

    this->decode_packet(it);
}

void SerialHandler::decode_packet(const unsigned char* packet_end) {
    const int packet_length = packet_end - this->buffer; // length not including the null delimiter
    std::vector<uint8_t> bytes(packet_length);

    // Copy the bytes into the bytes vector, excluding the null delimiter
    memcpy(bytes.data(), this->buffer, packet_length);

    // In case we read multiple packets in 1 libusb packet, move the data between the end of our current packet, and the total bytes read, to the beginning of the buffer
    memmove(this->buffer, this->buffer + packet_length + 1, this->next_write_index - packet_length);
    this->next_write_index -= (packet_length + 1);

    const std::optional<std::vector<uint8_t>> decoded = Utils::cobs_decode(bytes);
    if (!decoded.has_value()) return; // If we fail to decode, ignore the packet

    // Decode the header
    Header received_header{};
    memcpy(&received_header, decoded->data(), sizeof(received_header));
    const uint8_t* ptr = decoded->data();
    Packet received_packet{received_header, ptr + sizeof(received_header),
                                 decoded->size() - sizeof(received_header)};

    // if the packet id does not exist, discard the packet
    if (received_packet.get_id() >= PacketIds::LENGTH) return;

    mutex.lock();
    // get the function before while locked
    const auto& fn = this->listeners[received_header.packet_id];
    this->buffers[received_header.packet_id].add(received_packet);
    mutex.unlock();

    // call the function while NOT locked, so a user doesn't call a method like pop_latest which requires a lock and causes a deadlock
    if (fn) { // test if function is valid
        fn(*this, received_packet);
    }
}
