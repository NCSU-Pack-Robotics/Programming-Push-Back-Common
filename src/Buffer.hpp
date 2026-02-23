#pragma once
#include <deque>
#include <limits>

#include "Packet.hpp"

/** A buffer of packets. Used in case we receive multiple packets before we have a chance to read them. */
class Buffer {
    /** A double ended queue for our packets. The back is the newest values. */
    std::deque<Packet> data;

    /** The max size of the dequeue. Defaults to numeric_limits::max. If add is called when size is at max then values
     * are trimmed off the front. */
    size_t max_size;

    /** Adds a packet to the buffer */
    void add(const Packet& data);

    // Friend SerialHandler so that it can use the private add method
    friend class SerialHandler;

public:
    Buffer() : max_size(std::numeric_limits<size_t>::max()) {};

    /** Sets the maximum size of the dequeue. NOTE: This does not modify the size of the buffer until the next element is added. */
    void set_max_size(size_t size);

    /** Pops and returns the latest packet from the dequeue. This is the item at the END of the queue. Returns nullopt if the dequeue is empty */
    std::optional<Packet> pop_latest();

    /** Returns the size of the dequeue */
    [[nodiscard]] size_t size() const;
};
