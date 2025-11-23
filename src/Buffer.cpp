#include "Buffer.hpp"

size_t Buffer::size() const {
    return this->data.size();
}

std::optional<Packet> Buffer::pop_latest() {
    if (this->data.empty())
        return std::nullopt;

    Packet packet = this->data.back();
    this->data.pop_back();
    return std::move(packet);
}

void Buffer::add(const Packet& data) {
    if (max_size > 0 && this->data.size() > max_size) {
        this->data.erase(this->data.begin(), this->data.begin() + (this->max_size - this->data.size()));
    }

    this->data.push_back(data);
}

void Buffer::set_max_size(size_t size) {
    this->max_size = size;
}
