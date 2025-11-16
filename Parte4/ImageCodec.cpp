// Parte4/ImageCodec.cpp
#include "ImageCodec.h"
#include <iostream>
#include <cmath>
#include <numeric>

ImageCodec::ImageCodec() {}

int ImageCodec::estimateOptimalM(const cv::Mat& image, PredictorType predType) {
    long long sumOfAbsResiduals = 0;
    int count = 0;
    for (int r = 0; r < image.rows; ++r) {
        for (int c = 0; c < image.cols; ++c) {
            int prediction = getPrediction(image, r, c, predType);
            int residual = (int)image.at<uchar>(r, c) - prediction;
            if (r > 0 && c > 0) {
                sumOfAbsResiduals += std::abs(residual);
                count++;
            }
        }
    }
    if (count == 0) return 1;
    double E_abs_res = (double)sumOfAbsResiduals / count;
    int m = (int)std::ceil(E_abs_res * 0.693);
    return (m < 1) ? 1 : m;
}

int ImageCodec::getPrediction(const cv::Mat& image, int r, int c, PredictorType predType) {
    int A = (c > 0) ? (int)image.at<uchar>(r, c - 1) : 0;
    int B = (r > 0) ? (int)image.at<uchar>(r - 1, c) : 0;
    int C = (r > 0 && c > 0) ? (int)image.at<uchar>(r - 1, c - 1) : 0;

    if (r == 0 && c == 0) return 128;
    if (r == 0) return A;
    if (c == 0) return B;

    switch (predType) {
        case PRED_A: return A;
        case PRED_B: return B;
        case PRED_MEAN_AB: return (A + B) / 2;
        case PRED_LINEAR: return A + B - C;
        case PRED_JPEG_LS_SIMPLE:
            if (C >= std::max(A, B)) return std::min(A, B);
            if (C <= std::min(A, B)) return std::max(A, B);
            return A + B - C;
        default: return A;
    }
}

void ImageCodec::encode(const std::string& inputFile, const std::string& outputFile, PredictorType predType) {
    cv::Mat image = cv::imread(inputFile, cv::IMREAD_GRAYSCALE);
    if (image.empty()) {
        std::cerr << "Erro: Não foi possível ler a imagem " << inputFile << std::endl;
        return;
    }

    int m = estimateOptimalM(image, predType);
    std::cout << "A codificar com m = " << m << std::endl;

    BitStream bs(outputFile, "w");
    GolombCoding golomb(m);

    bs.writeNBits(image.rows, 32);
    bs.writeNBits(image.cols, 32);
    bs.writeNBits(m, 16); 
    bs.writeNBits((int)predType, 8);

    // Matriz para guardar resíduos normalizados para visualização
    cv::Mat residual_img = cv::Mat::zeros(image.rows, image.cols, CV_8U);

    for (int r = 0; r < image.rows; ++r) {
        for (int c = 0; c < image.cols; ++c) {
            int prediction = getPrediction(image, r, c, predType);
            int residual = (int)image.at<uchar>(r, c) - prediction;

            // Salva o resíduo normalizado para visualização
            int norm_residual = residual + 128;
            if (norm_residual < 0) norm_residual = 0;
            if (norm_residual > 255) norm_residual = 255;
            residual_img.at<uchar>(r, c) = (uchar)norm_residual;

            std::vector<bool> bits = golomb.encodeInterleaving(residual);
            for (bool bit : bits) {
                bs.writeBit(bit ? 1 : 0);
            }
        }
    }
    bs.close();

    // Salva a imagem dos resíduos para análise visual
    std::string residual_path = outputFile + "_residual.png";
    cv::imwrite(residual_path, residual_img);
    std::cout << "Codificação concluída. Imagem de resíduos salva em " << residual_path << std::endl;
}

void ImageCodec::decode(const std::string& inputFile, const std::string& outputFile) {
    BitStream bs(inputFile, "r");

    int rows = bs.readNBits(32);
    int cols = bs.readNBits(32);
    int m = bs.readNBits(16);
    PredictorType predType = (PredictorType)bs.readNBits(8);

    if (m == 0) {
         std::cerr << "Erro: 'm' lido é 0." << std::endl;
         return;
    }

    std::cout << "A descodificar imagem " << rows << "x" << cols 
              << " (m=" << m << ", pred=" << predType << ")" << std::endl;

    GolombCoding golomb(m); 
    cv::Mat outImage = cv::Mat::zeros(rows, cols, CV_8U);

    int b = (m == 1) ? 0 : (int)std::ceil(std::log2(m));

    for (int r = 0; r < rows; ++r) {
        for (int c = 0; c < cols; ++c) {
            int prediction = getPrediction(outImage, r, c, predType);

            std::vector<bool> bits_lidos;
            
            // --- INÍCIO DA CORREÇÃO ---
            // 1. Ler parte unária (quociente)
            int bit = bs.readBit();
            while (bit == 0) {
                bits_lidos.push_back(false);
                bit = bs.readBit();
                if (bit == -1) throw std::runtime_error("Invalid Golomb code: no terminator for unary code");
            }
            // Terminador unário
            bits_lidos.push_back(true);

            // 2. Ler parte binária (resto)
            if (m > 1) {
                int k = b;
                unsigned int power_of_2_b = 1 << k;
                unsigned int cutoff = power_of_2_b - m;

                unsigned int r_temp = 0;
                for(int i=0; i < k-1; ++i) {
                    bit = bs.readBit();
                    if (bit == -1) throw std::runtime_error("Invalid Golomb code: EOF premature in remainder");
                    bits_lidos.push_back(bit == 1);
                    r_temp = (r_temp << 1) | bit;
                }
                
                if (r_temp >= cutoff) {
                    bit = bs.readBit();
                    if (bit == -1) throw std::runtime_error("Invalid Golomb code: EOF premature in remainder (cutoff)");
                    bits_lidos.push_back(bit == 1);
                }
            }
            // --- FIM DA CORREÇÃO ---

            int residual = golomb.decodeInterleaving(bits_lidos);

            // Reconstruir
            int pixelValue = prediction + residual;
            if (pixelValue < 0) pixelValue = 0;
            if (pixelValue > 255) pixelValue = 255;
            outImage.at<uchar>(r, c) = (uchar)pixelValue;
        }
    }
    bs.close();
    cv::imwrite(outputFile, outImage);
    std::cout << "Descodificação concluída. Imagem guardada em " << outputFile << std::endl;
}