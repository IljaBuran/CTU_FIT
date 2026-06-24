#ifndef __PROGTEST__
#include <cstdlib>
#include <cstdio>
#include <cctype>
#include <climits>
#include <cstdint>
#include <iostream>
#include <iomanip>
#include <sstream>
#include <unistd.h>
#include <sys/stat.h>
#include <string>
#include <memory>
#include <vector>
#include <fstream>
#include <cassert>
#include <cstring>

#include <openssl/evp.h>
#include <openssl/rand.h>

struct crypto_config
{
    const char * m_crypto_function;
    std::unique_ptr<uint8_t[]> m_key;
    std::unique_ptr<uint8_t[]> m_IV;
    size_t m_key_len;
    size_t m_IV_len;
};

#endif

constexpr size_t CHUNK_TO_PROCESS = 4096;
constexpr size_t HEADER_SIZE = 18;

enum TYPE
{
    ENCRYPT,
    DECRYPT
};

/* initialize filestreams, copy first 18 bytes */
bool Init(std::ifstream& in_, 
          std::ofstream& out_,
          const std::string& inFileName_,
          const std::string& outFileName_,
          const EVP_CIPHER*& cypher_,
          const char*& cypherType_)
{
    in_.open(inFileName_, std::ios::binary);
    out_.open(outFileName_, std::ios::binary);

    /* reads first 18 bytes, and writes them into output file */
    if (!in_ || !out_)
        return false;
    char header[HEADER_SIZE];
    in_.read(header, HEADER_SIZE);
    if (in_.bad())
        return false;
    if (in_.gcount() != HEADER_SIZE)
        return false;
    out_.write(header, HEADER_SIZE);
    if (out_.fail()) return false;

    /* find cypher in library based on name */
    cypher_ = EVP_get_cipherbyname(cypherType_);
    if (!cypher_)
        return false;
    
    return true;
}

int FreeAndExit(EVP_CIPHER_CTX* const context_)
{
    EVP_CIPHER_CTX_free(context_);
    return EXIT_FAILURE;
}

int Finalize(TYPE t,
             std::ifstream& in_,
             std::ofstream& out_,
             const EVP_CIPHER* const cypher_,
             EVP_CIPHER_CTX* const context_)
{
    /* get pointer function whether we want to encrypt/decrypt */
    int (*updateFn)(EVP_CIPHER_CTX*, unsigned char*, int*, const unsigned char*, int) = (t == ENCRYPT ? EVP_EncryptUpdate : EVP_DecryptUpdate);
    int (*finalFn)(EVP_CIPHER_CTX*, unsigned char*, int*) = (t == ENCRYPT ? EVP_EncryptFinal_ex : EVP_DecryptFinal_ex);
    
    /* read from file stream, (en/de)crypt and write into output file */
    std::vector<uint8_t> inBuffer(CHUNK_TO_PROCESS);
    std::vector<uint8_t> outBuffer(CHUNK_TO_PROCESS + EVP_CIPHER_block_size(cypher_));
    size_t bytesRead;
    int len;
    
    while (in_)
    {
        in_.read((char*)inBuffer.data(), CHUNK_TO_PROCESS);
        if (in_.bad()) return FreeAndExit(context_);
        if ((bytesRead = (size_t)(in_.gcount())) > 0)
        {
            if (updateFn(context_, outBuffer.data(), &len, inBuffer.data(), bytesRead) != 1)
            {
                return FreeAndExit(context_);
            }
            out_.write((char*)outBuffer.data(), len);
            if (out_.bad()) return FreeAndExit(context_);
        }
    }
    
    /* finalize encryption */
    if (finalFn(context_, outBuffer.data(), &len) != 1)
    {
        return FreeAndExit(context_);
    }
    
    /* write into file */
    out_.write((char*)(outBuffer.data()), len);
    if (out_.bad())
        return FreeAndExit(context_);
    EVP_CIPHER_CTX_free(context_);
    return EXIT_SUCCESS;
}

namespace encrypt
{
    void CheckKey(crypto_config&    config, 
                  const EVP_CIPHER* cypher)
    {
        size_t keyLen = EVP_CIPHER_key_length(cypher);
        if (config.m_key == nullptr || keyLen > config.m_key_len)
        {
            config.m_key = std::make_unique<uint8_t[]>(keyLen);
            RAND_bytes(config.m_key.get(), keyLen);
            config.m_key_len = keyLen;
        }
    }

