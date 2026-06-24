#ifndef __PROGTEST__
#include <assert.h>
#include <ctype.h>
#include <limits.h>
#include <math.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <algorithm>
#include <iomanip>
#include <iostream>
#include <string>
#include <string_view>
#include <vector>
#include <memory>

#include <openssl/evp.h>
#include <openssl/rand.h>

using namespace std;

#endif /* __PROGTEST__ */

int checkHash(const uint8_t                     prefix[], 
              const unsigned int                prefixBitLen, 
              const std::unique_ptr<uint8_t[]>& outputHash) 
{
    if (!prefixBitLen)
        return EXIT_SUCCESS;

    /* get no. of whole bytes and reminding bits */
    unsigned fullBytes = prefixBitLen / 8;
    unsigned remBits = prefixBitLen % 8;

    /* compare bytes */
    for (unsigned i = 0; i < fullBytes; i++)
        if (outputHash[i] != prefix[i])
            return EXIT_FAILURE;
    
    /* if any reminding bits, then compare them using mask */
    if (remBits)
    {
        uint8_t mask = 0xFF << (8 - remBits);
        if ((outputHash[fullBytes] & mask) != (prefix[fullBytes] & mask))
            return EXIT_FAILURE;
    }

    return EXIT_SUCCESS;
}

inline bool CheckInput(const unsigned prefixBitLen,
                       const uint8_t* prefix,
                       const char*    hashName)
{
    return (hashName != nullptr) && ((prefixBitLen == 0) || (prefix != nullptr));
}

inline void ConvertToBigEndian(uint64_t                     counter, 
                               std::unique_ptr<uint8_t []>& buffer, 
                               size_t                       outputMessageLen) 
{
    for (size_t i = 0; i < outputMessageLen; i++) 
    {
        const size_t shift = 8 * (outputMessageLen - 1 - i);
        buffer[i] = (uint8_t)((counter >> shift) & 0xFF);
    }
}

bool Fail_FreeContext(EVP_MD_CTX* context)
{
    EVP_MD_CTX_free(context);
    return false;
}

bool GetHash(const EVP_MD*                     pHashAlgo,
             const std::unique_ptr<uint8_t[]>& msg,
             std::unique_ptr<uint8_t[]>&       hash,
             size_t                            outputMessageLen)
{
    /* create context */
    EVP_MD_CTX* context = EVP_MD_CTX_new();
    if (!context) return false;
    
    /* initialize digest algorithm */
    if (EVP_DigestInit_ex(context, pHashAlgo, nullptr) != 1)
        return Fail_FreeContext(context);
    
    /* add msg to digest if exists */
    if (outputMessageLen != 0)
        if (EVP_DigestUpdate(context, msg.get(), outputMessageLen) != 1)
            return Fail_FreeContext(context);
    
    /* finalize and get the digest */
    if (EVP_DigestFinal_ex(context, hash.get(), nullptr) != 1)
        return Fail_FreeContext(context);
    
    EVP_MD_CTX_free(context);
    return true;
}

