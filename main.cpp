#include <iostream>
#include <fstream>
#include <vector>
#include <string>
#include <cstring>
#include <sstream>

using namespace std;

/*void addRoundKey () {
    //
}*/

//input test (appendix B)3243f6a8885a308d313198a2e0370734

int main () {

    unsigned char x[17] = "abcdefghijklmnop";
    unsigned int box[3][3];
    //cout << x << endl;

    //Get symbol hex value
    int xValue = 0;
    int yValue = 0;
    for (int i = 0; i < 16; i++){
        if (x[i] != '\0'){
            box[xValue][yValue] = static_cast<unsigned int>(x[i]);
        }

        yValue++;

        if (i == 3 || i == 7 || i == 11 || i == 15) {
            xValue++;
            yValue = 0;
        }
    }

    for (int i = 0; i < 4; i++) {
        for (int j = 0; j < 4; j++) {
            cout << hex << box[i][j] << endl;
        }
    }

    /*ifstream ifs;

    ifs.open("test.txt", ios::in);

    if (ifs.fail())
    {
        //error
        cout << "Error" << endl;
    }

    vector<unsigned char> bytes;

    while (!ifs.eof())
    {
        unsigned char byte;

        ifs >> byte;
        cout << "ifs = " << byte << endl;
        if (ifs.fail())
        {
            //error
            break;
        }

        bytes.push_back(byte);
    }

    ifs.close();

    for(int i : bytes)
        cout << "i = " << i << endl;*/

    return 0;
}
