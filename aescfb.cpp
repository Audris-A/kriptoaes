#include "md1_libs.h"
#include "aescfb.h"
#include "aes.h"

using namespace std;

// I implemented this by myself by looking at the algorithm description
//   found in https://datatracker.ietf.org/doc/rfc4493/
//   It is CBC-MAC with AES
void CMACGeneraton(FILE * infile, unsigned char * MAC, unsigned char * recv_key, unsigned char * K1, unsigned char * K2){
    unsigned char key[16];

    ustrncpy(key, recv_key, 16);

    unsigned char plaintext[16]; // also will hold the cipher text
    unsigned char prev_cipher[16];
    unsigned char nextplaintext[16];

    int bytesRead;
    int checkedBytes;

    fpos_t infilepos;

    unsigned char expandedKey[176];
    expandKey(key, expandedKey);

    // Create first iteration
    // I can assume that the input has atleast one full block
    bytesRead = fread(plaintext, 1, 16, infile);
    aesEncrypt(plaintext, expandedKey);
    ustrncpy(prev_cipher, plaintext, 16);

    //   Encrypting full blocks
    int timesread = 0;
    while ((bytesRead = fread(plaintext, 1, 16, infile)) == 16) {
        fgetpos(infile,&infilepos);

        timesread++;

        if (fread(nextplaintext, 1, 16, infile) == 0) {
            // Reached the last block
            fsetpos(infile,&infilepos);
            break;
        }

        fsetpos(infile,&infilepos);

        uxorn(plaintext, prev_cipher, 16);
        aesEncrypt(plaintext, expandedKey);
        ustrncpy(prev_cipher, plaintext, 16);
    }

    if (bytesRead == 16){
        // Use K1
        uxorn(plaintext, prev_cipher, 16);
        uxorn(plaintext, K1, 16);
        aesEncrypt(plaintext, expandedKey);
    } else if (bytesRead < 16) {
        // Pad the plaintext with 10^i
        //   padding method = ISO/IEC 9797-1
        plaintext[bytesRead] = 1;
        bytesRead++;
        for (int i = bytesRead; i < 16; i++) {
            plaintext[i] = 0;
        }

        // Use K2
        uxorn(plaintext, prev_cipher, 16);
        uxorn(plaintext, K2, 16);
        aesEncrypt(plaintext, expandedKey);
    }

    // Fill the MAC array
    for (int i = 0; i < 16; i++) {
        MAC[i] = plaintext[i];
    }
}

