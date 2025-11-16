// Parte4/BitStream.h
#ifndef BITSTREAM_H
#define BITSTREAM_H

#include <fstream>
#include <string>

class BitStream {
public:
    BitStream(const std::string& filename, const std::string& mode);
    ~BitStream();
    void writeBit(int bit);
    void writeNBits(unsigned int value, int n);
    int readBit();
    unsigned int readNBits(int n);
    void close();

private:
    std::fstream file;
    std::string mode;
    unsigned char buffer;
    int bitCount;
    void flush();
};

#endif // BITSTREAM_H