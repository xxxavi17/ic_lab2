#ifndef GOLOMB_CODING_H
#define GOLOMB_CODING_H

#include <vector>
#include <string>

class GolombCoding {
private:
    int m; // Golomb parameter
    int b; // Number of bits for remainder
    int cutoff; // Cutoff value for unary code

public:
    // Constructor
    explicit GolombCoding(int parameter);

    // Encoding functions
    std::vector<bool> encode(int n);
    std::vector<bool> encodeSignMagnitude(int n);
    std::vector<bool> encodeInterleaving(int n);

    // Decoding functions
    int decode(const std::vector<bool>& bits);
    int decodeSignMagnitude(const std::vector<bool>& bits);
    int decodeInterleaving(const std::vector<bool>& bits);

    // Helper functions
    std::string bitsToString(const std::vector<bool>& bits);
    void setParameter(int parameter);
    int getParameter() const;
};

#endif // GOLOMB_CODING_H
