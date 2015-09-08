
#pragma once

#include <cstring>
#include <boost/static_assert.hpp>
#include <crypto++/modes.h>
#include <crypto++/aes.h>

namespace csocks {

class Crypto
{
private:
    BOOST_STATIC_ASSERT(sizeof(byte) == 1);     // 以下不再考虑 sizeof(byte) != 1 的情况。

    CryptoPP::CFB_Mode<CryptoPP::AES>::Encryption encor;
    CryptoPP::CFB_Mode<CryptoPP::AES>::Decryption decor;

public:
    // never void*
    void encrypt(const char* in, std::size_t len, char* out)
    {
        encrypt(reinterpret_cast<const uint8_t*>(in), len, out);
    }

    void encrypt(const uint8_t* in, std::size_t len, char* out)
    {
        encrypt(in, len, reinterpret_cast<uint8_t*>(out));
    }

    void encrypt(const uint8_t* in, std::size_t len, uint8_t* out)
    {
        encor.ProcessData(out, in, len);
    }

    void decrypt(const char* in, std::size_t len, char* out)
    {
        decrypt(in, len, reinterpret_cast<byte*>(out));
    }

    void decrypt(const char* in, std::size_t len, uint8_t* out)
    {
        decrypt(reinterpret_cast<const uint8_t*>(in), len, out);
    }

    void decrypt(const uint8_t* in, std::size_t len, uint8_t* out)
    {
        decor.ProcessData(out, in, len);
    }

    void setEncKeyWithIv(const char* _key, std::size_t keyLen,
        const char* _iv, std::size_t ivLen)
    {
        setEncKeyWithIv(reinterpret_cast<const byte*>(_key), keyLen,
            reinterpret_cast<const byte*>(_iv), ivLen);
    }

    void setEncKeyWithIv(const byte* _key, std::size_t keyLen,
        const byte* _iv, std::size_t ivLen)
    {
        encor.SetKeyWithIV(_key, keyLen, _iv, ivLen);
    }

    void setDecKeyWithIv(const char* _key, std::size_t keyLen,
        const char* _iv, std::size_t ivLen)
    {
        setDecKeyWithIv(reinterpret_cast<const byte*>(_key), keyLen,
            reinterpret_cast<const byte*>(_iv), ivLen);
    }

    void setDecKeyWithIv(const byte* _key, std::size_t keyLen,
        const byte* _iv, std::size_t ivLen)
    {
        decor.SetKeyWithIV(_key, keyLen, _iv, ivLen);
    }
};

}