void cfbEncrypt(string infname, string outfname) {
    FILE *infile = fopen(infname.c_str(), "rb");
    FILE *outfile = fopen(outfname.c_str(), "wb");

    if (infile == NULL || outfile == NULL) {
        perror("Error opening file");
        exit(1);
    }

    // Values/keys to generate
    unsigned char k1[16];
    unsigned char k2[16];
    unsigned char key[16];
    unsigned char mac_key[16];
    unsigned char iv[16];

    // Random, cryptographically strong (kind of) key generation
    //   using openssl RAND_bytes() function
    int randres = 0;
    for (int i = 0; i < 16; i++) {
        while (randres != 1) {
            randres = RAND_bytes(k1, 16);
        }
        
        randres = 0;
        while (randres != 1) {
            randres = RAND_bytes(k2, 16);
        }

        randres = 0;
        while (randres != 1) {
            randres = RAND_bytes(iv, 16);
        }

        randres = 0;
        while (randres != 1) {
            randres = RAND_bytes(key, 16);
        }

        randres = 0;
        while (randres != 1) {
            randres = RAND_bytes(mac_key, 16);
        }
    }

    unsigned char MACValue[16];

    // TODO: Call MAC generation here!
    CMACGeneraton(infile, MACValue, mac_key, k1, k2);

    fseek(infile, 0, 0);

    // Save the key for decryption
    ofstream mac_k1_file("mac_k1.txt");
    ofstream mac_k2_file("mac_k2.txt");
    ofstream mac_key_file("mac_key.txt");
    ofstream iv_file("iv.txt");
    ofstream key_file("key.txt");
    ofstream enc_mac("enc_mac.txt");
    for (int i = 0; i < 16; i++){
        mac_k1_file << k1[i];
        mac_k2_file << k2[i];
        mac_key_file << mac_key[i];
        iv_file << iv[i];
        key_file << key[i];
        enc_mac << MACValue[i];
    }

    int bytesRead;

    unsigned char expandedKey[176];

    unsigned char bytes[16]; // read plaintext
    unsigned char oldBytes[16]; // holds the previous ciphertext

    unsigned char plaincopy[16]; // just a copy of the read plaintext

    ustrncpy(oldBytes, iv, 16);

    expandKey(key, expandedKey);

    aesEncrypt(iv, expandedKey);
    bytesRead = fread(bytes, 1, 16, infile);
    uxorn(bytes, iv, 16);
    fwrite(bytes, 1, bytesRead, outfile);
    ustrncpy(oldBytes, bytes, 16);

    while ((bytesRead = fread(bytes, 1, 16, infile)) == 16) {
        ustrncpy(plaincopy, bytes, 16);
        aesEncrypt(oldBytes, expandedKey);
        uxorn(bytes, oldBytes, 16);
        fwrite(bytes, 1, bytesRead, outfile);
        ustrncpy(oldBytes, bytes, 16);
    }

    // Pad every message even when the message
    //   is a multiple of the block size
    //   padding method = ISO/IEC 9797-1
    // handle last bytes
    if (bytesRead != 0) {
        bytes[bytesRead++] = 1;
        for (int i = bytesRead; i < 16; i++) {
            bytes[i] = 0;//padd;
        }

        aesEncrypt(oldBytes, expandedKey);
        uxorn(bytes, oldBytes, 16);
        
        fwrite(bytes, 1, 16, outfile);
    } else {
        // insert padding block
        bytes[0] = 1;
        for (int i = 1; i < 16; i++) {
            bytes[i] = 0;
        }

        aesEncrypt(oldBytes, expandedKey);
        uxorn(bytes, oldBytes, 16);
        
        fwrite(bytes, 1, 16, outfile);
    }

    fclose(infile);
    fclose(outfile);
}

