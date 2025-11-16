#ifndef AUDIO_CODEC_H
#define AUDIO_CODEC_H

#include "GolombCoding.h"
#include <vector>
#include <string>
#include <cstdint>

// Structure to hold audio information
struct AudioInfo {
    uint32_t sampleRate;
    uint16_t channels;      // 1 = mono, 2 = stereo
    uint16_t bitsPerSample; // Typically 16
    uint32_t numSamples;
    
    AudioInfo() : sampleRate(44100), channels(1), bitsPerSample(16), numSamples(0) {}
};

// Structure for compressed audio data
struct CompressedAudio {
    AudioInfo info;
    int golombParameter;
    bool useAdaptiveParameter;
    std::vector<bool> data;
    
    // Statistics
    size_t originalSize;
    size_t compressedSize;
    double compressionRatio;
};

class AudioCodec {
private:
    GolombCoding golomb;
    int defaultGolombParameter;
    bool adaptiveMode;
    
    // Prediction methods
    int16_t predictTemporal(const std::vector<int16_t>& samples, size_t index, int order = 1);
    int16_t predictInterChannel(const std::vector<int16_t>& leftChannel, 
                                 const std::vector<int16_t>& rightChannel, 
                                 size_t index);
    
    // Adaptive parameter calculation
    int calculateOptimalParameter(const std::vector<int>& residuals, size_t windowSize = 1000);
    
    // Bit stream operations
    void writeBits(std::vector<bool>& bitstream, const std::vector<bool>& bits);
    void writeInteger(std::vector<bool>& bitstream, int value, int numBits);
    int readInteger(const std::vector<bool>& bitstream, size_t& pos, int numBits);
    
    // Encoding/decoding helpers
    std::vector<int> calculateResiduals(const std::vector<int16_t>& samples);
    std::vector<int16_t> reconstructFromResiduals(const std::vector<int>& residuals, 
                                                   const std::vector<int16_t>& original, 
                                                   size_t numSamples);

public:
    // Constructor
    explicit AudioCodec(int golombParam = 16, bool adaptive = true);
    
    // Main encoding/decoding functions
    CompressedAudio encode(const std::vector<int16_t>& audioData, const AudioInfo& info);
    std::vector<int16_t> decode(const CompressedAudio& compressed, AudioInfo& info);
    
    // Stereo-specific encoding/decoding
    CompressedAudio encodeStereo(const std::vector<int16_t>& leftChannel,
                                  const std::vector<int16_t>& rightChannel,
                                  const AudioInfo& info,
                                  bool useInterChannelPrediction = true);
    
    void decodeStereo(const CompressedAudio& compressed,
                     std::vector<int16_t>& leftChannel,
                     std::vector<int16_t>& rightChannel,
                     AudioInfo& info);
    
    // Configuration
    void setGolombParameter(int param);
    int getGolombParameter() const;
    void setAdaptiveMode(bool adaptive);
    bool isAdaptiveMode() const;
    
    // Utility functions
    double getCompressionRatio(const CompressedAudio& compressed) const;
    void printStatistics(const CompressedAudio& compressed) const;
};

#endif // AUDIO_CODEC_H
