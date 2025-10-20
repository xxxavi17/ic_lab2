#!/bin/bash
# Script para converter imagens PGM/PPM para PNG usando ImageMagick
# Uso: ./convert_to_png.sh <arquivo_entrada> [arquivo_saida]

if [ $# -eq 0 ]; then
    echo "Uso: ./convert_to_png.sh <arquivo_entrada> [arquivo_saida]"
    echo ""
    echo "Exemplos:"
    echo "  ./convert_to_png.sh airplanered.pgm"
    echo "  ./convert_to_png.sh airplanered.pgm airplane_red.png"
    echo "  ./convert_to_png.sh *.pgm  # converte todos os PGM"
    exit 1
fi

INPUT="$1"
OUTPUT="$2"

# Se não especificou arquivo de saída, gerar automaticamente
if [ -z "$OUTPUT" ]; then
    # Remove extensão e adiciona .png
    OUTPUT="${INPUT%.*}.png"
fi

# Verificar se o arquivo de entrada existe
if [ ! -f "$INPUT" ]; then
    echo "Erro: Arquivo '$INPUT' não encontrado."
    exit 1
fi

# Converter usando ImageMagick
echo "Convertendo: $INPUT → $OUTPUT"
if convert "$INPUT" "$OUTPUT"; then
    echo "✓ Conversão concluída com sucesso!"
    echo "Arquivo salvo: $OUTPUT"
    echo "Para visualizar no VS Code: code $OUTPUT"
else
    echo "✗ Erro na conversão."
    exit 1
fi