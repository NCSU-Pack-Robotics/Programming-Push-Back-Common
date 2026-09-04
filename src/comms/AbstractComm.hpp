#pragma once

#include <cstddef>

/**
 * Used as an interface for communication. Every device our code compiles on has different ways of
 * sending data different ways of handling concurrency. Therefore, this "interface" is intended to
 * abstract some of the platform-specific dependencies.
 */
class AbstractComm {
public:
    /** Reads data from somewhere into a buffer.
     * This method is responsible for handling all errors related to reading.
     * @param buf A pointer to where the data should be read to
     * @param count The maximum amount of bytes to read.
     * If count is zero this method should return 0 and perform no operation.
     * @returns The number of bytes read. */
    virtual size_t read(unsigned char* buf, size_t count) = 0;

    /** Writes data from a buffer to somewhere.
     * This method is responsible for handling all errors related to writing.
     * @param buf A pointer to the data to write.
     * @param count The amount of data to write from the buffer. */
    virtual void write(unsigned char* buf, size_t count) = 0;

    virtual ~AbstractComm() = default;
};
