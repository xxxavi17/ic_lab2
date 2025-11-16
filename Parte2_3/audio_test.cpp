#include "AudioCodec.h"
#include "WAVFile.h"
#include "GolombCoding.h"
#include <iostream>
#include <vector>
#include <cmath>
#include <cstdlib>
#include <ctime>
#include <iomanip>

// Generate synthetic audio samples for testing
std::vector<int16_t> generateSineWave(double frequency, double duration, uint32_t sampleRate, double amplitude = 16000.0) {
    uint32_t numSamples = static_cast<uint32_t>(duration * sampleRate);
    std::vector<int16_t> samples(numSamples);
    
    for (uint32_t i = 0; i < numSamples; i++) {
        double t = static_cast<double>(i) / sampleRate;
        samples[i] = static_cast<int16_t>(amplitude * std::sin(2.0 * M_PI * frequency * t));
    }
    
    return samples;
}

// Generate synthetic audio with multiple frequencies
std::vector<int16_t> generateComplexWave(double duration, uint32_t sampleRate) {
    uint32_t numSamples = static_cast<uint32_t>(duration * sampleRate);
    std::vector<int16_t> samples(numSamples);
    
    // Mix multiple frequencies
    double frequencies[] = {440.0, 880.0, 1320.0}; // A4, A5, E6
    double amplitudes[] = {10000.0, 5000.0, 2500.0};
    
    for (uint32_t i = 0; i < numSamples; i++) {
        double t = static_cast<double>(i) / sampleRate;
        double value = 0.0;
        
        for (int j = 0; j < 3; j++) {
            value += amplitudes[j] * std::sin(2.0 * M_PI * frequencies[j] * t);
        }
        
        samples[i] = static_cast<int16_t>(value);
    }
    
    return samples;
}

// Test mono audio compression
void testMonoCompression() {
    std::cout << "\n=== Testing Mono Audio Compression ===" << std::endl;
    std::cout << std::string(60, '=') << std::endl;
    
    // Generate test audio
    uint32_t sampleRate = 44100;
    double duration = 1.0; // 1 second
    
    std::cout << "\nGenerating sine wave (440 Hz, 1 second)..." << std::endl;
    std::vector<int16_t> samples = generateSineWave(440.0, duration, sampleRate);
    
    // Setup audio info
    AudioInfo info;
    info.sampleRate = sampleRate;
    info.channels = 1;
    info.bitsPerSample = 16;
    info.numSamples = samples.size();
    
    std::cout << "Samples generated: " << samples.size() << std::endl;
    
    // Test with different Golomb parameters
    int golombParams[] = {4, 8, 16, 32, 64};
    
    for (int m : golombParams) {
        std::cout << "\n--- Testing with Golomb parameter m = " << m << " ---" << std::endl;
        
        AudioCodec codec(m, false); // Non-adaptive
        
        // Encode
        auto start = std::clock();
        CompressedAudio compressed = codec.encode(samples, info);
        auto encodeTime = std::clock() - start;
        
        codec.printStatistics(compressed);
        std::cout << "Encoding time: " << (1000.0 * encodeTime / CLOCKS_PER_SEC) << " ms" << std::endl;
        
        // Decode
        start = std::clock();
        AudioInfo decodedInfo;
        std::vector<int16_t> decoded = codec.decode(compressed, decodedInfo);
        auto decodeTime = std::clock() - start;
        
        std::cout << "Decoding time: " << (1000.0 * decodeTime / CLOCKS_PER_SEC) << " ms" << std::endl;
        
        // Verify lossless compression
        bool lossless = true;
        size_t maxErrors = 10;
        size_t errorCount = 0;
        
        for (size_t i = 0; i < std::min(samples.size(), decoded.size()); i++) {
            if (samples[i] != decoded[i]) {
                if (errorCount < maxErrors) {
                    std::cout << "Error at sample " << i << ": original=" << samples[i] 
                              << ", decoded=" << decoded[i] << std::endl;
                }
                errorCount++;
                lossless = false;
            }
        }
        
        if (lossless) {
            std::cout << "✓ Lossless compression verified!" << std::endl;
        } else {
            std::cout << "✗ Compression is NOT lossless! Total errors: " << errorCount << std::endl;
        }
    }
    
    // Test adaptive mode
    std::cout << "\n--- Testing with Adaptive Golomb parameter ---" << std::endl;
    AudioCodec adaptiveCodec(16, true);
    CompressedAudio compressed = adaptiveCodec.encode(samples, info);
    adaptiveCodec.printStatistics(compressed);
}

