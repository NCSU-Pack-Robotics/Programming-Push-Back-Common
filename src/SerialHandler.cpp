#include "SerialHandler.hpp"

#include <cassert>

// TODO: To be safe, sent packets should begin with a null byte to end the previous data, in the case tha theres unknown
// data
// before it. It also couldn't hurt to add a small set of signature bytes to prefix a packet, to prevent junk data
// having the chance to be a valid packet id and pollute the buffers
void SerialHandler::send(const Packet& packet) {
    std::vector<uint8_t> data_to_send = packet.serialize();

    assert(data_to_send.size() <= MAX_PACKET_SIZE && "Cannot send a packet with size greater than max packet size!");

    // Frame the packet using COBS
    std::optional<std::vector<uint8_t>> encoded = Utils::cobs_encode(data_to_send);
    if (!encoded.has_value())
        return;

    // Write the data to the serial connection
    comm->write(encoded->data(), encoded->size());
}

void SerialHandler::receive() {
    // Get a pointer to the first null byte in the buffer
    auto it = std::ranges::find(this->buffer, '\0');
    while (it == std::ranges::end(this->buffer) // If a null byte is not found in the buffer
        || it - this->buffer >= this->next_write_index) { // If the null byte that gets found is past the amount of data we actually read

        size_t num_read = comm->read(this->buffer + this->next_write_index, MAX_LIBUSB_PACKET_SIZE);

        // Since we have to read 512 bytes each libusb call, we need to make sure there is always 512 bytes available in the buffer
        this->next_write_index += num_read;
        if (this->next_write_index >= MAX_ENCODED_PACKET_SIZE) {
            this->next_write_index = 0;
            continue;
        }

        it = std::ranges::find(this->buffer, '\0');
    }

    printf("Decoding packet\n");
    this->decode_packet(it);
}

void SerialHandler::decode_packet(const unsigned char* packet_end) {
    const int packet_length = packet_end - this->buffer; // length not including the null delimiter
    std::vector<uint8_t> bytes(packet_length);

    // Copy the bytes into the bytes vector, excluding the null delimiter
    memcpy(bytes.data(), this->buffer, packet_length);

    // In case we read multiple packets in 1 libusb packet, move the data between the end of our current packet, and the total bytes read, to the beginning of the buffer
    memmove(this->buffer, this->buffer + packet_length + 1, this->next_write_index - packet_length);
    this->next_write_index -= (packet_length + 1);

    const std::optional<std::vector<uint8_t>> decoded = Utils::cobs_decode(bytes);
    if (!decoded.has_value()) return; // If we fail to decode, ignore the packet

    // Decode the header
    Header received_header{};
    memcpy(&received_header, decoded->data(), sizeof(received_header));
    const uint8_t* ptr = decoded->data();
    Packet received_packet{received_header, ptr + sizeof(received_header),
                                 decoded->size() - sizeof(received_header)};

    // if the packet id does not exist, discard the packet
    if (received_packet.get_id() >= PacketIds::LENGTH) return;

    comm->mutex_lock();
    // get the function before while locked
    const auto& fn = this->listeners[received_header.packet_id];
    this->buffers[received_header.packet_id].add(received_packet);
    comm->mutex_unlock();

    // call the function while NOT locked, so a user doesn't call a method like pop_latest which requires a lock and causes a deadlock
    if (fn) { // test if function is valid
        fn(received_packet);
    }
}
