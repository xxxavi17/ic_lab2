#include "AudioCodec.h"
#include <cmath>
#include <iostream>
#include <algorithm>
#include <numeric>

// Constructor
AudioCodec::AudioCodec(int golombParam, bool adaptive) 
    : golomb(golombParam), defaultGolombParameter(golombParam), adaptiveMode(adaptive) {
}

// Temporal prediction (order 1 by default)
int16_t AudioCodec::predictTemporal(const std::vector<int16_t>& samples, size_t index, int order) {
    if (index == 0) {
        return 0; // No prediction for first sample
    }
    
    if (order == 1) {
        // First-order prediction: predict current sample as previous sample
        return samples[index - 1];
    } else if (order == 2 && index >= 2) {
        // Second-order prediction: linear extrapolation
        return 2 * samples[index - 1] - samples[index - 2];
    } else if (order == 3 && index >= 3) {
        // Third-order prediction
        return 3 * samples[index - 1] - 3 * samples[index - 2] + samples[index - 3];
    }
    
    return samples[index - 1]; // Fallback to order 1
}

// Inter-channel prediction for stereo
int16_t AudioCodec::predictInterChannel(const std::vector<int16_t>& leftChannel, 
                                        const std::vector<int16_t>& rightChannel, 
                                        size_t index) {
    // Predict right channel from left channel
    // Simple approach: assume right channel is similar to left
    if (index < leftChannel.size()) {
        return leftChannel[index];
    }
    return 0;
}

// Calculate optimal Golomb parameter based on residual statistics
int AudioCodec::calculateOptimalParameter(const std::vector<int>& residuals, size_t windowSize) {
    if (residuals.empty()) return defaultGolombParameter;
    
    // Calculate mean absolute value of residuals in a window
    size_t start = residuals.size() > windowSize ? residuals.size() - windowSize : 0;
    double sum = 0.0;
    int count = 0;
    
    for (size_t i = start; i < residuals.size(); i++) {
        sum += std::abs(residuals[i]);
        count++;
    }
    
    if (count == 0) return defaultGolombParameter;
    
    double mean = sum / count;
    
    // Estimate optimal m for Golomb coding
    // For geometric distribution with parameter p, optimal m ≈ -1/log2(p)
    // We estimate p from mean: mean ≈ (1-p)/p, so p ≈ 1/(1+mean)
    if (mean > 0) {
        double p = 1.0 / (1.0 + mean);
        int m = static_cast<int>(std::max(1.0, -1.0 / std::log2(p)));
        
        // Clamp to reasonable range
        m = std::max(1, std::min(m, 256));
        return m;
    }
    
    return defaultGolombParameter;
}

// Calculate residuals using temporal prediction
std::vector<int> AudioCodec::calculateResiduals(const std::vector<int16_t>& samples) {
    std::vector<int> residuals;
    residuals.reserve(samples.size());
    
    for (size_t i = 0; i < samples.size(); i++) {
        int16_t predicted = predictTemporal(samples, i, 2); // Use order 2 prediction
        int residual = samples[i] - predicted;
        residuals.push_back(residual);
    }
    
    return residuals;
}

// Reconstruct samples from residuals
std::vector<int16_t> AudioCodec::reconstructFromResiduals(const std::vector<int>& residuals, 
                                                          const std::vector<int16_t>& original, 
                                                          size_t numSamples) {
    std::vector<int16_t> reconstructed;
    reconstructed.reserve(numSamples);
    
    for (size_t i = 0; i < numSamples && i < residuals.size(); i++) {
        int16_t predicted = predictTemporal(reconstructed, i, 1);
        int16_t sample = predicted + residuals[i];
        reconstructed.push_back(sample);
    }
    
    return reconstructed;
}

// Write bits to bitstream
void AudioCodec::writeBits(std::vector<bool>& bitstream, const std::vector<bool>& bits) {
    bitstream.insert(bitstream.end(), bits.begin(), bits.end());
}

// Write integer with fixed number of bits
void AudioCodec::writeInteger(std::vector<bool>& bitstream, int value, int numBits) {
    for (int i = numBits - 1; i >= 0; i--) {
        bitstream.push_back((value >> i) & 1);
    }
}

