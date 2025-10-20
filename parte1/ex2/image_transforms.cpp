#include <opencv2/opencv.hpp>
#include <iostream>
#include <string>
#include <limits>

using namespace cv;
using namespace std;

// Função para criar negativo da imagem (pixel por pixel)
Mat createNegative(const Mat& image) {
    Mat negative(image.rows, image.cols, image.type());
    
    if (image.channels() == 1) {
        // Imagem grayscale
        for (int i = 0; i < image.rows; i++) {
            for (int j = 0; j < image.cols; j++) {
                negative.at<uchar>(i, j) = 255 - image.at<uchar>(i, j);
            }
        }
    } else if (image.channels() == 3) {
        // Imagem colorida
        for (int i = 0; i < image.rows; i++) {
            for (int j = 0; j < image.cols; j++) {
                Vec3b pixel = image.at<Vec3b>(i, j);
                negative.at<Vec3b>(i, j) = Vec3b(255 - pixel[0], 255 - pixel[1], 255 - pixel[2]);
            }
        }
    }
    
    return negative;
}

// Função para espelhar horizontalmente
Mat mirrorHorizontal(const Mat& image) {
    Mat mirrored(image.rows, image.cols, image.type());
    
    if (image.channels() == 1) {
        for (int i = 0; i < image.rows; i++) {
            for (int j = 0; j < image.cols; j++) {
                mirrored.at<uchar>(i, j) = image.at<uchar>(i, image.cols - 1 - j);
            }
        }
    } else if (image.channels() == 3) {
        for (int i = 0; i < image.rows; i++) {
            for (int j = 0; j < image.cols; j++) {
                mirrored.at<Vec3b>(i, j) = image.at<Vec3b>(i, image.cols - 1 - j);
            }
        }
    }
    
    return mirrored;
}

// Função para espelhar verticalmente
Mat mirrorVertical(const Mat& image) {
    Mat mirrored(image.rows, image.cols, image.type());
    
    if (image.channels() == 1) {
        for (int i = 0; i < image.rows; i++) {
            for (int j = 0; j < image.cols; j++) {
                mirrored.at<uchar>(i, j) = image.at<uchar>(image.rows - 1 - i, j);
            }
        }
    } else if (image.channels() == 3) {
        for (int i = 0; i < image.rows; i++) {
            for (int j = 0; j < image.cols; j++) {
                mirrored.at<Vec3b>(i, j) = image.at<Vec3b>(image.rows - 1 - i, j);
            }
        }
    }
    
    return mirrored;
}

// Função para rotacionar 90° no sentido horário
Mat rotate90Clockwise(const Mat& image) {
    Mat rotated(image.cols, image.rows, image.type());
    
    if (image.channels() == 1) {
        for (int i = 0; i < image.rows; i++) {
            for (int j = 0; j < image.cols; j++) {
                rotated.at<uchar>(j, image.rows - 1 - i) = image.at<uchar>(i, j);
            }
        }
    } else if (image.channels() == 3) {
        for (int i = 0; i < image.rows; i++) {
            for (int j = 0; j < image.cols; j++) {
                rotated.at<Vec3b>(j, image.rows - 1 - i) = image.at<Vec3b>(i, j);
            }
        }
    }
    
    return rotated;
}

// Função para rotacionar por múltiplos de 90°
Mat rotateByAngle(const Mat& image, int angle) {
    Mat result = image.clone();
    int times = (angle / 90) % 4;
    
    for (int i = 0; i < times; i++) {
        result = rotate90Clockwise(result);
    }
    
    return result;
}

// Função para ajustar intensidade (brilho)
Mat adjustIntensity(const Mat& image, double factor) {
    Mat adjusted(image.rows, image.cols, image.type());
    
    if (image.channels() == 1) {
        for (int i = 0; i < image.rows; i++) {
            for (int j = 0; j < image.cols; j++) {
                int newValue = (int)(image.at<uchar>(i, j) * factor);
                adjusted.at<uchar>(i, j) = (uchar)max(0, min(255, newValue));
            }
        }
    } else if (image.channels() == 3) {
        for (int i = 0; i < image.rows; i++) {
            for (int j = 0; j < image.cols; j++) {
                Vec3b pixel = image.at<Vec3b>(i, j);
                int b = (int)(pixel[0] * factor);
                int g = (int)(pixel[1] * factor);
                int r = (int)(pixel[2] * factor);
                adjusted.at<Vec3b>(i, j) = Vec3b(
                    (uchar)max(0, min(255, b)),
                    (uchar)max(0, min(255, g)),
                    (uchar)max(0, min(255, r))
                );
            }
        }
    }
    
    return adjusted;
}

