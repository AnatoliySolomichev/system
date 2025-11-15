#include "crypto_utils.h"

#include <openssl/pem.h>
#include <openssl/err.h>
#include <openssl/rand.h>
#include <fstream>
#include <iostream>
#include <sstream>
#include <iomanip>

namespace crypto {

void log_error(const std::string &msg) {
    std::cerr << "[crypto] " << msg << "\n";
}

bool keys_exist(const std::string &priv_path, const std::string &pub_path) {
    std::ifstream p(priv_path);
    std::ifstream q(pub_path);
    return p.good() && q.good();
}

bool generate_rsa_keys(const std::string &priv_path, const std::string &pub_path, int bits) {
    OpenSSL_add_all_algorithms();
    ERR_load_crypto_strings();

    EVP_PKEY *pkey = nullptr;
    EVP_PKEY_CTX *ctx = EVP_PKEY_CTX_new_id(EVP_PKEY_RSA, nullptr);
    if (!ctx) {
        log_error("EVP_PKEY_CTX_new_id failed");
        return false;
    }
    if (EVP_PKEY_keygen_init(ctx) <= 0) {
        log_error("EVP_PKEY_keygen_init failed");
        EVP_PKEY_CTX_free(ctx);
        return false;
    }

    unsigned int key_size = (unsigned int)bits;
    unsigned int pub_exp = 65537;
    OSSL_PARAM params[3];
    params[0] = OSSL_PARAM_construct_uint("bits", &key_size);
    params[1] = OSSL_PARAM_construct_uint("e", &pub_exp);
    params[2] = OSSL_PARAM_construct_end();
    if (EVP_PKEY_CTX_set_params(ctx, params) <= 0) {
        log_error("EVP_PKEY_CTX_set_params failed");
        EVP_PKEY_CTX_free(ctx);
        return false;
    }
    if (EVP_PKEY_keygen(ctx, &pkey) <= 0) {
        log_error("EVP_PKEY_keygen failed");
        EVP_PKEY_CTX_free(ctx);
        return false;
    }
    EVP_PKEY_CTX_free(ctx);

    FILE *fp = fopen(priv_path.c_str(), "wb");
    if (!fp) { log_error("opening private key file"); EVP_PKEY_free(pkey); return false; }
    if (PEM_write_PrivateKey(fp, pkey, nullptr, nullptr, 0, nullptr, nullptr) != 1) {
        log_error("writing private key"); fclose(fp); EVP_PKEY_free(pkey); return false;
    }
    fclose(fp);

    fp = fopen(pub_path.c_str(), "wb");
    if (!fp) { log_error("opening public key file"); EVP_PKEY_free(pkey); return false; }
    if (PEM_write_PUBKEY(fp, pkey) != 1) {
        log_error("writing public key"); fclose(fp); EVP_PKEY_free(pkey); return false;
    }
    fclose(fp);

    EVP_PKEY_free(pkey);
    return true;
}

EVP_PKEY* load_private_key(const std::string &priv_path) {
    FILE *fp = fopen(priv_path.c_str(), "rb");
    if (!fp) { log_error("open private key file failed: " + priv_path); return nullptr; }
    EVP_PKEY *p = PEM_read_PrivateKey(fp, nullptr, nullptr, nullptr);
    fclose(fp);
    if (!p) log_error("PEM_read_PrivateKey failed");
    return p;
}

EVP_PKEY* load_public_key(const std::string &pub_path) {
    FILE *fp = fopen(pub_path.c_str(), "rb");
    if (!fp) { log_error("open public key file failed: " + pub_path); return nullptr; }
    EVP_PKEY *p = PEM_read_PUBKEY(fp, nullptr, nullptr, nullptr);
    fclose(fp);
    if (!p) log_error("PEM_read_PUBKEY failed");
    return p;
}

bool sign_data(EVP_PKEY* priv, const std::vector<unsigned char> &data, std::vector<unsigned char> &out_sig) {
    if (!priv) { log_error("sign_data: priv is null"); return false; }
    EVP_MD_CTX *md = EVP_MD_CTX_new();
    if (!md) { log_error("EVP_MD_CTX_new failed"); return false; }
    if (1 != EVP_DigestSignInit(md, nullptr, EVP_sha256(), nullptr, priv)) { log_error("DigestSignInit failed"); EVP_MD_CTX_free(md); return false; }
    if (1 != EVP_DigestSignUpdate(md, data.data(), data.size())) { log_error("DigestSignUpdate failed"); EVP_MD_CTX_free(md); return false; }
    size_t req = 0;
    if (1 != EVP_DigestSignFinal(md, nullptr, &req)) { log_error("DigestSignFinal (size) failed"); EVP_MD_CTX_free(md); return false; }
    out_sig.resize(req);
    if (1 != EVP_DigestSignFinal(md, out_sig.data(), &req)) { log_error("DigestSignFinal (data) failed"); EVP_MD_CTX_free(md); return false; }
    out_sig.resize(req);
    EVP_MD_CTX_free(md);
    return true;
}

bool verify_signature(EVP_PKEY* pub, const std::vector<unsigned char> &data, const std::vector<unsigned char> &sig) {
    if (!pub) { log_error("verify_signature: pub is null"); return false; }
    EVP_MD_CTX *md = EVP_MD_CTX_new();
    if (!md) { log_error("EVP_MD_CTX_new failed"); return false; }
    if (1 != EVP_DigestVerifyInit(md, nullptr, EVP_sha256(), nullptr, pub)) { log_error("DigestVerifyInit failed"); EVP_MD_CTX_free(md); return false; }
    if (1 != EVP_DigestVerifyUpdate(md, data.data(), data.size())) { log_error("DigestVerifyUpdate failed"); EVP_MD_CTX_free(md); return false; }
    int rv = EVP_DigestVerifyFinal(md, sig.data(), sig.size());
    EVP_MD_CTX_free(md);
    if (rv == 1) return true;
    if (rv == 0) return false;
    log_error(std::string("DigestVerifyFinal error: ") + ERR_error_string(ERR_get_error(), nullptr));
    return false;
}

std::string to_hex(const unsigned char* data, size_t len) {
    std::ostringstream ss;
    ss << std::hex << std::setfill('0');
    for (size_t i = 0; i < len; ++i) ss << std::setw(2) << (int)data[i];
    return ss.str();
}

std::string sha256_hex(const unsigned char* data, size_t len) {
    unsigned char md[EVP_MAX_MD_SIZE];
    unsigned int md_len = 0;
    EVP_MD_CTX *mdctx = EVP_MD_CTX_new();
    if (!mdctx) { log_error("EVP_MD_CTX_new failed"); return std::string(); }
    EVP_DigestInit_ex(mdctx, EVP_sha256(), nullptr);
    EVP_DigestUpdate(mdctx, data, len);
    EVP_DigestFinal_ex(mdctx, md, &md_len);
    EVP_MD_CTX_free(mdctx);
    return to_hex(md, md_len);
}

} // namespace crypto
