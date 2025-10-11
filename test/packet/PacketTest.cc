#include <gtest/gtest.h>

#include "../../packet/Packet.hpp"
#include "optical.hpp"
#include "../../PacketId.hpp"

/**
 * Tests the constructor of a packet with the ID and the full data.
 */
TEST(PacketTest, ConstructionIdAndData) {
    // Create a packet
    constexpr PacketId id = PacketId::OPTICAL;
    constexpr OpticalData data_struct = {.x = 420.69, .y = -123.456, .heading = 0};
    const Packet packet(id, reinterpret_cast<const uint8_t*>(&data_struct), sizeof(data_struct));

    // Grab the data from the constructed packet
    const uint8_t id_byte = static_cast<uint8_t>(packet.header.packet_id);
    const uint16_t checksumBytes = packet.header.checksum;
    const std::float64_t heading = packet.get_data<OpticalData>().heading;
    const std::float64_t x = packet.get_data<OpticalData>().x;
    const std::float64_t y = packet.get_data<OpticalData>().y;

    // Ensure the fields are what they should be
    EXPECT_EQ(id_byte, 0x00);
    EXPECT_NE(checksumBytes, 0x00);  // Checksum is likely not zero
    EXPECT_EQ(packet.data.size(), sizeof(data_struct));
    EXPECT_EQ(heading, 0);
    EXPECT_EQ(x, 420.69);
    EXPECT_EQ(y, -123.456);
}
