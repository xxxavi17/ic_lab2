#include <opencv2/opencv.hpp>
#include <iostream>
#include <limits>

using namespace cv;
using namespace std;

// Função para extrair canal em modo grayscale (PGM)
int extractGrayscaleChannel(const Mat &image, int channel, const string &outputFile) {
    if (channel < 0 || channel > 2) {
        cout << "Erro: Canal deve ser 0 (azul), 1 (verde) ou 2 (vermelho)" << endl;
        return -1;
    }
    
    Mat singleChannel(image.rows, image.cols, CV_8UC1);
    
    // Extrair o canal especificado pixel por pixel
    for (int i = 0; i < image.rows; i++) {
        for (int j = 0; j < image.cols; j++) {
            Vec3b pixel = image.at<Vec3b>(i, j);
            singleChannel.at<uchar>(i, j) = pixel[channel];
        }
    }
    
    if (!imwrite(outputFile, singleChannel)) {
        cout << "Erro: Não foi possível salvar a imagem " << outputFile << endl;
        return -1;
    }
    
    return 0;
}

// Função para extrair canal em modo colorido (PPM) - outros canais ficam a zero
int extractColorSingleChannel(const Mat &image, int channel, const string &outputFile) {
    if (channel < 0 || channel > 2) {
        cout << "Erro: Canal deve ser 0 (azul), 1 (verde) ou 2 (vermelho)" << endl;
        return -1;
    }
    
    Mat colorOutput = Mat::zeros(image.size(), image.type());
    
    // Manter apenas o canal especificado
    for (int i = 0; i < image.rows; i++) {
        for (int j = 0; j < image.cols; j++) {
            Vec3b pixel = image.at<Vec3b>(i, j);
            colorOutput.at<Vec3b>(i, j)[channel] = pixel[channel];
        }
    }
    
    if (!imwrite(outputFile, colorOutput)) {
        cout << "Erro: Não foi possível salvar a imagem " << outputFile << endl;
        return -1;
    }
    
    return 0;
}

// Função auxiliar para encontrar o arquivo de imagem
string findImageFile(const string &inputPath) {
    // Primeiro tenta o caminho exato
    Mat test = imread(inputPath, IMREAD_COLOR);
    if (!test.empty()) return inputPath;
    
    // Se não encontrar, tenta caminhos relativos comuns
    vector<string> possiblePaths = {
        inputPath,
        "imagens/" + inputPath,
        "./" + inputPath,
        "../" + inputPath
    };
    
    for (const string &path : possiblePaths) {
        test = imread(path, IMREAD_COLOR);
        if (!test.empty()) return path;
    }
    
    return ""; // Não encontrado
}

void showMenu() {
    cout << "\n=== EXTRATOR DE CANAIS DE COR (AUTOMÁTICO) ===\n";
    cout << "Escolha o tipo de extração:\n";
    cout << " 1) Canal Azul (grayscale)\n";
    cout << " 2) Canal Verde (grayscale)\n";
    cout << " 3) Canal Vermelho (grayscale)\n";
    cout << " 4) Canal Azul (imagem colorida)\n";
    cout << " 5) Canal Verde (imagem colorida)\n";
    cout << " 6) Canal Vermelho (imagem colorida)\n";
    cout << " 0) Sair\n";
    cout << "Nota: Nomes dos arquivos gerados automaticamente\n";
    cout << "Opção: ";
}

int main(int argc, char** argv) {
    // Modo de linha de comando (compatibilidade)
    if (argc == 4) {
        string inputFile = argv[1];
        string outputFile = argv[2];
        int channel = atoi(argv[3]);
        
        string actualPath = findImageFile(inputFile);
        if (actualPath.empty()) {
            cout << "Erro: Não foi possível carregar a imagem " << inputFile << endl;
            return -1;
        }
        
        Mat image = imread(actualPath, IMREAD_COLOR);
        if (extractGrayscaleChannel(image, channel, outputFile) == 0) {
            cout << "Canal " << channel << " extraído e salvo em " << outputFile << endl;
        }
        return 0;
    }
    
    // Modo interativo
    string inputFile, outputFile;
    int option = -1;
    
    cout << "=== EXTRATOR DE CANAIS DE COR ===\n";
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
        cout << "Verifique se o arquivo existe e se está no diretório correto.\n";
        cout << "Dica: Se estiver na pasta parte1, use apenas 'imagens/airplane.ppm'\n";
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
    
    if (option < 1 || option > 6) {
        cout << "Opção inválida.\n";
        return -1;
    }
    
    int channel = -1;
    bool isColorMode = false;
    string channelName = "";
    
    switch (option) {
        case 1: channel = 0; channelName = "Azul"; break;
        case 2: channel = 1; channelName = "Verde"; break;
        case 3: channel = 2; channelName = "Vermelho"; break;
        case 4: channel = 0; channelName = "Azul"; isColorMode = true; break;
        case 5: channel = 1; channelName = "Verde"; isColorMode = true; break;
        case 6: channel = 2; channelName = "Vermelho"; isColorMode = true; break;
    }
    
    // Gerar nome do arquivo automaticamente
    size_t lastSlash = actualPath.find_last_of("/\\");
    string fileName = (lastSlash == string::npos) ? actualPath : actualPath.substr(lastSlash + 1);
    
    // Remover extensão
    size_t lastDot = fileName.find_last_of(".");
    string baseName = (lastDot == string::npos) ? fileName : fileName.substr(0, lastDot);
    
    // Criar sufixos para os canais
    string channelSuffix = "";
    switch (channel) {
        case 0: channelSuffix = "_blue"; break;
        case 1: channelSuffix = "_green"; break;
        case 2: channelSuffix = "_red"; break;
    }
    
    // Determinar extensão baseada no modo
    string extension = isColorMode ? ".png" : ".png";  // PNG para ambos (compatível VS Code)
    string modeSuffix = isColorMode ? "_color" : "_grayscale";
    
    // Gerar nome final
    outputFile = baseName + channelSuffix + modeSuffix + extension;
    
    cout << "\nArquivo de saída gerado automaticamente: " << outputFile << "\n";
    cout << "Extraindo canal " << channelName;
    
    if (isColorMode) {
        cout << " (modo colorido)...\n";
        if (extractColorSingleChannel(image, channel, outputFile) == 0) {
            cout << "✓ Canal " << channelName << " extraído com sucesso!\n";
            cout << "Arquivo salvo: " << outputFile << "\n";
            cout << "Dica: Use 'xdg-open " << outputFile << "' para visualizar\n";
        }
    } else {
        cout << " (modo grayscale)...\n";
        if (extractGrayscaleChannel(image, channel, outputFile) == 0) {
            cout << "✓ Canal " << channelName << " extraído com sucesso!\n";
            cout << "Arquivo salvo: " << outputFile << "\n";
            cout << "Dica: Use 'xdg-open " << outputFile << "' para visualizar\n";
        }
    }
    
    return 0;
}