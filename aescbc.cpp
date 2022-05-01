#include "md1_libs.h"
#include "aes.h"
#include "aescbc.h"

using namespace std;

void cbcEncrypt(string infname, string outfname) {
    FILE *infile = fopen(infname.c_str(), "rb");
    FILE *outfile = fopen(outfname.c_str(), "wb");

    int bytesRead;
    unsigned char iv[16];

    unsigned char key[16];
    unsigned char expandedKey[176];
    unsigned char plaintext[16]; 
    unsigned char prev_cipher[16];

    fseek(infile,0,SEEK_END);
    int size = ftell(infile);
    
    fseek(infile, 0, 0);

    if (infile == NULL || outfile == NULL) {
        perror("Error opening file");
        exit(1);
    }

    for (int i = 0; i < 16; i++) {
        iv[i] = 0;
    }

    // Random, cryptographically strong (kind of) key generation
    //  using openssl lib function RAND_bytes()
    int randres = 0;
    while (randres != 1) {
        randres = RAND_bytes(key, 16);
    }

    // Save the key for decryption
    ofstream keyFile("key.txt");
    keyFile << key;

    ustrncpy(prev_cipher, iv, 16);

    expandKey(key, expandedKey);

    fpos_t pos[2];
    int position_it = 0;

    // CBC encrypt
    //   Encrypting full blocks
    while ((bytesRead = fread(plaintext, 1, 16, infile)) == 16) {
        fgetpos(outfile,&pos[position_it]);
        position_it = !position_it;
        uxorn(plaintext, prev_cipher, 16);
        aesEncrypt(plaintext, expandedKey);

        fwrite(plaintext, 1, bytesRead, outfile);
        ustrncpy(prev_cipher, plaintext, 16);
    }

    // handle last bytes
    if (bytesRead != 0 && bytesRead != 16) {
        // Zero padding the last block
        for (int i = bytesRead; i < 16; i++) {
            plaintext[i] = 0;
        }

        // XOR with the previos ciphertext
        uxorn(plaintext, prev_cipher, 16);

        // encrypting
        aesEncrypt(plaintext, expandedKey);

        // setting pointer position to the start of the second to last block 
        position_it = !position_it;
        fsetpos(outfile,&pos[position_it]);

        // Writing the last block ciphertext in the second to last block position
        fwrite(plaintext, 1, 16, outfile);

        // Writing the second to last block ciphertext n bytes in the remaining position
        fwrite(prev_cipher, 1, bytesRead, outfile);
    }

    fclose(infile);
    fclose(outfile);
}

void cbcDecrypt(string infname, string outfname) {
    FILE *infile = fopen(infname.c_str(), "rb");
    FILE *outfile = fopen(outfname.c_str(), "wb");
    FILE *keyfile = fopen("key.txt", "rb");

    int bytesRead;

    unsigned char key[16];
    unsigned char iv[16];

    unsigned char expandedKey[176];

    unsigned char current_cipher[16];
    unsigned char prev_cipher[16];
    unsigned char third_cipher[16];

    unsigned char checkBytes[16];

    int checkedBytes;

    if (infile == NULL || outfile == NULL) { 
        perror("Error opening file");
        exit(1);
    }

    // Read the key generated in the encryption stage
    int keyBytesRead = fread(key, 1, 16, keyfile);
    if (keyBytesRead < 16) {
        cout << "Key reading problem!\n";
    }

    for (int i = 0; i < 16; i++) {
        iv[i] = 0;
    }

    expandKey(key, expandedKey);

    ustrncpy(third_cipher, iv, 16);

    fpos_t pos[2];
    fpos_t infilepos;
    int position_it = 0;

    // CBC decrypt
    while ((bytesRead = fread(current_cipher, 1, 16, infile)) == 16) {
        fgetpos(outfile,&pos[position_it]);
        position_it = !position_it;

        fgetpos(infile,&infilepos);
        
        ustrncpy(prev_cipher, current_cipher, 16);
        aesDecrypt(current_cipher, expandedKey);
        uxorn(current_cipher, third_cipher, 16);
        fwrite(current_cipher, 1, bytesRead, outfile);
        
        if ((checkedBytes = fread(current_cipher, 1, 16, infile)) == 16) {
            ustrncpy(third_cipher, prev_cipher, 16);
        }

        fsetpos(infile,&infilepos);
    }

    // Uneven blocks
    if (checkedBytes != 16) {
        // ECB mode decryption of the second to last block
        aesDecrypt(prev_cipher, expandedKey);

        // Last block padding using B-M bits of the second to last block decryption
        for (int i = bytesRead; i < 16; i++) {
            current_cipher[i] = prev_cipher[i];
        }

        uxorn(prev_cipher, current_cipher, 16);

        aesDecrypt(current_cipher, expandedKey);

        // xor last block with third to last block
        //  because last block is replaced with the second to last block
        uxorn(current_cipher, third_cipher, 16);

        position_it = !position_it;
        fsetpos(outfile,&pos[position_it]);

        fwrite(current_cipher, 1, 16, outfile);
        fwrite(prev_cipher, 1, bytesRead, outfile);
    }
    
    fclose(infile);
    fclose(outfile);
}
