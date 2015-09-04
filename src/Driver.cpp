// g++ -g3 -ggdb -O0 -DDEBUG -I/usr/include/cryptopp Driver.cpp -o Driver.exe -lcryptopp -lpthread
// g++ -g -O2 -DNDEBUG -I/usr/include/cryptopp Driver.cpp -o Driver.exe -lcryptopp -lpthread

#include "crypto++/osrng.h"
using CryptoPP::AutoSeededRandomPool;

#include <iostream>
using std::cout;
using std::cerr;
using std::endl;

#include <string>
using std::string;

#include <cstdlib>
using std::exit;

#include "crypto++/cryptlib.h"
using CryptoPP::Exception;

#include "crypto++/hex.h"
using CryptoPP::HexEncoder;
using CryptoPP::HexDecoder;

#include "crypto++/filters.h"
using CryptoPP::StringSink;
using CryptoPP::StringSource;
using CryptoPP::StreamTransformationFilter;

#include "crypto++/aes.h"
using CryptoPP::AES;

#include "crypto++/modes.h"
using CryptoPP::CFB_Mode;

#include <boost/lexical_cast.hpp>
#include "../../stage/ccpp/meta.hpp"

int main(int argc, char* argv[])
{
	AutoSeededRandomPool prng;

	byte key[AES::DEFAULT_KEYLENGTH];
	std::memcpy(key, "23DE1972B7E7D2EE", 16);
//	prng.GenerateBlock(key, sizeof(key));

	byte iv[AES::BLOCKSIZE];
	std::memcpy(iv, "23DE1972B7E7D2EE", 16);
//	prng.GenerateBlock(iv, sizeof(iv));

	string plain;
	if (argc > 1) {
		plain = argv[1];
	} else {
		plain = "CFB Mode Test";
	}

	int pos = 2;
	if (argc > 2) {
		pos = boost::lexical_cast<int>(argv[2]);
	}

	string cipher, encoded, recovered;

	/*********************************\
	\*********************************/

	// Pretty print key
	encoded.clear();
	StringSource(key, sizeof(key), true,
		new HexEncoder(
			new StringSink(encoded)
		) // HexEncoder
	); // StringSource
	cout << "key(" << sizeof(key) << "): " << encoded << endl;

	// Pretty print iv
	encoded.clear();
	StringSource(iv, sizeof(iv), true,
		new HexEncoder(
			new StringSink(encoded)
		) // HexEncoder
	); // StringSource
	cout << "iv(" << sizeof(iv) << "): " << encoded << endl;

	/*********************************\
	\*********************************/

	try
	{
		cout << "plain text: [" << plain << "]" << endl;

		CFB_Mode< AES >::Encryption e;
		e.SetKeyWithIV(key, sizeof(key), iv);

		// CFB mode must not use padding. Specifying
		//  a scheme will result in an exception
		auto stf = new StreamTransformationFilter(e, new StringSink(cipher));
		cout << "algorithm: [" << stf->AlgorithmName() << "]" << endl;
		StringSource(plain, true, stf); // StringSource
	}
	catch(const CryptoPP::Exception& e)
	{
		cerr << e.what() << endl;
		exit(1);
	}

	/*********************************\
	\*********************************/

	// Pretty print
	encoded.clear();
	StringSource(cipher, true,
		new HexEncoder(
			new StringSink(encoded)
		) // HexEncoder
	); // StringSource
	cout << "cipher text(" << encoded.size() << "): [" << encoded << "]" << endl;

	/*********************************\
	\*********************************/

	try
	{
		CFB_Mode< AES >::Decryption dec;
		dec.SetKeyWithIV(key, sizeof(key), iv);


		// The StreamTransformationFilter removes
		//  padding as required.
		auto sink = new StringSink(recovered);
		auto stf = new StreamTransformationFilter(dec);
		stf->Attach(sink);
		CS_DUMP(plain.size());
		CS_DUMP(cipher.size());
		CS_DUMP(pos);
		stf->Put(reinterpret_cast<const unsigned char*>(cipher.data()), pos);
		stf->MessageEnd();
		stf->Detach();
		stf->Flush(false);
		CS_DUMP(stf->Attachable());
		CS_DUMP(recovered);
		stf->Put(reinterpret_cast<const unsigned char*>(cipher.data() + pos), cipher.size() - pos);
		CS_DUMP(recovered);
		stf->MessageEnd();
		CS_DUMP(recovered);
//		stf->Put(reinterpret_cast<const unsigned char*>(cipher.data() + pos), cipher.size() - pos);

//		StringSource s(cipher, true,
//			 // StreamTransformationFilter
//		); // StringSource

		cout << "recovered text(" << recovered.size() << "): [" << recovered << "]" << endl;
	}
	catch(const CryptoPP::Exception& e)
	{
		cerr << e.what() << endl;
		exit(1);
	}

	/*********************************\
	\*********************************/

	return 0;
}

