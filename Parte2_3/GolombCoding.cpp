#include "GolombCoding.h"
#include <cmath>
#include <sstream>
#include <stdexcept>

// Constructor
GolombCoding::GolombCoding(int parameter) : m(parameter) {
    if (m <= 0) {
        throw std::invalid_argument("Golomb parameter m must be positive");
    }
    b = static_cast<int>(std::ceil(std::log2(m)));
    cutoff = static_cast<int>(std::pow(2, b)) - m;
}

// Set parameter
void GolombCoding::setParameter(int parameter) {
    if (parameter <= 0) {
        throw std::invalid_argument("Golomb parameter m must be positive");
    }
    m = parameter;
    b = static_cast<int>(std::ceil(std::log2(m)));
    cutoff = static_cast<int>(std::pow(2, b)) - m;
}

// Get parameter
int GolombCoding::getParameter() const {
    return m;
}

// Encode a non-negative integer
std::vector<bool> GolombCoding::encode(int n) {
    if (n < 0) {
        throw std::invalid_argument("Use encodeSignMagnitude or encodeInterleaving for negative numbers");
    }

    std::vector<bool> result;
    
    // Calculate quotient and remainder
    int q = n / m;
    int r = n % m;
    
    // Unary code for quotient: q zeros followed by a 1
    for (int i = 0; i < q; i++) {
        result.push_back(false);
    }
    result.push_back(true);
    
    // Binary code for remainder
    if (r < cutoff) {
        // Use b-1 bits
        for (int i = b - 2; i >= 0; i--) {
            result.push_back((r >> i) & 1);
        }
    } else {
        // Use b bits (add cutoff to remainder)
        int r_adjusted = r + cutoff;
        for (int i = b - 1; i >= 0; i--) {
            result.push_back((r_adjusted >> i) & 1);
        }
    }
    
    return result;
}

// Encode using sign and magnitude approach
std::vector<bool> GolombCoding::encodeSignMagnitude(int n) {
    std::vector<bool> result;
    
    // Sign bit: 0 for positive, 1 for negative
    if (n < 0) {
        result.push_back(true);
        n = -n;
    } else {
        result.push_back(false);
    }
    
    // Encode the magnitude
    std::vector<bool> magnitude = encode(n);
    result.insert(result.end(), magnitude.begin(), magnitude.end());
    
    return result;
}

// Encode using positive/negative interleaving approach
std::vector<bool> GolombCoding::encodeInterleaving(int n) {
    int mapped;
    
    // Map negative and positive numbers to non-negative integers
    // 0 -> 0, -1 -> 1, 1 -> 2, -2 -> 3, 2 -> 4, -3 -> 5, 3 -> 6, ...
    if (n >= 0) {
        mapped = 2 * n;
    } else {
        mapped = -2 * n - 1;
    }
    
    return encode(mapped);
}

// Decode a Golomb-coded sequence
int GolombCoding::decode(const std::vector<bool>& bits) {
    if (bits.empty()) {
        throw std::invalid_argument("Empty bit sequence");
    }
    
    // Read unary code for quotient
    int q = 0;
    size_t pos = 0;
    while (pos < bits.size() && !bits[pos]) {
        q++;
        pos++;
    }
    
    if (pos >= bits.size()) {
        throw std::invalid_argument("Invalid Golomb code: no terminator for unary code");
    }
    
    pos++; // Skip the terminator '1'
    
    // Read binary code for remainder
    int r = 0;
    
    // First, try to read b-1 bits
    if (pos + (b - 1) > bits.size()) {
        throw std::invalid_argument("Invalid Golomb code: insufficient bits for remainder");
    }
    
    for (int i = 0; i < b - 1; i++) {
        r = (r << 1) | (bits[pos++] ? 1 : 0);
    }
    
    // Check if we need the additional bit
    if (r < cutoff) {
        // r is correct with b-1 bits
    } else {
        // Need to read one more bit
        if (pos >= bits.size()) {
            throw std::invalid_argument("Invalid Golomb code: insufficient bits for remainder");
        }
        r = (r << 1) | (bits[pos++] ? 1 : 0);
        r -= cutoff;
    }
    
    return q * m + r;
}

// Decode using sign and magnitude approach
int GolombCoding::decodeSignMagnitude(const std::vector<bool>& bits) {
    if (bits.empty()) {
        throw std::invalid_argument("Empty bit sequence");
    }
    
    // Read sign bit
    bool negative = bits[0];
    
    // Decode magnitude
    std::vector<bool> magnitude_bits(bits.begin() + 1, bits.end());
    int magnitude = decode(magnitude_bits);
    
    return negative ? -magnitude : magnitude;
}

// Decode using positive/negative interleaving approach
int GolombCoding::decodeInterleaving(const std::vector<bool>& bits) {
    int mapped = decode(bits);
    
    // Reverse the mapping
    if (mapped % 2 == 0) {
        return mapped / 2;  // Even -> positive
    } else {
        return -(mapped + 1) / 2;  // Odd -> negative
    }
}

// Convert bit vector to string for display
std::string GolombCoding::bitsToString(const std::vector<bool>& bits) {
    std::ostringstream oss;
    for (bool bit : bits) {
        oss << (bit ? '1' : '0');
    }
    return oss.str();
}
