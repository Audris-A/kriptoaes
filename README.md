  The program has 2 types of algorithms:
    1) AES-CBC with CTS;
    2) AES-CFB alongside AES-CBC-MAC;

  Everything is written "by-hand". A library is only used for
    generating cryptographically safe keys (sort of).
    
  Not sure about the security of the keys I generate with openssl
  because I'm not using seeds but just plain RAND_bytes() function.
  
  Every generated key and mac value is saved in a seperate file in the project
  directory. Keep in mind that both algorithms use a file named key.txt. So
  running the program with aes-cbc encryption and afterwards aes-cfb decryption
  will probably give unexpected results. 

  Program has been tested only with the given test.pdf file (399210 bytes) and
  another testtext.txt file (2130 bytes). So it didn't have that much of a rigorous
  testing.

  There ofcourse can be optimizations both for the efficiency and error detection.

  To run the program you can use the command: make all
  If you want to compile the program separately: make build
  And then run the program with: make run  

# aescbc
  AES with CBC and CTS.
  Includes random key generation and cipher text stealing

# aescfb
 AES with cfb
 Includes RKG and MAC


Author: Audris Arzovs aa17083
