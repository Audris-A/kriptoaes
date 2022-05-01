using namespace std;

// AES
void aesEncrypt(unsigned char *plaintxt, unsigned char *expandedKey);
void aesDecrypt(unsigned char *cyphertxt, unsigned char *expandedKey);
void expandKey(unsigned char *key, unsigned char *expandedKey);

// Helpers
void ustrncpy(unsigned char *dest, unsigned char *src, int n);
void uxorn(unsigned char *dest, unsigned char *src, int n);
