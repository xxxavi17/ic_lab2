# Parte I - Extração de Canais de Cor

## Descrição
Programa em C++ usando OpenCV para extrair canais de cor de imagens, lendo e escrevendo pixel por pixel.

## Arquivos
- `extract_channel.cpp` - Código fonte principal
- `extract_channel` - Executável compilado
- `convert_to_png.sh` - Script para conversão para PNG (visualização no VS Code)
- `imagens/airplane.ppm` - Imagem de exemplo para testes

## Compilação
```bash
g++ -o extract_channel extract_channel.cpp `pkg-config --cflags --libs opencv4`
```

## Uso

### Modo linha de comando (compatível com requisitos)
```bash
./extract_channel <imagem_entrada> <imagem_saida> <canal>
```
Onde canal: 0 (azul), 1 (verde), 2 (vermelho)

Exemplo:
```bash
./extract_channel imagens/airplane.ppm canal_verde.pgm 1
```

### Modo interativo (menu)
```bash
./extract_channel
```
- Menu com 6 opções de extração
- Suporte para formatos PGM, PPM e PNG
- Busca automática de arquivos
- Correção automática de extensões

### Conversão para PNG (visualização no VS Code)
```bash
./convert_to_png.sh arquivo.pgm
```

## Funcionalidades
- Extração pixel por pixel conforme especificado
- Modo grayscale (canal único em PGM)
- Modo colorido (imagem com apenas um canal ativo em PPM)
- Suporte para PNG (compatibilidade com editores)
- Interface de menu interativa
- Busca automática de arquivos em diretórios
- Validação de entrada e tratamento de erros