// Read integer from bitstream
int AudioCodec::readInteger(const std::vector<bool>& bitstream, size_t& pos, int numBits) {
    int value = 0;
    for (int i = 0; i < numBits && pos < bitstream.size(); i++) {
        value = (value << 1) | (bitstream[pos++] ? 1 : 0);
    }
    return value;
}

// Main encoding function (mono or interleaved stereo)
CompressedAudio AudioCodec::encode(const std::vector<int16_t>& audioData, const AudioInfo& info) {
    CompressedAudio compressed;
    compressed.info = info;
    compressed.useAdaptiveParameter = false; // Disable adaptive for now
    compressed.originalSize = audioData.size() * sizeof(int16_t) * 8; // in bits
    
    // Calculate residuals
    std::vector<int> residuals = calculateResiduals(audioData);
    
    // Write header information to bitstream
    writeInteger(compressed.data, info.sampleRate, 32);
    writeInteger(compressed.data, info.channels, 16);
    writeInteger(compressed.data, info.bitsPerSample, 16);
    writeInteger(compressed.data, info.numSamples, 32);
    writeInteger(compressed.data, defaultGolombParameter, 16);
    compressed.data.push_back(false); // Always non-adaptive
    
    // Encode residuals
    golomb.setParameter(defaultGolombParameter);
    
    for (size_t i = 0; i < residuals.size(); i++) {
        // Encode residual using interleaving method (handles negative values)
        std::vector<bool> encoded = golomb.encodeInterleaving(residuals[i]);
        writeBits(compressed.data, encoded);
    }
    
    compressed.compressedSize = compressed.data.size();
    compressed.compressionRatio = static_cast<double>(compressed.originalSize) / compressed.compressedSize;
    compressed.golombParameter = defaultGolombParameter;
    
    return compressed;
}

// Main decoding function
std::vector<int16_t> AudioCodec::decode(const CompressedAudio& compressed, AudioInfo& info) {
    info = compressed.info;
    
    // Read header
    size_t pos = 0;
    uint32_t sampleRate = readInteger(compressed.data, pos, 32);
    uint16_t channels = readInteger(compressed.data, pos, 16);
    uint16_t bitsPerSample = readInteger(compressed.data, pos, 16);
    uint32_t numSamples = readInteger(compressed.data, pos, 32);
    int golombParam = readInteger(compressed.data, pos, 16);
    bool adaptive = compressed.data[pos++];
    
    info.sampleRate = sampleRate;
    info.channels = channels;
    info.bitsPerSample = bitsPerSample;
    info.numSamples = numSamples;
    
    // Decode residuals
    std::vector<int> residuals;
    residuals.reserve(numSamples);
    
    golomb.setParameter(golombParam);
    golomb.setParameter(golombParam);
    
    size_t sampleCount = 0;
    while (sampleCount < numSamples && pos < compressed.data.size()) {
        // Decode residual
        std::vector<bool> encodedBits;
        
        // Read unary part: read until we hit a '1'
        while (pos < compressed.data.size() && !compressed.data[pos]) {
            encodedBits.push_back(false);
            pos++;
        }
        if (pos < compressed.data.size()) {
            encodedBits.push_back(true); // Terminator
            pos++;
        }
        
        // Binary part: read b or b-1 bits
        int b = static_cast<int>(std::ceil(std::log2(golombParam)));
        int bitsToRead = b; // Maximum bits needed
        for (int i = 0; i < bitsToRead && pos < compressed.data.size(); i++) {
            encodedBits.push_back(compressed.data[pos++]);
        }
        
        try {
            int residual = golomb.decodeInterleaving(encodedBits);
            residuals.push_back(residual);
            sampleCount++;
        } catch (...) {
            // If decode fails, we might have read too many bits
            break;
        }
    }
    
    // Reconstruct audio samples from residuals
    std::vector<int16_t> reconstructed;
    reconstructed.reserve(numSamples);
    
    for (size_t i = 0; i < residuals.size(); i++) {
        int16_t predicted = predictTemporal(reconstructed, i, 2); // Use order 2 prediction
        int16_t sample = predicted + residuals[i];
        reconstructed.push_back(sample);
    }
    
    return reconstructed;
}