void cfbDecrypt(string infname, string outfname) {
    FILE *infile = fopen(infname.c_str(), "rb");
    FILE *outfile = fopen(outfname.c_str(), "wb");

    if (infile == NULL || outfile == NULL) {
        perror("Error opening file");
        exit(1);
    }

    // Read previously generated mac value
    FILE *encMACFile = fopen("enc_mac.txt", "rb");
    unsigned char enc_mac[16];
    int macKeybytesRead = fread(enc_mac, 1, 16, encMACFile);
    if (macKeybytesRead < 16) {
        cout << "ENC MAC VALUE READ ERROR!\n";
    }

    // Read previously generated keys and iv
    FILE *mac_k1_file = fopen("mac_k1.txt", "rb");
    FILE *mac_k2_file = fopen("mac_k2.txt", "rb");
    FILE *key_file = fopen("key.txt", "rb");
    FILE *iv_file = fopen("iv.txt", "rb");

    unsigned char k1[16];
    unsigned char k2[16];
    int bytesRead; 
    bytesRead = fread(k1, 1, 16, mac_k1_file);
    if (bytesRead != 16) {
        cout << "Error: k1 file should contain atleast 16 bytes" << endl;
        exit(1);
    }

    bytesRead = fread(k2, 1, 16, mac_k2_file);
    if (bytesRead != 16) {
        cout << "Error: k2 file should contain atleast 16 bytes" << endl;
        exit(1);
    }

    unsigned char key[16];
    unsigned char iv[16];

    bytesRead = fread(key, 1, 16, key_file);
    if (bytesRead != 16) {
        cout << "Error: key file should contain atleast 16 bytes" << endl;
        exit(1);
    }
    bytesRead = fread(iv, 1, 16, iv_file);
    if (bytesRead != 16) {
        cout << "Error: ivfile file should contain atleast 16 bytes" << endl;
        exit(1);
    }

    fclose(key_file);
    fclose(iv_file);

    unsigned char expandedKey[176];

    unsigned char bytes[16];
    unsigned char oldBytes[16];
    unsigned char veryOldBytes[16];

    unsigned char MACValue[16];

    unsigned char checkBytes[16];
    int checkedBytes;
    fpos_t pos; // for returning to the current pointer after checking the next block

    // Decryption block
    expandKey(key, expandedKey);

    aesEncrypt(iv, expandedKey);
    bytesRead = fread(bytes, 1, 16, infile);
    ustrncpy(veryOldBytes, bytes, 16);
    uxorn(bytes, iv, 16);
    fwrite(bytes, 1, bytesRead, outfile);
    while ((bytesRead = fread(bytes, 1, 16, infile)) == 16) {
        ustrncpy(oldBytes, bytes, 16);
        aesEncrypt(veryOldBytes, expandedKey);
        uxorn(bytes, veryOldBytes, 16);
        
        fgetpos(infile, &pos);
        // Skip writing the last block
        //   because it can be just padding and it can also be the needed bytes with padding
        if ((checkedBytes = fread(checkBytes, 1, 16, infile)) == 16){
            fwrite(bytes, 1, bytesRead, outfile);
        }
        
        fsetpos(infile, &pos);
        
        ustrncpy(veryOldBytes, oldBytes, 16);
    }

    // Filter the last block from padding
    int lastBlockEnd = 0;
    bool zero_start = false;
    if (bytes[15] == 0) {
        for (int i = 15; i > 0; i--){
            lastBlockEnd++;

            if ((int) bytes[i] == 0){
                continue;
            } else if ((int) bytes[i] == 1){
                break;
            } else {
                // Either there was a problem or the block is full without padding
                lastBlockEnd = 0;
                cout << "Maybe there was a problem." << endl;
                cout << "Or the last block just was full - without padding." << endl;
            }
        }
    }

    fwrite(bytes, 1, 16 - lastBlockEnd, outfile);
    fclose(outfile);

    // Set up the decrypted file for reading
    outfile = fopen(outfname.c_str(), "rb");

    // Get the mac key that was generated in the encryption stage
    FILE *macKeyFile = fopen("mac_key.txt", "rb");
    unsigned char mac_key[16];
    macKeybytesRead = fread(mac_key, 1, 16, macKeyFile);
    
    if (macKeybytesRead < 16) {
        cout << "MAC KEY READ ERROR!\n";
        cout << macKeybytesRead;
    } else {
        // Call mac generation
        unsigned char MAC[16];
        CMACGeneraton(outfile, MAC, mac_key, k1, k2);

        ofstream decr_mac("decr_mac.txt");
        for (int i = 0; i < 16; i++){
            decr_mac << MAC[i];
        }

        bool MAC_values_are_equal = true;
        for (int i = 0; i < 16; i++){
            if (MAC[i] != enc_mac[i]){
                cout << "\nTHE MAC VALUES DIFFER!\n";
                MAC_values_are_equal = false;

                cout << "The MAC values are: \n";
                cout << "enc mac = \n";
                for (int i = 0; i < 16; i++){
                    cout << enc_mac[i];
                }
                cout << "\ndecr mac = \n";
                for (int i = 0; i < 16; i++){
                    cout << MAC[i];
                }
                cout << "\n\n";
            
                break;
            }
        }

        if (MAC_values_are_equal) {
            cout << "ENC MAC == DEC MAC\n";
        }
    }

    fclose(macKeyFile);
    fclose(infile);
}
