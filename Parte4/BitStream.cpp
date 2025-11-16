
#include "BitStream.h"
#include <iostream>


//abre o ficheiro em modo leitura ('r') ou escrita ('w')
BitStream::BitStream(const std::string& filename, const std::string& mode)
    : mode(mode), buffer(0), bitCount(0) {
    if (mode == "w") {
        file.open(filename, std::ios::out | std::ios::binary);
    } else if (mode == "r") {
        file.open(filename, std::ios::in | std::ios::binary);
    } else {
        throw std::runtime_error("Modo inválido para BitStream (use 'r' ou 'w')");
    }
    if (!file.is_open()) {
        throw std::runtime_error("Não foi possível abrir o ficheiro: " + filename);
    }
}


//garante que o ficheiro é fechado corretamente
BitStream::~BitStream() {
    close();
}


//garante que o buffer é escrito
void BitStream::close() {
    if (mode == "w" && bitCount > 0) {
        flush();
    }
    if (file.is_open()) {
        file.close();
    }
}


//escreve o buffer restante no ficheiro, quando não está cheio
void BitStream::flush() {
    if (mode == "w" && bitCount > 0) {
        file.write((char*)&buffer, 1);
        buffer = 0;
        bitCount = 0;
    }
}


//escreve um bit no buffer; quando o buffer está cheio (8 bits), grava no ficheiro
void BitStream::writeBit(int bit) {
    if (mode != "w") return;
    buffer = buffer | ((bit & 1) << (7 - bitCount)); // Adiciona o bit na posição correta
    bitCount++;
    if (bitCount == 8) {
        file.write((char*)&buffer, 1);
        buffer = 0;
        bitCount = 0;
    }
}


//escreve 'n' bits de um valor inteiro, do bit mais significativo ao menos
void BitStream::writeNBits(unsigned int value, int n) {
    if (mode != "w") return;
    for (int i = n - 1; i >= 0; --i) {
        writeBit((value >> i) & 1);
    }
}


//le um bit do ficheiro e recarrega o buffer se necessario
int BitStream::readBit() {
    if (mode != "r") return -1;
    // Se o buffer está vazio, le o próximo byte
    if (bitCount == 0) {
        if (!file.read((char*)&buffer, 1)) {
            return -1;
        }
        bitCount = 8;
    }
    // Extrai o bit mais significativo
    int bit = (buffer >> 7) & 1;
    buffer <<= 1;
    bitCount--;
    return bit;
}


//le 'n' bits do ficheiro e devolve como inteiro
unsigned int BitStream::readNBits(int n) {
    if (mode != "r") return 0;
    unsigned int value = 0;
    for (int i = 0; i < n; ++i) {
        int bit = readBit();
        if (bit == -1) {
            throw std::runtime_error("Erro ao ler bits (fim de ficheiro inesperado)");
        }
        value = (value << 1) | bit;
    }
    return value;
}