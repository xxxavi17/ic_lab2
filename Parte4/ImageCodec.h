#ifndef IMAGECODEC_H
#define IMAGECODEC_H

#include <string>
#include <opencv2/opencv.hpp>
#include "GolombCoding.h"
#include "BitStream.h"

enum PredictorType {
    PRED_A = 1,
    PRED_B = 2,
    PRED_MEAN_AB = 3,
    PRED_LINEAR = 4,
    PRED_JPEG_LS_SIMPLE = 5
};

class ImageCodec {
public:
    ImageCodec();
    void encode(const std::string& inputFile, const std::string& outputFile, PredictorType predType);
    void decode(const std::string& inputFile, const std::string& outputFile);

private:
    int getPrediction(const cv::Mat& image, int r, int c, PredictorType predType);
    int estimateOptimalM(const cv::Mat& image, PredictorType predType);
};

#endif