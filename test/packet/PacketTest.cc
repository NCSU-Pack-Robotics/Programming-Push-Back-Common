#include <gtest/gtest.h>

#include "Packet.hpp"
#include "PacketIds.hpp"
#include "packets/OpticalPacket.hpp"

/**
 * Helper method to check the contents of a packet.
 * - The expected id is <code>OPTICAL</code>.
 * - The expected checksum is non-zero.
 * - The expected data is: <code>{.x = 420.69, .y = -123.456, .heading = 0}</code>
 * @param packet The packet whose contents to check.
 */
static void check_contents(const Packet& packet);

/** Data to use for testing. */
constexpr OpticalPacket::Data DATA_STRUCT = {.x = 420.69, .y = -123.456, .heading = 0};

/** Tests the constructor of a packet with the ID and the full data. */
TEST(PacketTest, constructor_id_and_data) {
    // Create a packet
    const OpticalPacket packet{DATA_STRUCT};

    check_contents(packet);

    // Ensure the implicit construction of a packet like this works
    const OpticalPacket* implicit_packet = nullptr;
    bool worked = false;
    EXPECT_NO_THROW({
        implicit_packet = new OpticalPacket{DATA_STRUCT};
        worked = true;
    });
    if (worked && implicit_packet) check_contents(*implicit_packet);
}

/** Tests the constructor of a packet with the full header and full data. */
TEST(PacketTest, constructor_header_and_data) {
    // Create necessary parts of a packet
    constexpr auto id = PacketIds::OPTICAL;
    constexpr Header header = {id};

    // Construct the packet
    const OpticalPacket packet(DATA_STRUCT);

    check_contents(packet);
}

TEST(PacketTest, constructor_header_and_bytes) {
    // Create necessary parts of a packet
    constexpr auto id = PacketIds::OPTICAL;
    constexpr Header header = {id};

    // Create byte array from data struct
    const auto data_bytes = reinterpret_cast<const uint8_t*>(&DATA_STRUCT);

    // Construct the packet
    const Packet packet(header, data_bytes, sizeof(OpticalPacket::Data));

    check_contents(packet);
}

/** Checks if the `get_data` method returns the byte array as the given type properly */
TEST(PacketTest, get_data) {
    const OpticalPacket packet(DATA_STRUCT);

    const auto [x, y, heading] = packet.get_data<OpticalPacket::Data>();
    EXPECT_TRUE(DATA_STRUCT.heading == heading);
    EXPECT_TRUE(DATA_STRUCT.x == x);
    EXPECT_TRUE(DATA_STRUCT.y == y);
}

static void check_contents(const Packet& packet) {
    // Grab the data from the given packet
    const uint8_t id_byte = packet.get_id();
    const std::float64_t heading = packet.get_data<OpticalPacket::Data>().heading;
    const std::float64_t x = packet.get_data<OpticalPacket::Data>().x;
    const std::float64_t y = packet.get_data<OpticalPacket::Data>().y;

    // Ensure the fields are what they should be
    EXPECT_EQ(id_byte, 0x00) << "ID Should be OPTICAL.";
    // TODO: Find a way to friend gtest so it can access private fields somehow?
    // EXPECT_EQ(packet.data.size(), sizeof(OpticalPacket::Data));
    EXPECT_EQ(heading, DATA_STRUCT.heading);
    EXPECT_EQ(x, DATA_STRUCT.x);
    EXPECT_EQ(y, DATA_STRUCT.y);
}
