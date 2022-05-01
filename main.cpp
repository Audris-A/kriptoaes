/*
    Just the UI of the program
      which will call the selected algorithm.
*/

#include "md1_libs.h"
#include "aescbc.h"
#include "aescfb.h"

using namespace std;

int main() {
    string infname, outfname;
    int mode;

    int cipherType;
    
    cout << "Choose the cipher type" << endl
         << "1. aes-cbc with zero iv and with cts;" << endl
         << "2. cfb with iv and padding, and AES-CBC-MAC." << endl
         << "Choose: ";
    cin >> cipherType;

    if (cipherType != 1 && cipherType != 2) {
        cout << "Error: no such cipher" << endl;
        return -1;
    }

    cout << "Choose mode" << endl
         << "1. Encrypt" << endl
         << "2. Decrypt" << endl
         << "Choose: ";
    cin >> mode;

    if (mode != 1 && mode != 2) {
        cout << "Error: no such mode" << endl;
        return -1;
    }

    cout << "Enter input filename: ";
    cin >> infname;

    cout << "Enter output filename: ";
    cin >> outfname;

    if (cipherType == 1){
        if (mode == 1) {
            cbcEncrypt(infname, outfname);
            cout << "ENCRIPTION SUCCESSFUL" << endl;
        } else {
            cbcDecrypt(infname, outfname);
            cout << "DECRYPTION SUCCESSFUL" << endl;
        }
    } else if (cipherType == 2){
        if (mode == 1) {
            cfbEncrypt(infname, outfname);
            cout << "ENCRIPTION SUCCESSFUL" << endl;
        } else {
            cfbDecrypt(infname, outfname);
            cout << "DECRYPTION SUCCESSFUL" << endl;
        }
    }

    return 0;
}