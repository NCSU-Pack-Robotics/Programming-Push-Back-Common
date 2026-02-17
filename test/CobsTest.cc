#include <cstring>
#include <gtest/gtest.h>

#include "Utils.hpp"

// long string used to test encoding
static constexpr char lorem[] = "Lorem ipsum dolor sit amet, consectetur adipiscing elit. Cras nulla dui, convallis quis quam nec, bibendum auctor lectus. Nam porta justo libero, in efficitur neque fringilla et. Praesent malesuada dui id justo varius, semper imperdiet nulla ultricies. Aliquam erat volutpat. Aenean sagittis dui sit amet velit lacinia volutpat. Sed sem lectus, ultricies ac neque eu, lobortis tempor dui. Nunc faucibus venenatis lectus vel fermentum. Duis a imperdiet neque. Sed et efficitur tellus. Donec id fermentum felis, et pretium arcu. Integer eleifend eros ut enim pulvinar, ut sagittis purus egestas. Interdum et malesuada fames ac ante ipsum primis in faucibus. Integer ultrices diam est, id tincidunt nisi tincidunt et. Nunc ex risus, ornare vitae tellus non, porta luctus urna. Mauris massa mauris, iaculis ac interdum eget, pellentesque eu augue.Mauris sed odio gravida, ultricies elit eget, bibendum tellus. Integer tincidunt vitae dolor at interdum. Aliquam a ex vel sem tempus pretium id at tortor. Sed non dui eget nisl gravida laoreet eget eget dui. Pellentesque et quam mollis lectus ultricies pulvinar. Vestibulum accumsan dolor elit, a egestas odio pellentesque sed. Nunc congue ornare leo, vel euismod nulla elementum auctor. Praesent eget mauris posuere dui sodales consequat nec at arcu. In nisi orci, ullamcorper eget dui non, condimentum porttitor nunc. Suspendisse elementum venenatis lacus, non elementum nisi iaculis non. Nullam et sodales sapien. Praesent tempor ligula eu dignissim lacinia. Cras pharetra tincidunt iaculis. Interdum et malesuada fames ac ante ipsum primis in faucibus.";

// test a basic sequence of characters
TEST(CobsTest, EncodingDecodingBasic) {
    const std::vector<uint8_t> bytes{'h', 'i', '\0', 'b', '\0', 'y', 'e'};

    auto encoded = Utils::cobs_encode(bytes);
    ASSERT_NE(encoded, std::nullopt); // use assert here because we cannot decode if encoding fails

    encoded->pop_back(); // remove null delimiter

    auto decoded = Utils::cobs_decode(encoded.value());
    ASSERT_NE(decoded, std::nullopt);
    EXPECT_EQ(*decoded, bytes);
}

// test a large string. this tests that the block marker works.
TEST(CobsTest, EncodingDecodingLong) {
    std::vector<uint8_t> bytes(sizeof(lorem) + 1);
    std::memcpy(bytes.data(), lorem, bytes.size());
    bytes[257] = '\0'; // first null character is > 255 so there will be a 0xFF start byte

    auto encoded = Utils::cobs_encode(bytes);
    ASSERT_NE(encoded, std::nullopt); // use assert here because we cannot decode if encoding fails

    encoded->pop_back(); // remove null delimiter

    auto decoded = Utils::cobs_decode(encoded.value());
    ASSERT_NE(decoded, std::nullopt);
    EXPECT_EQ(*decoded, bytes);
}

// test encoding a struct instead of a string, and also test that a struct full of mostly zeros works
TEST(CobsTest, EncodingDecodingStruct) {
    struct TestStruct {
        uint8_t x;
        char chars[500];
        int32_t y;
        float z;
    };
    // create test structure instance
    TestStruct test{.x=1, .y=28376, .z=0.000001f};
    // add some random characters to check
    // the rest of chars is zeroed
    test.chars[250] = 't';
    test.chars[100] = 'w';
    test.chars[499] = 'a';

    std::vector<uint8_t> bytes(sizeof(TestStruct));
    std::memcpy(bytes.data(), &test, bytes.size());

    auto encoded = Utils::cobs_encode(bytes);
    ASSERT_NE(encoded, std::nullopt); // use assert here because we cannot decode if encoding fails

    encoded->pop_back(); // remove null delimiter

    auto decoded = Utils::cobs_decode(encoded.value());
    ASSERT_NE(decoded, std::nullopt);
    // check that the decoded bytes are equal to the original bytes
    EXPECT_EQ(*decoded, bytes);

    // check that the decoded bytes are equal to the original test object
    EXPECT_EQ(memcmp(&test, decoded->data(), sizeof(TestStruct)), 0);
}

// test that encoding or decoding with an empty vector should return std::nullopt
TEST(CobsTest, EncodeOrDecodeEmpty) {
    EXPECT_EQ(Utils::cobs_encode({}), std::nullopt);
    EXPECT_EQ(Utils::cobs_decode({}), std::nullopt);
}

// test that encoded data should only have 1 null delimiter
TEST(CobsTest, EncodeHasOnlyOneDelimiter) {
    std::vector<uint8_t> bytes(sizeof(lorem) + 1);
    std::memcpy(bytes.data(), lorem, bytes.size());

    auto encoded = Utils::cobs_encode(bytes);
    ASSERT_NE(encoded, std::nullopt);

    // test that there are no nulls before the last byte
    int i;
    for (i = 0; i < encoded->size() - 1; i++) {
        EXPECT_NE(encoded->at(i), 0x00);
    }
    // test that the last byte is a null delimiter
    EXPECT_EQ(encoded->at(i), 0x00);
}
