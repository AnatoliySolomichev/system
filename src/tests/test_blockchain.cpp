#include "../crypto_utils.h"
#include "../blockchain.h"
#include "../blocks.h"

#include <iostream>
#include <vector>
#include <cstdio>
#include <cstring>

int main() {
    const std::string priv = "private_key.pem";
    const std::string pub = "public_key.pem";
    const std::string chainfile = "chain.dat";

    // ensure keys
    if (!crypto::keys_exist(priv, pub)) {
        std::cout << "Generating RSA keys...\n";
        if (!crypto::generate_rsa_keys(priv, pub, 2048)) {
            std::cerr << "Failed to generate keys\n";
            return 2;
        }
    }

    // remove existing chain file
    std::remove(chainfile.c_str());

    // create and append 3 blocks
    EVP_PKEY *privk = crypto::load_private_key(priv);
    if (!privk) { std::cerr << "load private failed\n"; return 2; }

    std::vector<std::string> payloads = {"Alpha", "Beta", "Gamma"};
    std::vector<sign_block_t> created;
    for (size_t i = 0; i < payloads.size(); ++i) {
        sign_block_t sb{};
        sb.block_data.number = (int)i;
        sb.block_data.time = (int)time(nullptr);
        if (i == 0) memset(sb.block_data.previous_hash, 0, HASH_LEN);
        else {
            std::string prev_hex = bc::compute_block_hash_hex(created.back().block_data, created.back().sign);
            memcpy(sb.block_data.previous_hash, prev_hex.c_str(), HASH_LEN);
        }
        sb.block_data.data_len = (int)payloads[i].size();
        sb.block_data.data = (char*)malloc(sb.block_data.data_len);
        memcpy(sb.block_data.data, payloads[i].data(), sb.block_data.data_len);

        std::vector<unsigned char> ser;
        bc::serialize_blockdata_be(sb.block_data, ser);
        std::vector<unsigned char> sig;
        if (!crypto::sign_data(privk, ser, sig)) { std::cerr << "sign failed\n"; return 2; }
        sb.sign.sign_len = sig.size();
        sb.sign.sign = (char*)malloc(sb.sign.sign_len);
        memcpy(sb.sign.sign, sig.data(), sb.sign.sign_len);

        if (!bc::append_block_file(sb, chainfile)) { std::cerr << "append failed\n"; return 2; }
        created.push_back(sb);
    }

    EVP_PKEY_free(privk);

    // load and verify
    std::vector<sign_block_t> loaded;
    if (!bc::load_chain_file(chainfile, loaded)) { std::cerr << "load_chain_file failed\n"; return 2; }

    EVP_PKEY *pubk = crypto::load_public_key(pub);
    if (!pubk) { std::cerr << "load public failed\n"; return 2; }

    bool ok = bc::verify_chain(loaded, pubk);
    std::cout << (ok ? "Chain verification OK" : "Chain verification FAILED") << "\n";

    // Tamper with second block (in-memory) and verify should fail
    if (loaded.size() >= 2 && loaded[1].block_data.data_len > 0) {
        loaded[1].block_data.data[0] ^= 0xFF; // flip first byte
        bool ok2 = bc::verify_chain(loaded, pubk);
        std::cout << "After tampering: " << (ok2 ? "OK (unexpected)" : "FAILED (expected)") << "\n";
    }

    EVP_PKEY_free(pubk);
    bc::free_chain(loaded);
    // free created chain allocations
    for (auto &s: created) { if (s.block_data.data) free(s.block_data.data); if (s.sign.sign) free(s.sign.sign); }

    return ok ? 0 : 1;
}
