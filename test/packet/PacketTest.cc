#include <gtest/gtest.h>

#include "../../packet/Packet.hpp"
#include "optical.hpp"
#include "../../PacketId.hpp"

/**
 * Helper method to check the contents of a packet.
 * - The expected id is <code>OPTICAL</code>.
 * - The expected checksum is non-zero.
 * - The expected data is: <code>{.x = 420.69, .y = -123.456, .heading = 0}</code>
 * @param packet The packet whose contents to check.
 */
static void check_contents(const Packet& packet);

/** Data to use for testing. */
constexpr OpticalData DATA_STRUCT = {.x = 420.69, .y = -123.456, .heading = 0};

/** Tests the constructor of a packet with the ID and the full data. */
TEST(PacketTest, constructor_id_and_data) {
    // Create a packet
    constexpr auto id = PacketId::OPTICAL;
    const Packet packet(id, reinterpret_cast<const uint8_t*>(&DATA_STRUCT), sizeof(DATA_STRUCT));

    check_contents(packet);
}

/** Tests the constructor of a packet with the full header and full data. */
TEST(PacketTest, constructor_header_and_data) {
    // Create necessary parts of a packet
    constexpr auto id = PacketId::OPTICAL;
    constexpr Header header = {id};

    // Convert data to vector
    const auto data_ptr = reinterpret_cast<const uint8_t*>(&DATA_STRUCT);
    const std::vector data_vector(data_ptr, data_ptr + sizeof(DATA_STRUCT));

    // Construct the packet
    const Packet packet(header, data_vector.data(), data_vector.size());

    check_contents(packet);
}

/** Checks if the `get_data` method returns the byte array as the given type properly */
TEST(PacketTest, get_data) {
    const Packet packet(PacketId::OPTICAL, reinterpret_cast<const uint8_t*>(&DATA_STRUCT), sizeof(DATA_STRUCT));

    const auto [x, y, heading] = packet.get_data<OpticalData>();
    EXPECT_TRUE(DATA_STRUCT.heading == heading);
    EXPECT_TRUE(DATA_STRUCT.x == x);
    EXPECT_TRUE(DATA_STRUCT.y == y);
}

static void check_contents(const Packet& packet) {
    // Grab the data from the given packet
    const uint8_t id_byte = static_cast<uint8_t>(packet.header.packet_id);
    const std::float64_t heading = packet.get_data<OpticalData>().heading;
    const std::float64_t x = packet.get_data<OpticalData>().x;
    const std::float64_t y = packet.get_data<OpticalData>().y;

    // Ensure the fields are what they should be
    EXPECT_EQ(id_byte, 0x00) << "ID Should be OPTICAL.";
    EXPECT_EQ(packet.data.size(), sizeof(OpticalData));
    EXPECT_EQ(heading, DATA_STRUCT.heading);
    EXPECT_EQ(x, DATA_STRUCT.x);
    EXPECT_EQ(y, DATA_STRUCT.y);
}
