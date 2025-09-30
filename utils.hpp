#pragma once

#include <cstdint>
#include <vector>
#include <optional>


/** Encodes an array of bytes and returns the cobs encoding of it, or nullopt if it fails for any reason. */
std::optional<std::vector<uint8_t>> cobs_encode(const std::vector<uint8_t>& data);

/**
 * Decodes a vector of bytes encoded with cobs into a vector, or nullopt if it fails for any reason.
 *
 * NOTE: The data passed here should not contain the null delimiter at the end.
 */
std::optional<std::vector<uint8_t>> cobs_decode(const std::vector<uint8_t>& data);


/**
 * Computes a one's complement addition. The purpose of this function is to compute the checksum.
 * Using one's complement to mimic IP. IP uses one's complement bc it's irrespective of edianness.
 *
 * https://datatracker.ietf.org/doc/html/rfc1071
 * @param data A pointer to the first byte of the data to compute the checksum over.
 * @param length The length of the data in bytes.
 * @return A 16 bit one's complement addition of the data.
 */
uint16_t compute_ones_sum(const uint8_t* data, size_t length);
