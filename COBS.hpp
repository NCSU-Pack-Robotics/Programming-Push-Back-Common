#pragma once

#include <cstdint>
#include <optional>
#include <vector>

/** Encodes an array of bytes and returns the cobs encoding of it, or nullopt if it fails for any reason. */
std::optional<std::vector<uint8_t>> cobs_encode(const std::vector<uint8_t>& data);

/**
 * Decodes a vector of bytes encoded with cobs into a vector, or nullopt if it fails for any reason.
 *
 * NOTE: The data passed here should not contain the null delimiter at the end.
 */
std::optional<std::vector<uint8_t>> cobs_decode(const std::vector<uint8_t>& data);