// Stereo encoding with inter-channel prediction
CompressedAudio AudioCodec::encodeStereo(const std::vector<int16_t>& leftChannel,
                                         const std::vector<int16_t>& rightChannel,
                                         const AudioInfo& info,
                                         bool useInterChannelPrediction) {
    CompressedAudio compressed;
    compressed.info = info;
    compressed.useAdaptiveParameter = adaptiveMode;
    compressed.originalSize = (leftChannel.size() + rightChannel.size()) * sizeof(int16_t) * 8;
    
    // Write header
    writeInteger(compressed.data, info.sampleRate, 32);
    writeInteger(compressed.data, info.channels, 16);
    writeInteger(compressed.data, info.bitsPerSample, 16);
    writeInteger(compressed.data, info.numSamples, 32);
    writeInteger(compressed.data, defaultGolombParameter, 16);
    compressed.data.push_back(adaptiveMode);
    compressed.data.push_back(useInterChannelPrediction);
    
    // Encode left channel (temporal prediction only)
    std::vector<int> leftResiduals = calculateResiduals(leftChannel);
    
    golomb.setParameter(defaultGolombParameter);
    for (size_t i = 0; i < leftResiduals.size(); i++) {
        std::vector<bool> encoded = golomb.encodeInterleaving(leftResiduals[i]);
        writeBits(compressed.data, encoded);
    }
    
    // Encode right channel
    std::vector<int> rightResiduals;
    rightResiduals.reserve(rightChannel.size());
    
    if (useInterChannelPrediction) {
        // Use both temporal and inter-channel prediction
        for (size_t i = 0; i < rightChannel.size(); i++) {
            int16_t temporalPred = predictTemporal(rightChannel, i, 1);
            int16_t interChannelPred = predictInterChannel(leftChannel, rightChannel, i);
            // Average both predictions
            int16_t predicted = (temporalPred + interChannelPred) / 2;
            int residual = rightChannel[i] - predicted;
            rightResiduals.push_back(residual);
        }
    } else {
        rightResiduals = calculateResiduals(rightChannel);
    }
    
    for (size_t i = 0; i < rightResiduals.size(); i++) {
        std::vector<bool> encoded = golomb.encodeInterleaving(rightResiduals[i]);
        writeBits(compressed.data, encoded);
    }
    
    compressed.compressedSize = compressed.data.size();
    compressed.compressionRatio = static_cast<double>(compressed.originalSize) / compressed.compressedSize;
    compressed.golombParameter = defaultGolombParameter;
    
    return compressed;
}

