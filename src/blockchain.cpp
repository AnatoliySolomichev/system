#include "blockchain.h"
#include "crypto_utils.h"

#include <fstream>
#include <cstring>
#include <arpa/inet.h>
#include <iostream>

namespace bc {

static void write_be32(std::vector<unsigned char> &out, uint32_t v) {
    out.push_back((v >> 24) & 0xFF);
    out.push_back((v >> 16) & 0xFF);
    out.push_back((v >> 8) & 0xFF);
    out.push_back(v & 0xFF);
}

static uint32_t read_be32(std::ifstream &ifs) {
    unsigned char b[4];
    ifs.read(reinterpret_cast<char*>(b), 4);
    return ((uint32_t)b[0] << 24) | ((uint32_t)b[1] << 16) | ((uint32_t)b[2] << 8) | (uint32_t)b[3];
}

void serialize_blockdata_be(const block_data_t &bd, std::vector<unsigned char> &out) {
    out.clear();
    write_be32(out, (uint32_t)bd.number);
    write_be32(out, (uint32_t)bd.time);
    // previous_hash expected to be 64 ASCII hex bytes
    out.insert(out.end(), reinterpret_cast<const unsigned char*>(bd.previous_hash), reinterpret_cast<const unsigned char*>(bd.previous_hash) + HASH_LEN);
    write_be32(out, (uint32_t)bd.data_len);
    if (bd.data && bd.data_len > 0) out.insert(out.end(), reinterpret_cast<const unsigned char*>(bd.data), reinterpret_cast<const unsigned char*>(bd.data) + bd.data_len);
}

std::string compute_block_hash_hex(const block_data_t &bd, const sign_t &sg) {
    std::vector<unsigned char> ser;
    serialize_blockdata_be(bd, ser);
    if (sg.sign && sg.sign_len > 0) ser.insert(ser.end(), reinterpret_cast<const unsigned char*>(sg.sign), reinterpret_cast<const unsigned char*>(sg.sign) + sg.sign_len);
    return crypto::sha256_hex(ser.data(), ser.size());
}

bool append_block_file(const sign_block_t &sb, const std::string &filename) {
    std::ofstream ofs(filename, std::ios::binary | std::ios::app);
    if (!ofs) return false;
    // number, time
    auto write32 = [&](uint32_t v) {
        unsigned char b[4];
        b[0] = (v >> 24) & 0xFF;
        b[1] = (v >> 16) & 0xFF;
        b[2] = (v >> 8) & 0xFF;
        b[3] = v & 0xFF;
        ofs.write(reinterpret_cast<char*>(b), 4);
    };
    write32((uint32_t)sb.block_data.number);
    write32((uint32_t)sb.block_data.time);
    // previous_hash (64 bytes)
    ofs.write(sb.block_data.previous_hash, HASH_LEN);
    write32((uint32_t)sb.block_data.data_len);
    if (sb.block_data.data && sb.block_data.data_len > 0) ofs.write(sb.block_data.data, sb.block_data.data_len);
    write32((uint32_t)sb.sign.sign_len);
    if (sb.sign.sign && sb.sign.sign_len > 0) ofs.write(sb.sign.sign, sb.sign.sign_len);
    ofs.close();
    return true;
}

bool load_chain_file(const std::string &filename, std::vector<sign_block_t> &out_chain) {
    std::ifstream ifs(filename, std::ios::binary);
    if (!ifs) return false;
    out_chain.clear();
    while (ifs.peek() != EOF) {
        sign_block_t sb;
        uint32_t num = read_be32(ifs);
        uint32_t tm = read_be32(ifs);
        sb.block_data.number = (int)num;
        sb.block_data.time = (int)tm;
        // previous_hash
        ifs.read(sb.block_data.previous_hash, HASH_LEN);
        uint32_t data_len = read_be32(ifs);
        sb.block_data.data_len = (int)data_len;
        if (data_len > 0) {
            sb.block_data.data = (char*)malloc(data_len);
            ifs.read(sb.block_data.data, data_len);
        } else sb.block_data.data = nullptr;
        uint32_t sign_len = read_be32(ifs);
        sb.sign.sign_len = sign_len;
        if (sign_len > 0) {
            sb.sign.sign = (char*)malloc(sign_len);
            ifs.read(sb.sign.sign, sign_len);
        } else sb.sign.sign = nullptr;
        out_chain.push_back(sb);
    }
    return true;
}

void free_chain(std::vector<sign_block_t> &chain) {
    for (auto &sb : chain) {
        if (sb.block_data.data) free(sb.block_data.data);
        if (sb.sign.sign) free(sb.sign.sign);
    }
    chain.clear();
}

bool verify_chain(const std::vector<sign_block_t> &chain, EVP_PKEY* pub) {
    if (!pub) return false;
    bool ok = true;
    for (size_t i = 0; i < chain.size(); ++i) {
        const sign_block_t &sb = chain[i];
        std::vector<unsigned char> ser;
        serialize_blockdata_be(sb.block_data, ser);
        std::vector<unsigned char> sig;
        if (sb.sign.sign && sb.sign.sign_len > 0) sig.assign(reinterpret_cast<const unsigned char*>(sb.sign.sign), reinterpret_cast<const unsigned char*>(sb.sign.sign) + sb.sign.sign_len);
        else sig.clear();
        if (!crypto::verify_signature(pub, ser, sig)) {
            std::cerr << "verify_chain: signature invalid for block " << i << "\n";
            ok = false;
            break;
        }
        if (i > 0) {
            std::string expected_prev = compute_block_hash_hex(chain[i-1].block_data, chain[i-1].sign);
            if (memcmp(chain[i].block_data.previous_hash, expected_prev.c_str(), HASH_LEN) != 0) {
                std::cerr << "verify_chain: previous hash mismatch at block " << i << "\n";
                ok = false;
                break;
            }
        } else {
            bool allzero = true;
            for (int k = 0; k < HASH_LEN; ++k) if (chain[i].block_data.previous_hash[k] != 0) { allzero = false; break; }
            if (!allzero) { std::cerr << "verify_chain: genesis previous hash not zero\n"; ok = false; break; }
        }
    }
    return ok;
}

} // namespace bc
