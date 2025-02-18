# Conversor A/D com joy stick 

Este projeto utiliza um joystick analógico para controlar a intensidade de LEDs RGB e movimentar um quadrado em um display SSD1306. O eixo X do joystick ajusta o brilho do LED vermelho, enquanto o eixo Y controla o LED azul. O botão do joystick alterna o estado do LED verde e muda o estilo da borda do display. O botão A liga/desliga os LEDs. O display exibe um quadrado que se move conforme a posição do joystick, e todas as interações com os botões são tratadas por interrupções, garantindo respostas rápidas e precisas. O código é organizado e utiliza PWM para controle suave dos LEDs.

## Hardware 🛠️

- Microcontrolador RP2040 (Raspberry Pi Pico).
- display SSD1306.
- joystick.
- LED RGB.
- Placa BitdogLab

## Software 💻

* **SDK do Raspberry Pi Pico:** O SDK (Software Development Kit) do Pico, que inclui as bibliotecas e ferramentas necessárias para desenvolver e compilar o código. [Instruções de instalação](https://www.raspberrypi.com/documentation/pico/getting-started/)
* **CMake:** Um sistema de construção multiplataforma usado para gerar os arquivos de construção do projeto.
* **Compilador C/C++:**  Um compilador C/C++ como o GCC (GNU Compiler Collection).
* **Git:** (Opcional) Para clonar o repositório do projeto.


### O código está dividido em vários arquivos para melhor organização:

- **`conversor.c`**: Código princiapl do projeto
- **`CMakeLists.txt`:** Define a estrutura do projeto para o CMake.



## Como Compilar e Executar ⚙️

1. **Instale o SDK do Raspberry Pi Pico:** Siga as instruções no site oficial do Raspberry Pi.
2. **Clone este repositório:** `https://github.com/wesiley/Joystick_CEPEDI`
3. **Navegue até o diretório do projeto:** `cd Joystick_CEPEDI`
4. **Compile o projeto:** `cmake -B build && cmake --build build`
5. **Copie para o Pico:** Copie o conteúdo da pasta `build` (gerada após a compilação) para o Raspberry Pi Pico. O programa iniciará automaticamente.
  
 ## 🔗 Link do Vídeo de Funcionamento:
https://drive.google.com/file/d/1almaaqFLhS2ieP1PaETdjbDiqO_wCpRl/view?usp=sharing

 ## Contribuições 🤝

Contribuições são bem-vindas! Sinta-se à vontade para abrir issues ou enviar pull requests.
