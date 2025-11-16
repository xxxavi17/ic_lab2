#ifndef WAV_FILE_H
#define WAV_FILE_H

#include "AudioCodec.h"
#include <string>
#include <vector>
#include <fstream>
#include <cstdint>

// WAV file format structures
#pragma pack(push, 1)
struct WAVHeader {
    // RIFF Header
    char riffHeader[4];      // "RIFF"
    uint32_t fileSize;       // File size - 8
    char waveHeader[4];      // "WAVE"
    
    // Format Chunk
    char fmtHeader[4];       // "fmt "
    uint32_t fmtChunkSize;   // Should be 16 for PCM
    uint16_t audioFormat;    // 1 for PCM
    uint16_t numChannels;    // 1 = Mono, 2 = Stereo
    uint32_t sampleRate;     // Sampling frequency
    uint32_t byteRate;       // sampleRate * numChannels * bitsPerSample/8
    uint16_t blockAlign;     // numChannels * bitsPerSample/8
    uint16_t bitsPerSample;  // 8, 16, 24, 32
    
    // Data Chunk
    char dataHeader[4];      // "data"
    uint32_t dataSize;       // Size of audio data
};
#pragma pack(pop)

class WAVFile {
private:
    WAVHeader header;
    std::vector<int16_t> audioData;
    AudioInfo info;
    
    bool validateHeader(const WAVHeader& hdr);
    void createHeader(uint32_t sampleRate, uint16_t channels, uint16_t bitsPerSample, uint32_t numSamples);

public:
    WAVFile();
    
    // Reading WAV files
    bool read(const std::string& filename);
    bool readMono(const std::string& filename, std::vector<int16_t>& samples, AudioInfo& outInfo);
    bool readStereo(const std::string& filename, 
                   std::vector<int16_t>& leftChannel, 
                   std::vector<int16_t>& rightChannel, 
                   AudioInfo& outInfo);
    
    // Writing WAV files
    bool write(const std::string& filename, const std::vector<int16_t>& samples, const AudioInfo& info);
    bool writeStereo(const std::string& filename,
                    const std::vector<int16_t>& leftChannel,
                    const std::vector<int16_t>& rightChannel,
                    const AudioInfo& info);
    
    // Getters
    const std::vector<int16_t>& getAudioData() const;
    const AudioInfo& getInfo() const;
    uint32_t getSampleRate() const;
    uint16_t getChannels() const;
    uint16_t getBitsPerSample() const;
    uint32_t getNumSamples() const;
    
    // Utility
    void printInfo() const;
    double getDurationSeconds() const;
};

#endif // WAV_FILE_H
