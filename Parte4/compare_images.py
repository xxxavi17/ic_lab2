import cv2
import numpy as np

# Caminhos das imagens
img1_path = '../parte1/imagens/airplane.ppm'
img2_path = './airplane_ppm_decoded.png'

img1 = cv2.imread(img1_path, cv2.IMREAD_GRAYSCALE)
img2 = cv2.imread(img2_path, cv2.IMREAD_GRAYSCALE)

if img1 is None or img2 is None:
    print('Erro ao carregar as imagens.')
    exit(1)

if img1.shape != img2.shape:
    print(f'Dimensões diferentes: {img1.shape} vs {img2.shape}')
    exit(1)

diff = img1 != img2
num_diff = np.sum(diff)

if num_diff == 0:
    print('As imagens são idênticas (lossless).')
else:
    print(f'As imagens diferem em {num_diff} pixels.')
    # Opcional: mostrar os primeiros pixels diferentes
    coords = np.argwhere(diff)
    print('Exemplos de diferenças (até 10):')
    for i in range(min(10, len(coords))):
        r, c = coords[i]
        print(f'Pixel ({r},{c}): original={img1[r,c]}, reconstruída={img2[r,c]}')
