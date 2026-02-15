#include <gtest/gtest.h>

#include "Packet.hpp"
#include "PacketIds.hpp"
#include "packets/OpticalPacket.hpp"


/** Data used in all the optical tests as the data to compare against */
static OpticalPacket::Data OPTICAL_TEST_DATA{420.69, -123.456, 0.0012};

// using this fixture makes it easier for the tests to call this function which requires access to private members,
// without friending each individual class
class PacketTest : public testing::Test {
protected:
    /**
     * Helper method to check the contents of a packet.
     * The data should be equal to OPTICAL_TEST_DATA, and
     * the serialization and data size should be correct
     * @param packet The packet whose contents to check. This should only be an OpticalPacket, but is left as a Packet,
     * because it is tested from a raw constructed packet as well
     */
    static void test_optical_packet(const Packet& packet) {
        // Grab the data from the given packet
        const uint8_t id_byte = packet.get_id();
        const std::float64_t x = packet.get_data<OpticalPacket>().x;
        const std::float64_t y = packet.get_data<OpticalPacket>().y;
        const std::float64_t heading = packet.get_data<OpticalPacket>().heading;

        // Check that the sizes are correct
        EXPECT_EQ(packet.data.size(), sizeof(OpticalPacket::Data)) << "Packet internal data size mismatch";
        EXPECT_EQ(packet.serialize().size(), packet.data.size() + sizeof(packet.header)) << "Packet serialized data size mismatch";

        // Ensure the fields are what they should be
        EXPECT_EQ(id_byte, PacketIds::OPTICAL) << "ID Should be OPTICAL.";
        EXPECT_EQ(heading, OPTICAL_TEST_DATA.heading) << "decoded header incorrectly";
        EXPECT_EQ(x, OPTICAL_TEST_DATA.x) << "decoded x incorrectly";
        EXPECT_EQ(y, OPTICAL_TEST_DATA.y) << "decoded y incorrectly";
    }
};


// test constructing packet and then reading data back from bytes
TEST_F(PacketTest, ConstructingFromData) {
    const OpticalPacket packet{OPTICAL_TEST_DATA.x, OPTICAL_TEST_DATA.y, OPTICAL_TEST_DATA.heading};
    test_optical_packet(packet);
}

// test constructing packet from bytes and reading data
TEST_F(PacketTest, ConstructingFromBytes) {
    std::vector<uint8_t> bytes(sizeof(std::float64_t) * 3);
    // make the data we want to test for
    std::float64_t x = OPTICAL_TEST_DATA.x;
    std::float64_t y = OPTICAL_TEST_DATA.y;
    std::float64_t heading = OPTICAL_TEST_DATA.heading;
    // copy the data as raw bytes into our vector
    std::memcpy(bytes.data(), &x, sizeof(std::float64_t));
    std::memcpy(bytes.data() + sizeof(std::float64_t), &y, sizeof(std::float64_t));
    std::memcpy(bytes.data() + sizeof(std::float64_t) * 2, &heading, sizeof(std::float64_t));
    // construct using the constructor that takes bytes and a length
    const Packet packet{Header{PacketIds::OPTICAL}, bytes.data(), bytes.size()};
    test_optical_packet(packet);
}

// test constructing from the data struct, the normal way packets are constructed from their derived classes
TEST_F(PacketTest, ConstructingFromDataStruct) {
    const OpticalPacket::Data data{OPTICAL_TEST_DATA.x, OPTICAL_TEST_DATA.y, OPTICAL_TEST_DATA.heading};
    const Packet packet{Header{PacketIds::OPTICAL}, data};
    test_optical_packet(packet);
}
