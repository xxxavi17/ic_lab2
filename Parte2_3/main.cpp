#include "GolombCoding.h"
#include <iostream>
#include <iomanip>

void testGolombCoding() {
    std::cout << "=== Golomb Coding Test Program ===" << std::endl << std::endl;
    
    // Test with different m values
    int m_values[] = {3, 4, 5, 8};
    
    for (int m : m_values) {
        std::cout << "Testing with m = " << m << std::endl;
        std::cout << std::string(50, '-') << std::endl;
        
        GolombCoding golomb(m);
        
        // Test positive integers
        std::cout << "\nEncoding positive integers:" << std::endl;
        for (int n = 0; n <= 10; n++) {
            auto encoded = golomb.encode(n);
            int decoded = golomb.decode(encoded);
            std::cout << "  n=" << std::setw(2) << n 
                      << " -> " << std::setw(12) << golomb.bitsToString(encoded)
                      << " (" << std::setw(2) << encoded.size() << " bits)"
                      << " -> decoded: " << decoded
                      << (decoded == n ? " ✓" : " ✗") << std::endl;
        }
        
        // Test sign-magnitude approach
        std::cout << "\nSign-Magnitude approach (negative numbers):" << std::endl;
        int test_values[] = {5, -5, 3, -3, 0, -1, 1, -7, 7};
        for (int n : test_values) {
            auto encoded = golomb.encodeSignMagnitude(n);
            int decoded = golomb.decodeSignMagnitude(encoded);
            std::cout << "  n=" << std::setw(3) << n 
                      << " -> " << std::setw(15) << golomb.bitsToString(encoded)
                      << " (" << std::setw(2) << encoded.size() << " bits)"
                      << " -> decoded: " << std::setw(3) << decoded
                      << (decoded == n ? " ✓" : " ✗") << std::endl;
        }
        
        // Test interleaving approach
        std::cout << "\nInterleaving approach (positive/negative):" << std::endl;
        for (int n : test_values) {
            auto encoded = golomb.encodeInterleaving(n);
            int decoded = golomb.decodeInterleaving(encoded);
            std::cout << "  n=" << std::setw(3) << n 
                      << " -> " << std::setw(15) << golomb.bitsToString(encoded)
                      << " (" << std::setw(2) << encoded.size() << " bits)"
                      << " -> decoded: " << std::setw(3) << decoded
                      << (decoded == n ? " ✓" : " ✗") << std::endl;
        }
        
        std::cout << "\n" << std::endl;
    }
}

void interactiveMode() {
    std::cout << "\n=== Interactive Golomb Coding ===" << std::endl;
    
    int m;
    std::cout << "Enter Golomb parameter m: ";
    std::cin >> m;
    
    if (m <= 0) {
        std::cerr << "Error: m must be positive!" << std::endl;
        return;
    }
    
    GolombCoding golomb(m);
    std::cout << "Golomb coding initialized with m = " << m << std::endl;
    
    while (true) {
        std::cout << "\nOptions:" << std::endl;
        std::cout << "  1. Encode positive integer" << std::endl;
        std::cout << "  2. Encode using sign-magnitude" << std::endl;
        std::cout << "  3. Encode using interleaving" << std::endl;
        std::cout << "  4. Change parameter m" << std::endl;
        std::cout << "  0. Exit" << std::endl;
        std::cout << "Choose option: ";
        
        int option;
        std::cin >> option;
        
        if (option == 0) break;
        
        if (option == 4) {
            std::cout << "Enter new m value: ";
            std::cin >> m;
            if (m <= 0) {
                std::cerr << "Error: m must be positive!" << std::endl;
                continue;
            }
            golomb.setParameter(m);
            std::cout << "Parameter updated to m = " << m << std::endl;
            continue;
        }
        
        int n;
        std::cout << "Enter integer to encode: ";
        std::cin >> n;
        
        try {
            std::vector<bool> encoded;
            int decoded;
            
            switch (option) {
                case 1:
                    if (n < 0) {
                        std::cerr << "Error: Use option 2 or 3 for negative numbers" << std::endl;
                        continue;
                    }
                    encoded = golomb.encode(n);
                    decoded = golomb.decode(encoded);
                    break;
                case 2:
                    encoded = golomb.encodeSignMagnitude(n);
                    decoded = golomb.decodeSignMagnitude(encoded);
                    break;
                case 3:
                    encoded = golomb.encodeInterleaving(n);
                    decoded = golomb.decodeInterleaving(encoded);
                    break;
                default:
                    std::cerr << "Invalid option!" << std::endl;
                    continue;
            }
            
            std::cout << "  Input:    " << n << std::endl;
            std::cout << "  Encoded:  " << golomb.bitsToString(encoded) 
                      << " (" << encoded.size() << " bits)" << std::endl;
            std::cout << "  Decoded:  " << decoded;
            std::cout << (decoded == n ? " ✓" : " ✗") << std::endl;
            
        } catch (const std::exception& e) {
            std::cerr << "Error: " << e.what() << std::endl;
        }
    }
}

int main(int argc, char* argv[]) {
    if (argc > 1 && std::string(argv[1]) == "-i") {
        interactiveMode();
    } else {
        testGolombCoding();
        
        std::cout << "\nRun with '-i' flag for interactive mode." << std::endl;
    }
    
    return 0;
}
