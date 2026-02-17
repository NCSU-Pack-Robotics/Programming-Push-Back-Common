#include "OpticalPacket.hpp"
#include "SerialHandler.hpp"
#include "TextPacket.hpp"
#include "gtest/gtest.h"

#include <gmock/gmock.h>


class UsbTransferMock : public UsbTransferWrapper {
public:
    MOCK_METHOD(int, libusb_bulk_transfer, (libusb_device_handle*, unsigned char, unsigned char*, int, int*, unsigned int), (const, override));
};

// test receiving a normal optical packet that fits in one libusb bulk transfer call
TEST(SerialHandlerTest, ReceiveOptical) {
    UsbTransferMock usb_mock;
    SerialHandler handler{&usb_mock}; // create a serial handler using the mock usb class

    OpticalPacket test_packet{1, 2, 3};
    auto encoded = Utils::cobs_encode(test_packet.serialize());
    ASSERT_NE(encoded, std::nullopt) << "cobs encoding failed";

    EXPECT_CALL(usb_mock, libusb_bulk_transfer)
        .Times(1) // the entire optical packet is sent in one libusb call
        .WillRepeatedly([&encoded](libusb_device_handle *dev_handle,
        unsigned char endpoint, unsigned char *data, int length,
        int *transferred, unsigned int timeout) -> int {
                // set our mock usb function to return the encoded bytes of the test packet
                std::memcpy(data, encoded->data(), encoded->size());
                if (transferred) *transferred = encoded->size();
                return 0;
        });

    handler.receive(); // call receive which will trigger our mocked function
    std::optional<Packet> received = handler.pop_latest<OpticalPacket>();
    OpticalPacket::Data received_data = received->get_data<OpticalPacket>();
    // make sure the received data is correct
    EXPECT_EQ(received_data.x, test_packet.get_data<OpticalPacket>().x);
    EXPECT_EQ(received_data.y, test_packet.get_data<OpticalPacket>().y);
    EXPECT_EQ(received_data.heading, test_packet.get_data<OpticalPacket>().heading);
}

// test receiving a large packet that takes multiple libusb calls to store the whole thing
// this test also tests that a packet of MAX_PACKET_SIZE doesnt overflow the internal buffer
TEST(SerialHandlerTest, ReceiveLargeSplit) {
    UsbTransferMock usb_mock;
    SerialHandler handler{&usb_mock}; // create a serial handler using the mock usb class

    std::array<char, SerialHandler::MAX_PACKET_SIZE> large_data{}; // must be < MAX_PACKET_SIZE as defined in SerialHandler
    // create some large test data with a few values to test for later
    large_data[187] = 'a';
    large_data[200] = 'b';
    large_data[511] = 'c';
    TextPacket test_packet{large_data};
    auto encoded = Utils::cobs_encode(test_packet.serialize());
    ASSERT_NE(encoded, std::nullopt) << "cobs encoding failed";

    unsigned int total_bytes_sent{};

    EXPECT_CALL(usb_mock, libusb_bulk_transfer)
        // make sure the libusb method is called the right amount of times
        .Times(std::ceil(encoded->size() / 10.0))
        .WillRepeatedly([&encoded, &total_bytes_sent](libusb_device_handle *dev_handle,
        unsigned char endpoint, unsigned char *data, int length,
        int *transferred, unsigned int timeout) -> int {
                // each part is a maximum of 10 bytes, or smaller if there is not 10 bytes left
                int bytes_sent = std::min<int>(10, encoded->size() - total_bytes_sent);
                std::memcpy(data, encoded->data() + total_bytes_sent, bytes_sent);
                total_bytes_sent += bytes_sent;
                if (transferred) *transferred = bytes_sent;
                return 0;
        });

    handler.receive();
    std::optional<Packet> received = handler.pop_latest<TextPacket>();
    EXPECT_NE(received, std::nullopt) << "failed to find TextPacket in buffers";
    TextPacket::Data received_data = received->get_data<TextPacket>();
    // make sure the received data is correct
    EXPECT_EQ(received_data.text[187], 'a');
    EXPECT_EQ(received_data.text[200], 'b');
    EXPECT_EQ(received_data.text[511], 'c');
}

// test that receiving data that is not a valid packet does not crash the program
TEST(SerialHandlerTest, ReceiveGarbageData) {
    UsbTransferMock usb_mock;
    SerialHandler handler{&usb_mock}; // create a serial handler using the mock usb class

    // mimic someone accidentally leaving in a print statement on the brain
    const char print_msg[]{"hello world!"};

    EXPECT_CALL(usb_mock, libusb_bulk_transfer)
        .WillRepeatedly([print_msg](libusb_device_handle *dev_handle,
        unsigned char endpoint, unsigned char *data, int length,
        int *transferred, unsigned int timeout) -> int {
                // set our mock usb function to return the encoded bytes of the test packet
                std::memcpy(data, print_msg, sizeof(print_msg));
                if (transferred) *transferred = sizeof(print_msg);
                return 0;
        });

    // make sure this does not crash the program
    handler.receive();
}


// TODO:
// test that callbacks work
// test that buffers are populated in correct order
// test mocking the read method
// test the SerialHandler constructor when vex brain doesnt exist and other errors
