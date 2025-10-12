#pragma once
#include <deque>

#include "packet/Packet.hpp"

/** A buffer of packets. Used in case we receive multiple packets before we have a chance to read them. */
class Buffer {
    /** A double ended queue for our packets. The back is the newest values. */
    std::deque<Packet> data;

    /** The max size of the dequeue. Defaults to -1 which means no max size. If add is called when size is at max then values
     * are trimmed off the front. */
    size_t max_size;

    /** Adds a packet to the buffer */
    void add(const Packet& data);

    friend class SerialHandler; // Friend SerialHandler so that it can use the private add method

public:
    Buffer() : max_size(-1) {};

    /** Sets the maximum size of the dequeue */
    void set_max_size(size_t size);

    /** Pops and returns the latest packet from the dequeue. Returns nullopt if the dequeue is empty */
    std::optional<Packet> pop_latest();

    /** Returns the size of the dequeue */
    size_t size() const;
};
