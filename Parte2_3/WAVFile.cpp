#include "WAVFile.h"
#include <iostream>
#include <cstring>

WAVFile::WAVFile() {
    std::memset(&header, 0, sizeof(WAVHeader));
}

// Validate WAV header
bool WAVFile::validateHeader(const WAVHeader& hdr) {
    if (std::strncmp(hdr.riffHeader, "RIFF", 4) != 0) {
        std::cerr << "Error: Invalid RIFF header" << std::endl;
        return false;
    }
    
    if (std::strncmp(hdr.waveHeader, "WAVE", 4) != 0) {
        std::cerr << "Error: Invalid WAVE header" << std::endl;
        return false;
    }
    
    if (std::strncmp(hdr.fmtHeader, "fmt ", 4) != 0) {
        std::cerr << "Error: Invalid fmt header" << std::endl;
        return false;
    }
    
    if (hdr.audioFormat != 1) {
        std::cerr << "Error: Only PCM format (audioFormat=1) is supported" << std::endl;
        return false;
    }
    
    if (std::strncmp(hdr.dataHeader, "data", 4) != 0) {
        std::cerr << "Error: Invalid data header" << std::endl;
        return false;
    }
    
    return true;
}

// Create WAV header
void WAVFile::createHeader(uint32_t sampleRate, uint16_t channels, uint16_t bitsPerSample, uint32_t numSamples) {
    std::memcpy(header.riffHeader, "RIFF", 4);
    std::memcpy(header.waveHeader, "WAVE", 4);
    std::memcpy(header.fmtHeader, "fmt ", 4);
    std::memcpy(header.dataHeader, "data", 4);
    
    header.fmtChunkSize = 16;
    header.audioFormat = 1; // PCM
    header.numChannels = channels;
    header.sampleRate = sampleRate;
    header.bitsPerSample = bitsPerSample;
    header.blockAlign = channels * bitsPerSample / 8;
    header.byteRate = sampleRate * header.blockAlign;
    header.dataSize = numSamples * channels * bitsPerSample / 8;
    header.fileSize = 36 + header.dataSize;
}

// Read WAV file
bool WAVFile::read(const std::string& filename) {
    std::ifstream file(filename, std::ios::binary);
    if (!file.is_open()) {
        std::cerr << "Error: Cannot open file " << filename << std::endl;
        return false;
    }
    
    // Read header
    file.read(reinterpret_cast<char*>(&header), sizeof(WAVHeader));
    
    if (!validateHeader(header)) {
        file.close();
        return false;
    }
    
    // Read audio data
    uint32_t numSamples = header.dataSize / (header.bitsPerSample / 8);
    audioData.resize(numSamples);
    
    if (header.bitsPerSample == 16) {
        file.read(reinterpret_cast<char*>(audioData.data()), header.dataSize);
    } else if (header.bitsPerSample == 8) {
        // Convert 8-bit unsigned to 16-bit signed
        std::vector<uint8_t> data8(numSamples);
        file.read(reinterpret_cast<char*>(data8.data()), header.dataSize);
        for (size_t i = 0; i < numSamples; i++) {
            audioData[i] = (data8[i] - 128) * 256;
        }
    }
    
    file.close();
    
    // Fill AudioInfo
    info.sampleRate = header.sampleRate;
    info.channels = header.numChannels;
    info.bitsPerSample = header.bitsPerSample;
    info.numSamples = numSamples / header.numChannels;
    
    return true;
}

// Read mono WAV file
bool WAVFile::readMono(const std::string& filename, std::vector<int16_t>& samples, AudioInfo& outInfo) {
    if (!read(filename)) {
        return false;
    }
    
    if (header.numChannels != 1) {
        std::cerr << "Error: File is not mono" << std::endl;
        return false;
    }
    
    samples = audioData;
    outInfo = info;
    return true;
}

// Read stereo WAV file
bool WAVFile::readStereo(const std::string& filename, 
                        std::vector<int16_t>& leftChannel, 
                        std::vector<int16_t>& rightChannel, 
                        AudioInfo& outInfo) {
    if (!read(filename)) {
        return false;
    }
    
    if (header.numChannels != 2) {
        std::cerr << "Error: File is not stereo" << std::endl;
        return false;
    }
    
    // De-interleave stereo data
    size_t numSamples = audioData.size() / 2;
    leftChannel.resize(numSamples);
    rightChannel.resize(numSamples);
    
    for (size_t i = 0; i < numSamples; i++) {
        leftChannel[i] = audioData[i * 2];
        rightChannel[i] = audioData[i * 2 + 1];
    }
    
    outInfo = info;
    return true;
}

// Write WAV file
bool WAVFile::write(const std::string& filename, const std::vector<int16_t>& samples, const AudioInfo& info) {
    std::ofstream file(filename, std::ios::binary);
    if (!file.is_open()) {
        std::cerr << "Error: Cannot create file " << filename << std::endl;
        return false;
    }
    
    // Create header
    createHeader(info.sampleRate, info.channels, info.bitsPerSample, samples.size());
    
    // Write header
    file.write(reinterpret_cast<const char*>(&header), sizeof(WAVHeader));
    
    // Write audio data
    file.write(reinterpret_cast<const char*>(samples.data()), samples.size() * sizeof(int16_t));
    
    file.close();
    return true;
}

// Write stereo WAV file
bool WAVFile::writeStereo(const std::string& filename,
                         const std::vector<int16_t>& leftChannel,
                         const std::vector<int16_t>& rightChannel,
                         const AudioInfo& info) {
    if (leftChannel.size() != rightChannel.size()) {
        std::cerr << "Error: Left and right channels have different sizes" << std::endl;
        return false;
    }
    
    // Interleave stereo data
    std::vector<int16_t> interleaved(leftChannel.size() * 2);
    for (size_t i = 0; i < leftChannel.size(); i++) {
        interleaved[i * 2] = leftChannel[i];
        interleaved[i * 2 + 1] = rightChannel[i];
    }
    
    AudioInfo stereoInfo = info;
    stereoInfo.channels = 2;
    stereoInfo.numSamples = leftChannel.size();
    
    return write(filename, interleaved, stereoInfo);
}

// Getters
const std::vector<int16_t>& WAVFile::getAudioData() const {
    return audioData;
}

const AudioInfo& WAVFile::getInfo() const {
    return info;
}

uint32_t WAVFile::getSampleRate() const {
    return info.sampleRate;
}

uint16_t WAVFile::getChannels() const {
    return info.channels;
}

uint16_t WAVFile::getBitsPerSample() const {
    return info.bitsPerSample;
}

uint32_t WAVFile::getNumSamples() const {
    return info.numSamples;
}

// Print file information
void WAVFile::printInfo() const {
    std::cout << "=== WAV File Information ===" << std::endl;
    std::cout << "Sample Rate: " << info.sampleRate << " Hz" << std::endl;
    std::cout << "Channels: " << info.channels << " (" 
              << (info.channels == 1 ? "Mono" : info.channels == 2 ? "Stereo" : "Multi-channel") 
              << ")" << std::endl;
    std::cout << "Bits per Sample: " << info.bitsPerSample << std::endl;
    std::cout << "Number of Samples: " << info.numSamples << std::endl;
    std::cout << "Duration: " << getDurationSeconds() << " seconds" << std::endl;
    std::cout << "File Size: " << (audioData.size() * sizeof(int16_t) + sizeof(WAVHeader)) 
              << " bytes" << std::endl;
}

// Get duration in seconds
double WAVFile::getDurationSeconds() const {
    if (info.sampleRate == 0) return 0.0;
    return static_cast<double>(info.numSamples) / info.sampleRate;
}
