#pragma once

#include <any>

#include <cstdint>
#include <cstring>
#include <functional>
#include <unistd.h>
#if PI
#include <libusb.h>
#endif

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

/** The maximum packet size in bytes that is supported by the Vex Brain. This is a hardware limitation. It is important to read at least this amount in bulk
 * transfers to avoid errors. */
constexpr int MAX_LIBUSB_PACKET_SIZE = 512;

/** The maximum packet size that we can use for our packets. */
constexpr size_t MAX_PACKET_SIZE = 1024;

/** The request ID for setting the line coding over the USB control endpoint. */
constexpr int SET_LINE_CODING = 0x20;

/** The payload for the set line coding command. This is required to be sent before the Vex Brain will recognise bulk transfers
 * as standard input/output. The data for set line coding is as follows:
 * The first 4 bytes are the baud rate, in this case 9600
 * The next byte is an enum for the amount of stop bits. 0x00 corresponds to 1 stop bit.
 * The next byte is an enum for the type of parity. 0x00 means no parity.
 * The final byte is the number of data bits, in this case 8.
 * Here's a good reference for these types: https://www.silabs.com/documents/public/application-notes/AN758.pdf
 */
inline unsigned char line_coding_bytes[] = {0x80, 0x25, 0x0, 0x0, 0x0, 0x0, 0x8};

class SerialHandler {
public:
    /** Constructs a SerialHandler for the given device type. */
    SerialHandler();

    /** Cleans up the SerialHandler, closing any open file descriptors. */
    ~SerialHandler();

    /**
     * Sends the given packet over the serial connection.
     *
     * @param header The header of the packet to send.
     * @param data The data to be sent in the packet.
     * @tparam T The type of the packet struct to send.
     */
    template <typename T>
    void send(Header header, T& data) {
        Packet packet{header, reinterpret_cast<uint8_t*>(&data), sizeof(data)};
        // Create enough space to store the entire packet
        std::vector<uint8_t> data_to_send(sizeof(packet.header) + packet.data.size());

        // Copy header and data into the byte array
        memcpy(data_to_send.data(), &packet.header, sizeof(packet.header));
        memcpy(data_to_send.data() + sizeof(packet.header), packet.data.data(), packet.data.size());

        // Frame the packet using COBS
        std::optional<std::vector<uint8_t>> encoded = cobs_encode(data_to_send);
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


    /**
     * Blocking call that reads and handles a single packet
     * @return A packet of any data type. Use the ID from the header to decode which data the packet uses.
     */
    void receive();

    /**
     * Returns and remove the last recieved packet from the appropriate buffer.
     * @param packet_id The type of backet (and buffer) to remove from.
     * @return The removed packet.
     */
    std::optional<Packet> pop_latest(PacketId packet_id);

    /** A map of packet ids to their Buffer */
    std::unordered_map<PacketId, Buffer> buffers;

private:
    #if PI
    /** A libusb device handle. */
    libusb_device_handle* device_handle;
    #endif

    /** An array of bytes that stores the data from receiving packets. */
    unsigned char buffer[MAX_PACKET_SIZE]{};

    /** The index in the buffer array where the next read data should be placed. */
    ssize_t next_write_index = 0;
};