int findHash(const uint8_t*              prefix, 
             const unsigned int          prefixBitLen, 
             std::unique_ptr<uint8_t[]>& outputMessage, 
             size_t&                     outputMessageLen, 
             std::unique_ptr<uint8_t[]>& outputHash, 
             size_t&                     outputHashLen, 
             const char*                 hashName = "sha256")
{
    /* local variables */
    size_t outputMessageLen_, outputHashLen_;
    std::unique_ptr<uint8_t[]> outputMessage_, outputHash_;
    
    /* check input validity */
    if (!CheckInput(prefixBitLen, prefix, hashName)) return EXIT_FAILURE;

    /* get pointer to function based on hashName parameter */
    const EVP_MD* pHashAlgo = EVP_get_digestbyname(hashName);
    if (!pHashAlgo) return EXIT_FAILURE;
    
    /* get hash length */
    outputHashLen_ = EVP_MD_size(pHashAlgo);
    if (prefixBitLen > outputHashLen_ * 8) return EXIT_FAILURE;

    /* if prefixBitLen == 0, returns hash of empty msg */
    if (!prefixBitLen)
    {
        outputMessageLen_ = 0;
        outputMessage_ = std::make_unique<uint8_t[]>(1);
        auto tempHash = std::make_unique<uint8_t[]>(outputHashLen_);
        if (!GetHash(pHashAlgo, nullptr, tempHash, outputMessageLen_))
            return EXIT_FAILURE;
        outputMessage = std::move(outputMessage_);
        outputHash = std::move(tempHash);
        outputHashLen = outputHashLen_;
        return EXIT_SUCCESS;
    }

    /* set 8 byte msg length */
    outputMessageLen_ = sizeof(uint64_t);
    auto tempMsg = std::make_unique<uint8_t[]>(outputMessageLen_);
    auto tempHash = std::make_unique<uint8_t[]>(outputHashLen_);

    size_t counter = 0;
    while (true)
    {
        /* convert counter to big-endian array */
        ConvertToBigEndian(counter, tempMsg, outputMessageLen_);

        /* calculate hash for msg */
        if (!GetHash(pHashAlgo, tempMsg, tempHash, outputMessageLen_))
            return EXIT_FAILURE;
        
        /* check if hash requirement is satisfied */
        if (checkHash(prefix, prefixBitLen, tempHash) == EXIT_SUCCESS)
        {
            /* if valid, store the msg and hash into local variables */
            outputMessage_ = std::make_unique<uint8_t[]>(outputMessageLen_);
            memcpy(outputMessage_.get(), tempMsg.get(), outputMessageLen_);
            outputHash_ = std::move(tempHash);
            break;
        }
        /* if invalid, increment and try again */
        counter++;
    }
    
    /* success -> write into output parameters */
    outputMessage = std::move(outputMessage_);
    outputMessageLen = outputMessageLen_;
    outputHash = std::move(outputHash_);
    outputHashLen = outputHashLen_;
    return EXIT_SUCCESS;
}

#ifndef __PROGTEST__

int main (void) {
    size_t outputMessageLen, outputHashLen;
    // BASIC TEST
    {
        std::unique_ptr<uint8_t[]> outputMessage, outputHash;
        assert(EXIT_SUCCESS == findHash(nullptr, 0, outputMessage, outputMessageLen, outputHash, outputHashLen));
        assert(outputHashLen == 32);
        //Check if the outputHash is correct for the outputMessage.
        //For instance, in CMD: "echo abcdef | xxd -r -ps | openssl sha256", where "abcdef" is a hexadecimal message
    }

    {
        std::unique_ptr<uint8_t[]> outputMessage, outputHash;
        uint8_t prefix[] = {0x12};
        assert(EXIT_SUCCESS == findHash(prefix, 8, outputMessage, outputMessageLen, outputHash, outputHashLen));
        assert(EXIT_SUCCESS == checkHash(prefix, 8, outputHash));
    }

    {
        std::unique_ptr<uint8_t[]> outputMessage, outputHash;
        uint8_t prefix[] = {0x12, 0x30};
        assert(EXIT_SUCCESS == findHash(prefix, 12, outputMessage, outputMessageLen, outputHash, outputHashLen));
        assert(EXIT_SUCCESS == checkHash(prefix, 12, outputHash));
    }

    // INVALID INPUT TEST
    {
        std::unique_ptr<uint8_t[]> outputMessage, outputHash;
        uint8_t prefix[] = {0x12, 0x30};
        assert(EXIT_FAILURE == findHash(nullptr, 10, outputMessage, outputMessageLen, outputHash, outputHashLen));
        assert(EXIT_FAILURE == findHash(prefix, 1000, outputMessage, outputMessageLen, outputHash, outputHashLen));
        assert(EXIT_FAILURE == findHash(prefix, 10, outputMessage, outputMessageLen, outputHash, outputHashLen, nullptr));
    }

    // BASIC TEST FOR 3 POINTS
    {
        std::unique_ptr<uint8_t[]> outputMessage, outputHash;
        uint8_t prefix[] = {0x12, 0x34};
        assert(EXIT_SUCCESS == findHash(prefix, 15, outputMessage, outputMessageLen, outputHash, outputHashLen, "sha384"));
        assert(EXIT_SUCCESS == checkHash(prefix, 15, outputHash));
    }

    return EXIT_SUCCESS;
}
#endif /* __PROGTEST__ */