// Stereo decoding
void AudioCodec::decodeStereo(const CompressedAudio& compressed,
                              std::vector<int16_t>& leftChannel,
                              std::vector<int16_t>& rightChannel,
                              AudioInfo& info) {
    // Read header
    size_t pos = 0;
    uint32_t sampleRate = readInteger(compressed.data, pos, 32);
    uint16_t channels = readInteger(compressed.data, pos, 16);
    uint16_t bitsPerSample = readInteger(compressed.data, pos, 16);
    uint32_t numSamples = readInteger(compressed.data, pos, 32);
    int golombParam = readInteger(compressed.data, pos, 16);
    bool adaptive = compressed.data[pos++];
    bool useInterChannelPred = compressed.data[pos++];
    
    info.sampleRate = sampleRate;
    info.channels = channels;
    info.bitsPerSample = bitsPerSample;
    info.numSamples = numSamples;
    
    golomb.setParameter(golombParam);
    
    // Decode left channel
    std::vector<int> leftResiduals;
    leftResiduals.reserve(numSamples);
    
    for (size_t i = 0; i < numSamples && pos < compressed.data.size(); i++) {
        std::vector<bool> encodedBits;
        
        // Read unary part
        while (pos < compressed.data.size() && !compressed.data[pos]) {
            encodedBits.push_back(false);
            pos++;
        }
        if (pos < compressed.data.size()) {
            encodedBits.push_back(true);
            pos++;
        }
        
        // Read binary part
        int b = static_cast<int>(std::ceil(std::log2(golombParam)));
        for (int j = 0; j < b && pos < compressed.data.size(); j++) {
            encodedBits.push_back(compressed.data[pos++]);
        }
        
        try {
            int residual = golomb.decodeInterleaving(encodedBits);
            leftResiduals.push_back(residual);
        } catch (...) {
            break;
        }
    }
    
    // Reconstruct left channel
    leftChannel.clear();
    leftChannel.reserve(numSamples);
    for (size_t i = 0; i < leftResiduals.size(); i++) {
        int16_t predicted = predictTemporal(leftChannel, i, 1);
        int16_t sample = predicted + leftResiduals[i];
        leftChannel.push_back(sample);
    }
    
    // Decode right channel
    std::vector<int> rightResiduals;
    rightResiduals.reserve(numSamples);
    
    for (size_t i = 0; i < numSamples && pos < compressed.data.size(); i++) {
        std::vector<bool> encodedBits;
        
        // Read unary part
        while (pos < compressed.data.size() && !compressed.data[pos]) {
            encodedBits.push_back(false);
            pos++;
        }
        if (pos < compressed.data.size()) {
            encodedBits.push_back(true);
            pos++;
        }
        
        // Read binary part
        int b = static_cast<int>(std::ceil(std::log2(golombParam)));
        for (int j = 0; j < b && pos < compressed.data.size(); j++) {
            encodedBits.push_back(compressed.data[pos++]);
        }
        
        try {
            int residual = golomb.decodeInterleaving(encodedBits);
            rightResiduals.push_back(residual);
        } catch (...) {
            break;
        }
    }
    
    // Reconstruct right channel
    rightChannel.clear();
    rightChannel.reserve(numSamples);
    
    if (useInterChannelPred) {
        for (size_t i = 0; i < rightResiduals.size(); i++) {
            int16_t temporalPred = predictTemporal(rightChannel, i, 1);
            int16_t interChannelPred = predictInterChannel(leftChannel, rightChannel, i);
            int16_t predicted = (temporalPred + interChannelPred) / 2;
            int16_t sample = predicted + rightResiduals[i];
            rightChannel.push_back(sample);
        }
    } else {
        for (size_t i = 0; i < rightResiduals.size(); i++) {
            int16_t predicted = predictTemporal(rightChannel, i, 1);
            int16_t sample = predicted + rightResiduals[i];
            rightChannel.push_back(sample);
        }
    }
}

// Configuration methods
void AudioCodec::setGolombParameter(int param) {
    defaultGolombParameter = param;
    golomb.setParameter(param);
}

int AudioCodec::getGolombParameter() const {
    return defaultGolombParameter;
}

void AudioCodec::setAdaptiveMode(bool adaptive) {
    adaptiveMode = adaptive;
}

bool AudioCodec::isAdaptiveMode() const {
    return adaptiveMode;
}

// Get compression ratio
double AudioCodec::getCompressionRatio(const CompressedAudio& compressed) const {
    return compressed.compressionRatio;
}

// Print statistics
void AudioCodec::printStatistics(const CompressedAudio& compressed) const {
    std::cout << "=== Compression Statistics ===" << std::endl;
    std::cout << "Sample Rate: " << compressed.info.sampleRate << " Hz" << std::endl;
    std::cout << "Channels: " << compressed.info.channels << std::endl;
    std::cout << "Bits per Sample: " << compressed.info.bitsPerSample << std::endl;
    std::cout << "Number of Samples: " << compressed.info.numSamples << std::endl;
    std::cout << "Golomb Parameter: " << compressed.golombParameter << std::endl;
    std::cout << "Adaptive Mode: " << (compressed.useAdaptiveParameter ? "Yes" : "No") << std::endl;
    std::cout << "Original Size: " << compressed.originalSize << " bits (" 
              << compressed.originalSize / 8 << " bytes)" << std::endl;
    std::cout << "Compressed Size: " << compressed.compressedSize << " bits (" 
              << compressed.compressedSize / 8 << " bytes)" << std::endl;
    std::cout << "Compression Ratio: " << compressed.compressionRatio << ":1" << std::endl;
    std::cout << "Space Savings: " << (1.0 - 1.0/compressed.compressionRatio) * 100.0 << "%" << std::endl;
}
