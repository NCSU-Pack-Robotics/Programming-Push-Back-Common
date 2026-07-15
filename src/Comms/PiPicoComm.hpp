#pragma once
#include "AbstractComm.hpp"

/** Communication abstraction for the Raspberry Pi Pico 2 - rp2350A */
class PiPicoComm : public AbstractComm {
public:
    PiPicoComm() = default;
    size_t read(unsigned char* buf, size_t count) override;
    void write(unsigned char* buf, size_t count) override;

    void mutex_lock() override;
    void mutex_unlock() override;
};
