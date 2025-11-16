// Parte4/main.cpp
#include <iostream>
#include <string>
#include "ImageCodec.h"

void printUsage() {
    std::cout << "Uso:" << std::endl;
    std::cout << "  ./image_codec encode <input_image> <output_file> <predictor_type>" << std::endl;
    std::cout << "  ./image_codec decode <input_file> <output_image>" << std::endl;
    std::cout << "\nTipos de Preditores:" << std::endl;
    std::cout << "  1: Predit_A (esquerda)" << std::endl;
    std::cout << "  2: Predit_B (cima)" << std::endl;
    std::cout << "  3: Média(A, B)" << std::endl;
    std::cout << "  4: Linear (A + B - C)" << std::endl;
    std::cout << "  5: JPEG-LS Simples (adaptativo)" << std::endl;
}

int main(int argc, char* argv[]) {
    if (argc < 4) {
        printUsage();
        return 1;
    }

    std::string mode = argv[1];
    std::string inputFile = argv[2];
    std::string outputFile = argv[3];

    ImageCodec codec;

    try {
        if (mode == "encode") {
            if (argc < 5) {
                std::cerr << "Erro: Modo 'encode' requer um tipo de preditor." << std::endl;
                printUsage();
                return 1;
            }
            int predTypeInt = std::stoi(argv[4]);
            if (predTypeInt < 1 || predTypeInt > 5) {
                std::cerr << "Erro: Tipo de preditor inválido." << std::endl;
                printUsage();
                return 1;
            }
            PredictorType predType = (PredictorType)predTypeInt;
            
            std::cout << "Modo: Codificar" << std::endl;
            codec.encode(inputFile, outputFile, predType);

        } else if (mode == "decode") {
            std::cout << "Modo: Descodificar" << std::endl;
            codec.decode(inputFile, outputFile);

        } else {
            std::cerr << "Erro: Modo inválido '" << mode << "'." << std::endl;
            printUsage();
            return 1;
        }
    } catch (const std::exception& e) {
        std::cerr << "Ocorreu um erro: " << e.what() << std::endl;
        return 1;
    }

    return 0;
}