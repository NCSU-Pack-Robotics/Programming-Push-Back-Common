#include "Utils.hpp"

std::optional<std::vector<uint8_t>> Utils::cobs_encode(const std::vector<uint8_t>& data) {
    if (data.empty()) return std::nullopt;

    std::vector<uint8_t> output;
    output.resize(data.size() + data.size() / 254 + 2);
    /* This is the maximum size. Encoded data will be the originals size, include a start and end byte, and have +1
     * byte each time it goes over 254 without a zero in between */

    int marker_index = 0; // Stores the index of where the marker will go
    int output_index = 1;
    // The current index to write normal bytes to, we skip 0 because a marker will go there
    for (const uint8_t byte : data) {
        // If there is a zero byte, then instead set the marker to the distance from the marker to the zero
        // The byte at this position won't be written until we find another zero, we go over 0xFF, or after the loop ends
        if (byte == 0x00) {
            output[marker_index] = output_index - marker_index;
            marker_index = output_index++;
        }
        // A byte can store a max of 255. Instead of using 255 to represent that there is a 0 in 255 spaces, we use 255 to
        // represent that there is not a 0 here or for the next 254 spots, and at the 255th spot there will be another marker
        else if (output_index - marker_index > 254) {
            output[marker_index] = 0xFF;
            marker_index = output_index++;
            output[output_index++] = byte;
        }
        else {
            output[output_index++] = byte;
        }
    }
    output[marker_index] = output_index - marker_index;
    output[output_index++] = 0x00;

    output.resize(output_index);

    return output;
}

std::optional<std::vector<uint8_t>> Utils::cobs_decode(const std::vector<uint8_t>& data) {
    // Encoded data must have at east 2 elements.
    // If it has only 1, that elements says where the next zero is, but that marker is not supported to be 0,
    // so it must be 1 but there are no other elements in the array
    // This depends on cobs implementation, but follows what our cobs_encode method produces
    if (data.size() <= 1) return std::nullopt;
    // The start marker cannot be greater than the size. If it is equal to the size, that means the '0' is after the end of the
    // array, so the entire array is preserved. Anything more than that is not allowed.
    if (data[0] > data.size()) return std::nullopt;
    // Encoded cobs bytes cannot contain zeros
    if (data[0] == 0) return std::nullopt;

    std::vector<uint8_t> output(data.size() - 1);

    int output_index = 0;
    int next_marker_index = data[0]; // get the next marker, which is always the first element
    bool was_block_marker = (data[0] == 0xFF);
    // Block markers are special because they don't have a zero at the position they mark

    // Note: Jumping directly to marker indexes and memcpying the data ranges into output would be
    // more efficient, but this does not matter for our usage for now, especially since most
    // of our packets are very small and full of zeros

    for (int i = 1; i < data.size(); i++) {
        if (data[i] == 0) return std::nullopt; // cobs encoded bytes cannot contains zeros
        if (i == next_marker_index) {
            if (!was_block_marker) {
                output[output_index++] = 0x00;
            }

            next_marker_index = i + data[i];
            // same reason we return early if data[0] is > data.size()
            if (next_marker_index > data.size()) return std::nullopt;
            was_block_marker = (data[i] == 0xFF);
        }
        else {
            output[output_index++] = data[i];
        }
    }
    output.resize(output_index);

    return output;
}
