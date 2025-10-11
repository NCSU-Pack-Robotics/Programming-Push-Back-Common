#pragma once

#include <any>

#include <cstdint>
#include <cstring>
#include <functional>
#include <typeindex>
#include <unistd.h>
#include <libusb-1.0/libusb.h>

#include "Buffer.hpp"
#include "packet/Packet.hpp"


/**
 * A USB device is structured as follows:
 * Highest level is a device, which is the actual usb, such as a mouse, headphones, etc.
 * Each device can have multiple configurations. In our code we use libusb_get_active_config_descriptor
 * to get the active one.
 * A configuration contains multiple interfaces, which are used for different things. For example a
 * webcam may have an interface for video, and another for audio.
 * Each interface has endpoints, which is what is used to actually send and receive data. An endpoint can only
 * send or receive data, not both. One exception is the endpoint at number 0, the control endpoint, which is required
 * to be bidirectional.
 * An endpoint address is 8 bits, with the MSB used as a input/output (1/0) flag. Bits 3:0 are used for the endpoint
 * number, and the rest of the bits are reserved.
*/

// "udevadm info -a -n /dev/ttyACMx" can be used to print information about the device

/** Every device under VEX has the same vendor ID */
// This can be checked here by searching for VEX: https://the-sz.com/products/usbid/index.php
constexpr uint16_t VEX_USB_VENDOR_ID = 0x2888;

// These numbers could only change on firmware updates, but that is unlikely because it would
// probably break other code.
/** The communications interface. Possibly used for uploading files. Unused in our code. */
constexpr uint8_t VEX_USB_COMMUNICATIONS_INTERFACE_NUMBER = 0x00;
/** The input endpoint for the communications interface, with type interrupt. */
constexpr uint8_t VEX_USB_COMMUNICATIONS_ENDPOINT_IN = 0x81;

/** The interface 1 after contains the actual input and output endpoints. */
constexpr uint8_t VEX_USB_COMMUNICATIONS_DATA_INTERFACE_NUMBER = 0x01;
/** The input endpoint for the communications data interface, with type bulk. */
constexpr uint8_t VEX_USB_COMMUNICATIONS_DATA_ENDPOINT_IN = 0x82;
/** The output endpoint for the communications data interface, with type bulk. */
constexpr uint8_t VEX_USB_COMMUNICATIONS_DATA_ENDPOINT_OUT = 0x03;

/** The user interface. Used to send and receive data to the PI. */
constexpr uint8_t VEX_USB_USER_INTERFACE_NUMBER = 0x02;
/** The input endpoint for the user interface, with type interrupt. */
constexpr uint8_t VEX_USB_USER_ENDPOINT_IN = 0x84;

/** The interface 1 after contains the actual input and output endpoints. */
constexpr uint8_t VEX_USB_USER_DATA_INTERFACE_NUMBER = 0x03;
/** The input endpoint for the user data interface, with type bulk. */
constexpr uint8_t VEX_USB_USER_DATA_ENDPOINT_IN = 0x85;
/** The output endpoint for the user data interface, with type bulk. */
constexpr uint8_t VEX_USB_USER_DATA_ENDPOINT_OUT = 0x06;

class SerialHandler {
public:
    /**
     * Constructs a SerialHandler for the given device type.
     */
    SerialHandler();

    /** Cleans up the SerialHandler, closing any open file descriptors. */
    ~SerialHandler();

    /**
     * Sends the given packet over the serial connection.
     *
     * This method takes an entire packet as opposed to the data in order to focus the scope to transmission logic and
     * allow flexibility in packet structure later.
     *
     * @param packet The packet struct to send.
     * This is a reference to avoid unnecessary copying.
     * @tparam T The type of the packet struct to send.
     */
    void send(const Packet& packet) {
        // Create enough space to store the entire packet
        std::vector<uint8_t> data_to_send (2 * sizeof(packet.header) + packet.data.size());

        // Copy header and data into the byte array
        memcpy(data_to_send.data(), &packet.header, sizeof(packet.header));
        memcpy(data_to_send.data() + sizeof(packet.header), packet.data.data(), packet.data.size());

        // Frame the packet using COBS
        std::optional<std::vector<uint8_t>> encoded = cobs_encode(data_to_send);
        if (!encoded.has_value()) return;

        // Write the data to the serial connection
        // TODO: Both of these functions have a possibility to not fully send all of the data, we would need to resend the part not sent
        #if BRAIN
        write(STDOUT_FILENO, encoded->data(), encoded->size());
        #endif
        #if PI
        libusb_bulk_transfer(this->device_handle, VEX_USB_USER_DATA_ENDPOINT_OUT, encoded->data(), static_cast<int>(encoded->size()), nullptr, 0);
        #endif
    }

    /**
     * Blocking call that reads and handles a single packet
     * @return A packet of any data type. Use the ID from the header to decode which data the packet uses.
     */
    void receive();

    std::optional<Packet> pop_latest(PacketId packet_id);

    /** A map of packet ids to their Buffer */
    std::unordered_map<PacketId, Buffer> buffers;

private:
    #if PI
    /** A libusb device handle. */
    libusb_device_handle* device_handle;
    #endif
};
