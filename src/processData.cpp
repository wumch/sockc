
#include <cstring>
#include <iostream>
#include <boost/lexical_cast.hpp>
#include <crypto++/modes.h>
#include <crypto++/aes.h>
#include <crypto++/filters.h>
#include "stage/meta.hpp"

byte key[CryptoPP::AES::DEFAULT_KEYLENGTH] = {0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01};
byte iv[CryptoPP::AES::BLOCKSIZE] = {0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01};

class Crypto
{
private:
    BOOST_STATIC_ASSERT(sizeof(byte) == 1);     // 以下不再考虑 sizeof(byte) != 1 的情况。

    CryptoPP::CFB_Mode<CryptoPP::AES>::Encryption encor;
    CryptoPP::CFB_Mode<CryptoPP::AES>::Decryption decor;

public:

    void encrypt(const char* in, std::size_t len, char* out)
    {
        encrypt(reinterpret_cast<const uint8_t*>(in), len, out);
    }

    void encrypt(const uint8_t* in, std::size_t len, char* out)
    {
        encrypt(in, len, reinterpret_cast<byte*>(out));
    }

    void encrypt(const uint8_t* in, std::size_t len, byte* out)
    {
        return;
        encor.ProcessData(out, in, len);
    }

    void decrypt(const char* in, std::size_t len, char* out)
    {
        decrypt(in, len, reinterpret_cast<byte*>(out));
    }

    void decrypt(const char* in, std::size_t len, uint8_t* out)
    {
        decrypt(reinterpret_cast<const byte*>(in), len, out);
    }

    void decrypt(const byte* in, std::size_t len, uint8_t* out)
    {
        return;
        decor.ProcessData(out, in, len);
    }

    void setEncKeyWithIv(const byte* _key, std::size_t keyLen,
        const byte* _iv, std::size_t ivLen)
    {
        encor.SetKeyWithIV(reinterpret_cast<const byte*>(_key), keyLen,
            reinterpret_cast<const byte*>(_iv), ivLen);
    }

    void setDecKeyWithIv(const byte* _key, std::size_t keyLen,
        const byte* _iv, std::size_t ivLen)
    {
        decor.SetKeyWithIV(reinterpret_cast<const byte*>(_key), keyLen,
            reinterpret_cast<const byte*>(_iv), ivLen);
    }
};

int main(int argc, char* argv[])
{
//	byte key[CryptoPP::AES::DEFAULT_KEYLENGTH], iv[CryptoPP::AES::BLOCKSIZE];

//	std::memcpy(key, "23DE1972B7E7D2EE", 16);
//	std::memcpy(iv, "23DE1972B7E7D2EE", 16);
	{
	    Crypto crypto;
	}
    Crypto* crypto = new Crypto;
//    crypto->setEncKeyWithIv(key, sizeof(key), iv, sizeof(iv));
//    crypto->setDecKeyWithIv(key, sizeof(key), iv, sizeof(iv));

	byte plain[100], cipher[100], deced[100];
	std::memset(plain, 0, 100);
	std::memset(cipher, 0, 100);
	std::memset(deced, 0, 100);

	int pos = 2, len = 0;

	if (argc > 2) {
		len = std::strlen(argv[1]);
		std::memcpy(plain, argv[1], len);
		pos = boost::lexical_cast<int>(argv[2]);
	}

	std::memcpy(plain, "1234567890qwertyuiopasdfghjklzxcvbnm", 36);

	crypto->encrypt(plain, 100, cipher);

	crypto->decrypt(cipher, pos, deced);
	CS_DUMP(deced);

	std::memset(deced, 0, 100);
	crypto->decrypt(cipher + pos, len - pos, deced);
	CS_DUMP(deced);

	delete crypto;

	CS_DUMP(deced);
}
