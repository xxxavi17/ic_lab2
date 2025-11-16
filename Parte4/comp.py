# Parte4/comp.py
import cv2
import os
import sys

# 1. Define os caminhos
# O PPM original (a cores)
original_ppm_path = "../parte1/imagens/airplane.ppm" 
# O output do teu codec (que já descodificaste para PNG)
decoded_png_path = "airplane_ppm_decoded.png" 

# Onde vamos guardar a referência correta
ground_truth_path = "opencv_ground_truth.png"

# 2. Cria a imagem de referência "Ground Truth"
#    Usando *exatamente* a mesma função que o teu codec usou
print(f"A criar a referência (ground truth) a partir de {original_ppm_path}...")
ground_truth_image = cv2.imread(original_ppm_path, cv2.IMREAD_GRAYSCALE)

if ground_truth_image is None:
    print(f"Erro: Não foi possível ler a imagem original em {original_ppm_path}")
    sys.exit(1)

cv2.imwrite(ground_truth_path, ground_truth_image)
print(f"Referência guardada em {ground_truth_path}")

# 3. Verifica se o ficheiro do teu codec existe
if not os.path.exists(decoded_png_path):
    print(f"Erro: O ficheiro {decoded_png_path} não foi encontrado.")
    print("Corre o 'decode' para .png primeiro!")
    sys.exit(1)

# 4. Compara a referência com o output do teu codec
print("\n--- A Comparar ---")
print(f"Referência: {ground_truth_path}")
print(f"Codec:      {decoded_png_path}")
print("--------------------")
os.system(f"compare -metric PSNR {ground_truth_path} {decoded_png_path} NULL:")