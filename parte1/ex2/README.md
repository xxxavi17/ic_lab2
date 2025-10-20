# Exercício 2 - Transformações de Imagem

## Descrição
Programa em C++ usando OpenCV que implementa transformações básicas de imagem **sem usar funções prontas do OpenCV**, realizando todas as operações pixel por pixel.

## Funcionalidades Implementadas

### a) Negativo da imagem
- Inverte os valores dos pixels: `novo_valor = 255 - valor_original`
- Funciona para imagens grayscale e coloridas

### b) Espelhamento de imagem
- **Horizontal**: `novo_x = largura - 1 - x_original`
- **Vertical**: `novo_y = altura - 1 - y_original`

### c) Rotação por múltiplos de 90°
- **90° horário**: `(x,y) → (y, altura-1-x)`
- **180°**: Aplicação dupla de 90°
- **270°**: Aplicação tripla de 90°

### d) Ajuste de intensidade (brilho)
- **Aumentar**: `novo_valor = valor_original × 1.5`
- **Diminuir**: `novo_valor = valor_original × 0.5`
- **Customizado**: Fator definido pelo usuário
- Valores limitados entre 0-255

## Arquivos
- `image_transforms.cpp` - Código fonte principal
- `image_transforms` - Executável compilado

## Compilação
```bash
g++ -o image_transforms image_transforms.cpp `pkg-config --cflags --libs opencv4`
```

## Uso
```bash
./image_transforms
```

### Menu Interativo
1. Criar negativo da imagem
2. Espelhar horizontalmente  
3. Espelhar verticalmente
4. Rotacionar 90° (horário)
5. Rotacionar 180°
6. Rotacionar 270° (ou -90°)
7. Aumentar brilho (+50%)
8. Diminuir brilho (-50%)
9. Ajuste de brilho customizado
0. Sair

### Exemplo de Uso
```bash
./image_transforms
# Caminho da imagem: airplane.ppm
# Opção: 1 (negativo)
# Resultado: airplane_negative.png
```

## Características Técnicas
- **Implementação pixel por pixel** (sem funções prontas do OpenCV)
- **Suporte para imagens grayscale e coloridas**
- **Busca automática de arquivos** em diretórios relativos
- **Nomes de saída automáticos** com sufixos descritivos
- **Validação de valores** (0-255 para intensidade)
- **Saída em PNG** para compatibilidade com editores

## Exemplos de Arquivos Gerados
- `airplane_negative.png` - Versão negativa
- `airplane_mirror_h.png` - Espelhamento horizontal
- `airplane_rot90.png` - Rotação 90°
- `airplane_bright.png` - Brilho aumentado
- `airplane_custom.png` - Ajuste customizado