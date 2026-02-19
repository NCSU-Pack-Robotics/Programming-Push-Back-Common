#pragma once

#include <functional>
#include <unistd.h>
#if PI
#include <libusb.h>
#endif

#include "Buffer.hpp"
#include "Packet.hpp"


#if PI
// helper methods used for mocking usb methods in gtest
class UsbTransferWrapper {
public:
    virtual ~UsbTransferWrapper() = default;
    // TODO: Also wrap read and write calls
    virtual int libusb_bulk_transfer(libusb_device_handle *dev_handle,
        unsigned char endpoint, unsigned char *data, int length,
        int *transferred, unsigned int timeout) const = 0;
};

class UsbTransferProd : public UsbTransferWrapper {
public:
    int libusb_bulk_transfer(libusb_device_handle *dev_handle,
        unsigned char endpoint, unsigned char *data, int length,
        int *transferred, unsigned int timeout) const override {
        return ::libusb_bulk_transfer(dev_handle, endpoint, data, length, transferred, timeout);
    }
};
#endif

class SerialHandler {
public:
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
    static constexpr uint16_t VEX_USB_VENDOR_ID = 0x2888;

    // These numbers could only change on firmware updates, but that is unlikely because it would
    // probably break other code.
    /** The communications interface. Possibly used for uploading files. Unused in our code. */
    static constexpr uint8_t VEX_USB_COMMUNICATIONS_INTERFACE_NUMBER = 0x00;
    /** The input endpoint for the communications interface, with type interrupt. */
    static constexpr uint8_t VEX_USB_COMMUNICATIONS_ENDPOINT_IN = 0x81;

    /** The interface 1 after contains the actual input and output endpoints. */
    static constexpr uint8_t VEX_USB_COMMUNICATIONS_DATA_INTERFACE_NUMBER = 0x01;
    /** The input endpoint for the communications data interface, with type bulk. */
    static constexpr uint8_t VEX_USB_COMMUNICATIONS_DATA_ENDPOINT_IN = 0x82;
    /** The output endpoint for the communications data interface, with type bulk. */
    static constexpr uint8_t VEX_USB_COMMUNICATIONS_DATA_ENDPOINT_OUT = 0x03;

    /** The user interface. Used to send and receive data to the PI. */
    static constexpr uint8_t VEX_USB_USER_INTERFACE_NUMBER = 0x02;
    /** The input endpoint for the user interface, with type interrupt. */
    static constexpr uint8_t VEX_USB_USER_ENDPOINT_IN = 0x84;

    /** The interface 1 after contains the actual input and output endpoints. */
    static constexpr uint8_t VEX_USB_USER_DATA_INTERFACE_NUMBER = 0x03;
    /** The input endpoint for the user data interface, with type bulk. */
    static constexpr uint8_t VEX_USB_USER_DATA_ENDPOINT_IN = 0x85;
    /** The output endpoint for the user data interface, with type bulk. */
    static constexpr uint8_t VEX_USB_USER_DATA_ENDPOINT_OUT = 0x06;

    /** The maximum packet size in bytes that is supported by the Vex Brain. This is a hardware limitation. It is important to read at least this amount in bulk
     * transfers to avoid errors. */
    static constexpr int MAX_LIBUSB_PACKET_SIZE = 512;

    /** The maximum packet size that we can use for our packets. */
    static constexpr size_t MAX_PACKET_SIZE = 1024;
    /** The maximum packet that can be sent is MAX_PACKET_SIZE, but the total is higher
    * to account for the increased size from cobs encoding
    * - +2 bytes from the start and end byte
    * - +5 bytes from ceil(1024 / 254). See Utils::cobs_encode for more details
    * TODO: In the future it may be better to make the buffer dynamically sized to avoid this confusion
    */
    static constexpr size_t MAX_ENCODED_PACKET_SIZE = 1024 + 2 + 5;
    /** The max size in bytes that the data of a packet can be so that once its encoded it doesn't go over MAX_PACKET_SIZE */
    static constexpr size_t MAX_PACKET_DATA_SIZE = MAX_PACKET_SIZE - sizeof(Header);


