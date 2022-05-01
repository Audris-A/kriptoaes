build:
	c++ main.cpp aescfb.cpp aescbc.cpp aes.cpp -o main -lcrypto

run:
	./main

all: build run
