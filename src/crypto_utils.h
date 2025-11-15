#pragma once

#include <openssl/evp.h>
#include <string>
#include <vector>

namespace crypto {

// key file helpers
bool keys_exist(const std::string &priv_path, const std::string &pub_path);
bool generate_rsa_keys(const std::string &priv_path, const std::string &pub_path, int bits = 2048);
EVP_PKEY* load_private_key(const std::string &priv_path);
EVP_PKEY* load_public_key(const std::string &pub_path);

// signing / verification
bool sign_data(EVP_PKEY* priv, const std::vector<unsigned char> &data, std::vector<unsigned char> &out_sig);
bool verify_signature(EVP_PKEY* pub, const std::vector<unsigned char> &data, const std::vector<unsigned char> &sig);

// hashing / hex
std::string to_hex(const unsigned char* data, size_t len);
std::string sha256_hex(const unsigned char* data, size_t len);

// simple logging helper
void log_error(const std::string &msg);

} // namespace crypto