// Test stereo audio compression
void testStereoCompression() {
    std::cout << "\n\n=== Testing Stereo Audio Compression ===" << std::endl;
    std::cout << std::string(60, '=') << std::endl;
    
    // Generate test audio
    uint32_t sampleRate = 44100;
    double duration = 1.0;
    
    std::cout << "\nGenerating stereo audio..." << std::endl;
    std::vector<int16_t> leftChannel = generateSineWave(440.0, duration, sampleRate);
    std::vector<int16_t> rightChannel = generateSineWave(554.37, duration, sampleRate); // C#5
    
    AudioInfo info;
    info.sampleRate = sampleRate;
    info.channels = 2;
    info.bitsPerSample = 16;
    info.numSamples = leftChannel.size();
    
    std::cout << "Samples per channel: " << leftChannel.size() << std::endl;
    
    AudioCodec codec(16, false);
    
    // Test without inter-channel prediction
    std::cout << "\n--- Without Inter-channel Prediction ---" << std::endl;
    CompressedAudio compressed1 = codec.encodeStereo(leftChannel, rightChannel, info, false);
    codec.printStatistics(compressed1);
    
    // Test with inter-channel prediction
    std::cout << "\n--- With Inter-channel Prediction ---" << std::endl;
    CompressedAudio compressed2 = codec.encodeStereo(leftChannel, rightChannel, info, true);
    codec.printStatistics(compressed2);
    
    std::cout << "\nCompression improvement with inter-channel prediction: " 
              << std::fixed << std::setprecision(2)
              << ((compressed2.compressionRatio / compressed1.compressionRatio - 1.0) * 100.0) 
              << "%" << std::endl;
    
    // Decode and verify
    std::vector<int16_t> decodedLeft, decodedRight;
    AudioInfo decodedInfo;
    codec.decodeStereo(compressed2, decodedLeft, decodedRight, decodedInfo);
    
    // Verify
    bool lossless = true;
    for (size_t i = 0; i < std::min(leftChannel.size(), decodedLeft.size()); i++) {
        if (leftChannel[i] != decodedLeft[i] || rightChannel[i] != decodedRight[i]) {
            lossless = false;
            break;
        }
    }
    
    if (lossless) {
        std::cout << "✓ Lossless stereo compression verified!" << std::endl;
    } else {
        std::cout << "✗ Stereo compression is NOT lossless!" << std::endl;
    }
}

// Test with complex waveforms
void testComplexWaveforms() {
    std::cout << "\n\n=== Testing with Complex Waveforms ===" << std::endl;
    std::cout << std::string(60, '=') << std::endl;
    
    uint32_t sampleRate = 44100;
    double duration = 2.0;
    
    std::cout << "\nGenerating complex waveform (multiple frequencies)..." << std::endl;
    std::vector<int16_t> samples = generateComplexWave(duration, sampleRate);
    
    AudioInfo info;
    info.sampleRate = sampleRate;
    info.channels = 1;
    info.bitsPerSample = 16;
    info.numSamples = samples.size();
    
    // Compare fixed vs adaptive
    std::cout << "\n--- Fixed Golomb Parameter (m=16) ---" << std::endl;
    AudioCodec fixedCodec(16, false);
    CompressedAudio compressed1 = fixedCodec.encode(samples, info);
    fixedCodec.printStatistics(compressed1);
    
    std::cout << "\n--- Adaptive Golomb Parameter ---" << std::endl;
    AudioCodec adaptiveCodec(16, true);
    CompressedAudio compressed2 = adaptiveCodec.encode(samples, info);
    adaptiveCodec.printStatistics(compressed2);
    
    std::cout << "\nAdaptive mode improvement: " 
              << std::fixed << std::setprecision(2)
              << ((compressed2.compressionRatio / compressed1.compressionRatio - 1.0) * 100.0) 
              << "%" << std::endl;
}