    void CheckIv(crypto_config&    config,
                     const EVP_CIPHER* cypher)
    {
        size_t ivLen = EVP_CIPHER_iv_length(cypher);
        if (ivLen != 0)
        {
            if (config.m_IV == nullptr || ivLen > config.m_IV_len)
            {
                config.m_IV = std::make_unique<uint8_t[]>(ivLen);
                RAND_bytes(config.m_IV.get(), ivLen);
                config.m_IV_len = ivLen;
            }
        }
        else
        {
            config.m_IV_len = 0;
        }
    }

    EVP_CIPHER_CTX* StartEncryption(crypto_config&    config,
                                    const EVP_CIPHER* cypher,
                                    const uint8_t*    ivPtr)
    {
        /* initialize encryption context */
        EVP_CIPHER_CTX* context = EVP_CIPHER_CTX_new();
        if (!context)
            return nullptr;
        if (EVP_EncryptInit_ex(context, cypher, nullptr, config.m_key.get(), ivPtr) != 1)
        {
            EVP_CIPHER_CTX_free(context); return nullptr;
        }
        return context;
    }
};
namespace decrypt
{
    size_t CheckKeyIv(crypto_config&    config,
                 const EVP_CIPHER* cypher)
    {
        size_t keyLen = EVP_CIPHER_key_length(cypher);
        size_t ivLen = EVP_CIPHER_iv_length(cypher);
        if ((keyLen > config.m_key_len) || !config.m_key)
            return SIZE_MAX;
        if (ivLen > 0)
            if (config.m_IV_len < ivLen || !config.m_IV)
                return SIZE_MAX;
        return ivLen;
    }

    EVP_CIPHER_CTX* StartDecryption(crypto_config&    config,
                                    const EVP_CIPHER* cypher,
                                    int               ivLen)
    {
        EVP_CIPHER_CTX* context = EVP_CIPHER_CTX_new();
        if (!context)
            return nullptr;
        uint8_t* ivPtr = (ivLen != 0 ? config.m_IV.get() : nullptr);
        if (EVP_DecryptInit_ex(context, cypher, nullptr, config.m_key.get(), ivPtr) != 1)
        {
            EVP_CIPHER_CTX_free(context); return nullptr;
        }
        return context;
    }
};

int encrypt_data(const std::string& in_filename, 
                 const std::string& out_filename, 
                 crypto_config& config)
{
    /* initialize */
    std::ifstream in;
    std::ofstream out;
    const EVP_CIPHER* cypher = nullptr;

    if (!Init(in, out, in_filename, out_filename, cypher, config.m_crypto_function))
        return EXIT_FAILURE;
    
    /* check key length, if short -> reallocates and generates new one */
    encrypt::CheckKey(config, cypher);
    
    /* check initialization vector, if there is one and it's short -> reallocates and generates new one */
    encrypt::CheckIv(config, cypher);

    /* initialize encryption context */
    EVP_CIPHER_CTX* context = encrypt::StartEncryption(config, cypher, 
            (EVP_CIPHER_iv_length(cypher) ? config.m_IV.get() : nullptr));
    if (!context)
        return EXIT_FAILURE;
    
    /* process remaining data and finalize encrpytion */
    return Finalize(TYPE::ENCRYPT, in, out, cypher, context);
}

int decrypt_data(const std::string& in_filename, 
                 const std::string& out_filename, 
                 crypto_config& config)
{
    /* initialize */
    std::ifstream in;
    std::ofstream out;
    const EVP_CIPHER* cypher = nullptr;

    if (!Init(in, out, in_filename, out_filename, cypher, config.m_crypto_function))
        return EXIT_FAILURE;
    
    /* 
        check key length and initialization vector
        if length is not corresponding to cypher -> decryption fails
    */
    size_t ivLen = decrypt::CheckKeyIv(config, cypher);
    if (ivLen == SIZE_MAX) return EXIT_FAILURE;
    
    /* decypher context initialization */
    EVP_CIPHER_CTX* context = decrypt::StartDecryption(config, cypher, ivLen);
    if (!context) return EXIT_FAILURE;
    
    /* process remaining data and finalize decryption */
    return Finalize(TYPE::DECRYPT, in, out, cypher, context);
}


#ifndef __PROGTEST__

bool compare_files(const char* name1, const char* name2)
{
    /* open file in binary mode, compare file sizes */
    std::ifstream f1(name1, std::ios::binary | std::ios::ate);
    std::ifstream f2(name2, std::ios::binary | std::ios::ate);
    if ((!f1 || !f2) || (f1.tellg() != f2.tellg())) 
        return false;
    f1.seekg(0);
    f2.seekg(0);
    
    char buffer1[CHUNK_TO_PROCESS], buffer2[CHUNK_TO_PROCESS];
    
    /* read both files and compare them */
    while (f1 && f2)
    {
        f1.read(buffer1, CHUNK_TO_PROCESS);
        f2.read(buffer2, CHUNK_TO_PROCESS);
        size_t readSize;

        if ((readSize = (size_t)f1.gcount()) != (size_t)f2.gcount() || std::memcmp(buffer1, buffer2, readSize) != 0)
            return false;
    }
    return true;
}