// Função para encontrar arquivo de imagem
string findImageFile(const string& inputPath) {
    Mat test = imread(inputPath, IMREAD_COLOR);
    if (!test.empty()) return inputPath;
    
    vector<string> possiblePaths = {
        inputPath,
        "../imagens/" + inputPath,
        "../../imagens/" + inputPath,
        "../" + inputPath
    };
    
    for (const string& path : possiblePaths) {
        test = imread(path, IMREAD_COLOR);
        if (!test.empty()) return path;
    }
    
    return "";
}

void showMenu() {
    cout << "\n=== TRANSFORMAÇÕES DE IMAGEM ===\n";
    cout << "Escolha a operação:\n";
    cout << " 1) Criar negativo da imagem\n";
    cout << " 2) Espelhar horizontalmente\n";
    cout << " 3) Espelhar verticalmente\n";
    cout << " 4) Rotacionar 90° (horário)\n";
    cout << " 5) Rotacionar 180°\n";
    cout << " 6) Rotacionar 270° (ou -90°)\n";
    cout << " 7) Aumentar brilho (+50%)\n";
    cout << " 8) Diminuir brilho (-50%)\n";
    cout << " 9) Ajuste de brilho customizado\n";
    cout << " 0) Sair\n";
    cout << "Opção: ";
}

int main() {
    string inputFile;
    int option = -1;
    
    cout << "=== TRANSFORMAÇÕES DE IMAGEM ===\n";
    cout << "Caminho da imagem de entrada: ";
    getline(cin, inputFile);
    
    if (inputFile.empty()) {
        cout << "Nenhum arquivo especificado. Saindo.\n";
        return 0;
    }
    
    // Procurar o arquivo
    string actualPath = findImageFile(inputFile);
    if (actualPath.empty()) {
        cout << "Erro: Não foi possível encontrar a imagem '" << inputFile << "'.\n";
        return -1;
    }
    
    Mat image = imread(actualPath, IMREAD_COLOR);
    if (image.empty()) {
        cout << "Erro: Arquivo encontrado mas não é uma imagem válida.\n";
        return -1;
    }
    
    cout << "Imagem carregada com sucesso: " << actualPath << endl;
    cout << "Dimensões: " << image.cols << "x" << image.rows << " pixels\n";
    
    showMenu();
    cin >> option;
    cin.ignore(numeric_limits<streamsize>::max(), '\n');
    
    if (option == 0) {
        cout << "Saindo.\n";
        return 0;
    }
    
    if (option < 1 || option > 9) {
        cout << "Opção inválida.\n";
        return -1;
    }
    
    // Gerar nome do arquivo de saída automaticamente
    size_t lastSlash = actualPath.find_last_of("/\\");
    string fileName = (lastSlash == string::npos) ? actualPath : actualPath.substr(lastSlash + 1);
    size_t lastDot = fileName.find_last_of(".");
    string baseName = (lastDot == string::npos) ? fileName : fileName.substr(0, lastDot);
    
    string suffix = "";
    Mat result;
    
    switch (option) {
        case 1:
            result = createNegative(image);
            suffix = "_negative";
            break;
        case 2:
            result = mirrorHorizontal(image);
            suffix = "_mirror_h";
            break;
        case 3:
            result = mirrorVertical(image);
            suffix = "_mirror_v";
            break;
        case 4:
            result = rotateByAngle(image, 90);
            suffix = "_rot90";
            break;
        case 5:
            result = rotateByAngle(image, 180);
            suffix = "_rot180";
            break;
        case 6:
            result = rotateByAngle(image, 270);
            suffix = "_rot270";
            break;
        case 7:
            result = adjustIntensity(image, 1.5);
            suffix = "_bright";
            break;
        case 8:
            result = adjustIntensity(image, 0.5);
            suffix = "_dark";
            break;
        case 9:
            {
                double factor;
                cout << "Digite o fator de ajuste (ex: 1.5 para +50%, 0.5 para -50%): ";
                cin >> factor;
                result = adjustIntensity(image, factor);
                suffix = "_custom";
            }
            break;
    }
    
    string outputFile = baseName + suffix + ".png";
    
    if (!imwrite(outputFile, result)) {
        cout << "Erro: Não foi possível salvar a imagem " << outputFile << endl;
        return -1;
    }
    
    cout << "✓ Transformação aplicada com sucesso!\n";
    cout << "Arquivo salvo: " << outputFile << "\n";
    cout << "Dimensões da saída: " << result.cols << "x" << result.rows << " pixels\n";
    cout << "Dica: Use 'xdg-open " << outputFile << "' para visualizar\n";
    
    return 0;
}