    /** The request ID for setting the line coding over the USB control endpoint. */
    static constexpr int SET_LINE_CODING = 0x20;

    /** The payload for the set line coding command. This is required to be sent before the Vex Brain will recognize bulk transfers
     * as standard input/output. The data for set line coding is as follows:
     * The first 4 bytes are the baud rate, in this case 9600
     * The next byte is an enum for the amount of stop bits. 0x00 corresponds to 1 stop bit.
     * The next byte is an enum for the type of parity. 0x00 means no parity.
     * The final byte is the number of data bits, in this case 8.
     * Here's a good reference for these types: https://www.silabs.com/documents/public/application-notes/AN758.pdf
     */
    static constexpr unsigned char line_coding_bytes[] = {0x80, 0x25, 0x0, 0x0, 0x0, 0x0, 0x8};


    /** Constructs the serial handler. Leave the default argument unless you want to use the gtest usb wrappers */
    SerialHandler(
#if PI
        const UsbTransferWrapper* usb_wrapper = &default_wrapper
#endif
        );

    /** Cleans up the SerialHandler, closing the libusb device handle if running on the pi. */
    ~SerialHandler();

    /**
     * Sends the given packet over the serial connection.
     *
     * @param packet A reference to the packet to transmit.
     */
    void send(const Packet& packet);

#if BRAIN
    /**
     * Non-blocking call that will read a single packet if there is one available, and return instantly.
     * @returns true if it read a packet, false if no packet was available.
     */
    bool try_receive();
#endif
    /**
     * Blocking call that reads a single packet.
     * If the packet has listeners registered to it, they will execute before this function returns.
     * Note: It is possible that a packet fails to decode after being read, this function will return
     * regardless of the success of decoding.
     */
    void receive();

    /**
     * Returns and remove the last received packet from the appropriate buffer.
     * @return The removed packet.
     */
    template <typename T>
    std::optional<Packet> pop_latest()
    {
        return this->buffers[T::id].pop_latest();
    }

    /**
     * Adds an event listener to the list. There can only be one listener for each packet id.
     * @Returns True if it was successfully added, or false if a listener for that id already exists.
     */
    template <typename T>
    bool add_listener(const std::function<void(SerialHandler& serial_handler, const Packet&)>& listener)
    {
        if (this->listeners[T::id]) return false;
        this->listeners[T::id] = listener;
        return true;
    }

    /**
     * Removes a listener from the list.
     * @Returns True if the listener was removed, or false if no listener exits with that id.
     */
    template <typename T>
    bool remove_listener()
    {
        if (this->listeners[T::id])
        {
            this->listeners[T::id] = nullptr; // Put the function in an empty state
            return true;
        }
        return false;
    }

private:
    /**
     * Helper function used in try_receive and receive to decode a packet after one has been found.
     * @param packet_end Pointer in buffer to the inclusive end of the packet
     */
    void decode_packet(const unsigned char* packet_end);

    #if PI
    /** A libusb device handle. */
    libusb_device_handle* device_handle;
    #endif

    /** An array where the indices of the array correspond to the packet id whose buffer is stored there */
    std::array<Buffer, PacketIds::LENGTH> buffers;

    /** An array where the indices of the array correspond to the packet id whose listener is stored there */
    std::array<std::function<void(SerialHandler& serial_handler, const Packet&)>, PacketIds::LENGTH> listeners;

    /** An array of bytes that stores the data from receiving packets. Used temporarily between calls to libusb_block_transfer when receiving */
    unsigned char buffer[MAX_ENCODED_PACKET_SIZE]{};
    /** The index in the buffer array where the next read data should be placed. */
    ssize_t next_write_index = 0;

#if PI
    static constexpr UsbTransferProd default_wrapper{};
    const UsbTransferWrapper* usb_wrapper;
#endif
};