int main ( void )
{
    crypto_config config {nullptr, nullptr, nullptr, 0, 0};
    
    // ECB mode
    config.m_crypto_function = "AES-128-ECB";
    config.m_key = std::make_unique<uint8_t[]>(16);
    memset(config.m_key.get(), 0, 16);
    config.m_key_len = 16;
    
    assert( EXIT_SUCCESS == encrypt_data("tests/homer-simpson.TGA", "tests/out_file.TGA", config) &&
            compare_files("tests/out_file.TGA", "tests/homer-simpson_enc_ecb.TGA") );
    
    assert( EXIT_SUCCESS == decrypt_data("tests/homer-simpson_enc_ecb.TGA", "tests/out_file.TGA", config) &&
            compare_files("tests/out_file.TGA", "tests/homer-simpson.TGA") );
    
    assert( EXIT_SUCCESS == encrypt_data("tests/UCM8.TGA", "tests/out_file.TGA", config) &&
            compare_files("tests/out_file.TGA", "tests/UCM8_enc_ecb.TGA") );
    
    assert( EXIT_SUCCESS == decrypt_data("tests/UCM8_enc_ecb.TGA", "tests/out_file.TGA", config) &&
            compare_files("tests/out_file.TGA", "tests/UCM8.TGA") );
    
    assert( EXIT_SUCCESS == encrypt_data("tests/image_1.TGA", "tests/out_file.TGA", config) &&
            compare_files("tests/out_file.TGA", "tests/ref_1_enc_ecb.TGA") );
    
    assert( EXIT_SUCCESS == encrypt_data("tests/image_2.TGA", "tests/out_file.TGA", config) &&
            compare_files("tests/out_file.TGA", "tests/ref_2_enc_ecb.TGA") );
    
    assert( EXIT_SUCCESS == decrypt_data("tests/image_3_enc_ecb.TGA", "tests/out_file.TGA", config)  &&
            compare_files("tests/out_file.TGA", "tests/ref_3_dec_ecb.TGA") );
    
    assert( EXIT_SUCCESS == decrypt_data("tests/image_4_enc_ecb.TGA", "tests/out_file.TGA", config)  &&
            compare_files("tests/out_file.TGA", "tests/ref_4_dec_ecb.TGA") );
    
    // CBC mode
    config.m_crypto_function = "AES-128-CBC";
    config.m_IV = std::make_unique<uint8_t[]>(16);
    config.m_IV_len = 16;
    memset(config.m_IV.get(), 0, 16);
    
    assert( EXIT_SUCCESS == encrypt_data("tests/UCM8.TGA", "tests/out_file.TGA", config) &&
            compare_files("tests/out_file.TGA", "tests/UCM8_enc_cbc.TGA") );
    
    assert( EXIT_SUCCESS == decrypt_data("tests/UCM8_enc_cbc.TGA", "tests/out_file.TGA", config) &&
            compare_files("tests/out_file.TGA", "tests/UCM8.TGA") );
    
    assert( EXIT_SUCCESS == encrypt_data("tests/homer-simpson.TGA", "tests/out_file.TGA", config) &&
            compare_files("tests/out_file.TGA", "tests/homer-simpson_enc_cbc.TGA") );
    
    assert( EXIT_SUCCESS == decrypt_data("tests/homer-simpson_enc_cbc.TGA", "tests/out_file.TGA", config) &&
            compare_files("tests/out_file.TGA", "tests/homer-simpson.TGA") );
    
    assert( EXIT_SUCCESS == encrypt_data("tests/image_1.TGA", "tests/out_file.TGA", config) &&
            compare_files("tests/out_file.TGA", "tests/ref_5_enc_cbc.TGA") );
    
    assert( EXIT_SUCCESS == encrypt_data("tests/image_2.TGA", "tests/out_file.TGA", config) &&
            compare_files("tests/out_file.TGA", "tests/ref_6_enc_cbc.TGA") );
    
    assert( EXIT_SUCCESS == decrypt_data("tests/image_7_enc_cbc.TGA", "tests/out_file.TGA", config)  &&
            compare_files("tests/out_file.TGA", "tests/ref_7_dec_cbc.TGA") );
    
    assert( EXIT_SUCCESS == decrypt_data("tests/image_8_enc_cbc.TGA", "tests/out_file.TGA", config)  &&
            compare_files("tests/out_file.TGA", "tests/ref_8_dec_cbc.TGA") );
    
    return 0;
}

#endif