// Test WAV file I/O
void testWAVFileIO() {
    std::cout << "\n\n=== Testing WAV File I/O ===" << std::endl;
    std::cout << std::string(60, '=') << std::endl;
    
    // Generate and save a test WAV file
    uint32_t sampleRate = 44100;
    double duration = 0.5;
    
    std::cout << "\nGenerating test WAV file..." << std::endl;
    std::vector<int16_t> samples = generateSineWave(440.0, duration, sampleRate);
    
    AudioInfo info;
    info.sampleRate = sampleRate;
    info.channels = 1;
    info.bitsPerSample = 16;
    info.numSamples = samples.size();
    
    WAVFile wavFile;
    std::string filename = "test_output.wav";
    
    std::cout << "Writing " << filename << "..." << std::endl;
    if (wavFile.write(filename, samples, info)) {
        std::cout << "✓ File written successfully" << std::endl;
        
        // Read it back
        std::cout << "\nReading " << filename << "..." << std::endl;
        std::vector<int16_t> readSamples;
        AudioInfo readInfo;
        
        if (wavFile.readMono(filename, readSamples, readInfo)) {
            std::cout << "✓ File read successfully" << std::endl;
            wavFile.printInfo();
            
            // Verify
            bool match = (samples.size() == readSamples.size());
            if (match) {
                for (size_t i = 0; i < samples.size(); i++) {
                    if (samples[i] != readSamples[i]) {
                        match = false;
                        break;
                    }
                }
            }
            
            if (match) {
                std::cout << "✓ WAV file I/O verified!" << std::endl;
            } else {
                std::cout << "✗ Data mismatch after read!" << std::endl;
            }
        }
    }
}

// Interactive mode
void interactiveMode() {
    std::cout << "\n=== Interactive Audio Codec Mode ===" << std::endl;
    
    while (true) {
        std::cout << "\n--- Menu ---" << std::endl;
        std::cout << "1. Compress WAV file (mono)" << std::endl;
        std::cout << "2. Compress WAV file (stereo)" << std::endl;
        std::cout << "3. Decompress and save" << std::endl;
        std::cout << "4. Test with synthetic audio" << std::endl;
        std::cout << "0. Exit" << std::endl;
        std::cout << "Choose option: ";
        
        int option;
        std::cin >> option;
        
        if (option == 0) break;
        
        switch (option) {
            case 1: {
                std::string filename;
                std::cout << "Enter WAV filename: ";
                std::cin >> filename;
                
                WAVFile wav;
                std::vector<int16_t> samples;
                AudioInfo info;
                
                if (wav.readMono(filename, samples, info)) {
                    wav.printInfo();
                    
                    int m;
                    std::cout << "Enter Golomb parameter (e.g., 16): ";
                    std::cin >> m;
                    
                    char adaptive;
                    std::cout << "Use adaptive mode? (y/n): ";
                    std::cin >> adaptive;
                    
                    AudioCodec codec(m, adaptive == 'y' || adaptive == 'Y');
                    CompressedAudio compressed = codec.encode(samples, info);
                    codec.printStatistics(compressed);
                } else {
                    std::cout << "Error reading file!" << std::endl;
                }
                break;
            }
            case 4:
                testMonoCompression();
                break;
            default:
                std::cout << "Invalid option!" << std::endl;
        }
    }
}

int main(int argc, char* argv[]) {
    std::cout << "=== Lossless Audio Codec - Part III ===" << std::endl;
    std::cout << "Using Golomb Coding for Prediction Residuals" << std::endl;
    
    if (argc > 1 && std::string(argv[1]) == "-i") {
        interactiveMode();
    } else {
        // Run all tests
        testMonoCompression();
        testStereoCompression();
        testComplexWaveforms();
        testWAVFileIO();
        
        std::cout << "\n\nAll tests completed!" << std::endl;
        std::cout << "Run with '-i' flag for interactive mode." << std::endl;
    }
    
    return 0;
}
