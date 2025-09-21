#include <cstdint>
#include <vector>

std::vector<uint8_t> cobs_encode(const std::vector<uint8_t>& data)
{
    std::vector<uint8_t> output;
    output.resize(data.size() + data.size() / 254 + 2); // This is the maximum size. Encoded data will be the originals size, include a start and end byte, and have +1 byte each time it goes over 254 without a zero in between

    int marker_index = 0; // Stores the index of where the marker will go
    int output_index = 1; // The current index to write normal bytes to, we skip 0 because a marker will go there
    for (uint8_t byte : data)
    {
        // If there is a zero byte, then instead set the marker to the distance from the marker to the zero
        // The byte at this position won't be written until we find another zero, we go over 0xFF, or after the loop ends
        if (byte == 0x00)
        {
            output[marker_index] = output_index - marker_index;
            marker_index = output_index++;
        }
        // A byte can store a max of 255. Instead of using 255 to represent that there is a 0 in 255 spaces, we use 255 to
        // represent that there is not a 0 here or for the next 254 spots, and at the 255th spot there will be another marker
        else if (output_index - marker_index > 254)
        {
            output[marker_index] = 0xFF;
            marker_index = output_index++;
            output[output_index++] = byte;
        }
        else
        {
            output[output_index++] = byte;
        }
    }
    output[marker_index] = output_index - marker_index;
    output[output_index++] = 0x00;

    output.resize(output_index);

    return output;
}

std::vector<uint8_t> cobs_decode(const std::vector<uint8_t>& data)
{
    std::vector<uint8_t> output;
    output.resize(data.size() - 2);

    int output_index = 0;
    int next_marker_index = data[0];
    bool was_block_marker = (data[0] == 0xFF); // Block markers are special because they don't have a zero at the position they mark

    for (int i = 1; i < data.size(); i++)
    {
        if (i == next_marker_index)
        {
            if (!was_block_marker)
            {
                output[output_index++] = 0x00;
            }

            next_marker_index = i + data[i];
            was_block_marker = (data[i] == 0xFF);
        }
        else
        {
            output[output_index++] = data[i];
        }
    }
    output.resize(output_index);

    return output;
}
