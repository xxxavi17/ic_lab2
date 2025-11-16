import os
import subprocess

# Imagem de entrada
input_image = '../parte1/imagens/airplane.ppm'

# Caminho para o executável
codec_exec = './build/image_codec'

# Gerar arquivos comprimidos para cada preditor
tamanhos = {}
for pred in range(1, 6):
    out_file = f'../pred_{pred}.compressed'
    print(f'Codificando com preditor {pred}...')
    subprocess.run([codec_exec, 'encode', input_image, out_file, str(pred)], check=True)
    size = os.path.getsize(out_file)
    tamanhos[pred] = size
    print(f'Tamanho de pred_{pred}.compressed: {size} bytes')

# Mostrar tamanhos dos arquivos
print('\nResumo dos tamanhos:')
for pred, size in tamanhos.items():
    print(f'Preditor {pred}: {size} bytes')

melhor = min(tamanhos, key=tamanhos.get)
print(f'\nMelhor preditor para esta imagem: {melhor} (arquivo menor)')

# Tamanho da imagem original
orig_size = os.path.getsize(input_image)
print(f'\nTamanho da imagem original: {orig_size} bytes')
