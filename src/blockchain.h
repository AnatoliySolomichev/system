#pragma once

#include "blocks.h"
#include <vector>
#include <string>
#include <openssl/evp.h>

namespace bc {

// Append block in deterministic on-disk format (big-endian ints, previous_hash as 64 ASCII hex bytes)
bool append_block_file(const sign_block_t &sb, const std::string &filename);
// Load chain from file (allocates block_data.data and sign.sign using malloc)
bool load_chain_file(const std::string &filename, std::vector<sign_block_t> &out_chain);
// Free allocations from load_chain_file
void free_chain(std::vector<sign_block_t> &chain);

// Serialize block_data in canonical big-endian form
void serialize_blockdata_be(const block_data_t &bd, std::vector<unsigned char> &out);

// Compute SHA-256 hex of (serialized block_data BE || signature bytes)
std::string compute_block_hash_hex(const block_data_t &bd, const sign_t &sg);

// Verify chain: check signatures (using pub key) and previous_hash linking
bool verify_chain(const std::vector<sign_block_t> &chain, EVP_PKEY* pub);

} // namespace bc
