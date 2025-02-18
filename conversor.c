#include <stdio.h>
#include <stdlib.h>
#include "pico/stdlib.h"
#include "hardware/adc.h"
#include "hardware/i2c.h"
#include "lib/ssd1306.h"
#include "lib/font.h"
#include "hardware/pwm.h"
#include "pico/bootrom.h"

// Definições de pinos
#define I2C_PORT i2c1
#define I2C_SDA 14 // Display
#define I2C_SCL 15 // Display
#define endereco 0x3C

#define JOYSTICK_X_PIN 26  // GPIO para eixo X
#define JOYSTICK_Y_PIN 27  // GPIO para eixo Y
#define JOYSTICK_PB 22     // GPIO para botão do Joystick
#define Botao_A 5          // GPIO para botão A

#define LED_BLUE 12        // GPIO para LED azul
#define LED_RED 13         // GPIO para LED vermelho
#define LED_GREEN 11       // GPIO para LED verde

#define WRAP 4096          // Valor de wrap para PWM
#define DIV 16             // Divisor de clock para PWM

// Variáveis globais
uint16_t led_b_level = 0, led_r_level = 0; // Níveis de PWM para os LEDs
uint slice_led_b, slice_led_r;             // Slices de PWM para os LEDs
static uint sliceRED, sliceBLUE;           // Slices de PWM para LEDs vermelho e azul
bool enableSlices = true;                  // Controle de habilitação dos slices PWM
bool leds_enabled = true;                  // Estado dos LEDs
bool cor = true;                           // Controle de cor para o display
ssd1306_t ssd;                             // Estrutura do display
uint32_t last_time = 0;                    // Tempo do último evento para debouncing
uint32_t current_time;                     // Tempo atual para debouncing
uint8_t border_style = 0;                  // Estilo atual da borda do display

// Protótipos de funções
void gpio_irq_handler(uint gpio, uint32_t events); // Handler de interrupção
void setup_pwm_led(uint led, uint *slice, uint16_t level); // Configuração de PWM para LEDs
void init_gpio_buttons(); // Inicialização dos botões e LEDs
void setup_joystick();    // Inicialização do joystick
void update_display_border(); // Atualiza a borda do display

// Função principal
int main() {
    stdio_init_all();
    init_gpio_buttons();

    // Inicialização do I2C para o display
    i2c_init(I2C_PORT, 400 * 1000);
    gpio_set_function(I2C_SDA, GPIO_FUNC_I2C);
    gpio_set_function(I2C_SCL, GPIO_FUNC_I2C);
    gpio_pull_up(I2C_SDA);
    gpio_pull_up(I2C_SCL);

    // Inicialização do display SSD1306
    ssd1306_init(&ssd, WIDTH, HEIGHT, false, endereco, I2C_PORT);
    ssd1306_config(&ssd);
    ssd1306_send_data(&ssd);
    ssd1306_fill(&ssd, false); // Limpa o display
    ssd1306_send_data(&ssd);

    // Inicialização do joystick e PWM para LEDs
    setup_joystick();
    setup_pwm_led(LED_BLUE, &slice_led_b, led_b_level);
    setup_pwm_led(LED_RED, &slice_led_r, led_r_level);

    // Configuração das interrupções para os botões
    gpio_set_irq_enabled_with_callback(JOYSTICK_PB, GPIO_IRQ_EDGE_FALL, true, &gpio_irq_handler);
    gpio_set_irq_enabled_with_callback(Botao_A, GPIO_IRQ_EDGE_FALL, true, &gpio_irq_handler);

    // Variáveis para leitura do joystick
    uint16_t adc_value_x, adc_value_y;
    char str_x[5], str_y[5]; // Buffers para strings

    while (true) {
        // Leitura dos valores do joystick
        adc_select_input(1); // Eixo Y
        adc_value_x = adc_read();
        adc_select_input(0); // Eixo X
        adc_value_y = abs(adc_read() - 4095);

        // Cálculo do brilho dos LEDs com base no joystick
        uint16_t led_brightness_x = abs((int)adc_value_x - 2048) * 2;
        uint16_t led_brightness_y = abs((int)adc_value_y - 2048) * 2;

        // Ajuste do brilho dos LEDs
        if (led_brightness_x < 400) led_brightness_x = 0;
        if (led_brightness_y < 200) led_brightness_y = 0;

        pwm_set_gpio_level(LED_RED, led_brightness_x);
        pwm_set_gpio_level(LED_BLUE, led_brightness_y);

        // Atualização do display
        ssd1306_fill(&ssd, !cor); // Limpa o display
        update_display_border(); // Atualiza a borda do display
        ssd1306_draw_string(&ssd, "+", adc_value_x / 35, adc_value_y / 75); // Desenha uma string
        ssd1306_send_data(&ssd); // Atualiza o display

        sleep_ms(100);
    }
}

// Configuração de PWM para os LEDs
void setup_pwm_led(uint led, uint *slice, uint16_t level) {
    gpio_set_function(led, GPIO_FUNC_PWM);
    *slice = pwm_gpio_to_slice_num(led);
    pwm_set_clkdiv(*slice, DIV);
    pwm_set_wrap(*slice, WRAP);
    pwm_set_enabled(*slice, true);
    pwm_set_gpio_level(led, level);

    sliceRED = pwm_gpio_to_slice_num(LED_RED);
    sliceBLUE = pwm_gpio_to_slice_num(LED_BLUE);
    gpio_set_function(LED_RED, GPIO_FUNC_PWM);
    gpio_set_function(LED_BLUE, GPIO_FUNC_PWM);
    pwm_set_clkdiv(sliceRED, DIV);
    pwm_set_wrap(sliceRED, WRAP);
    pwm_set_clkdiv(sliceBLUE, DIV);
    pwm_set_wrap(sliceBLUE, WRAP);

    pwm_set_enabled(sliceRED, true);
    pwm_set_enabled(sliceBLUE, true);
    enableSlices = true;
}

// Inicialização dos botões e LEDs
void init_gpio_buttons() {
    gpio_init(LED_GREEN);
    gpio_set_dir(LED_GREEN, GPIO_OUT);
    gpio_init(LED_BLUE);
    gpio_set_dir(LED_BLUE, GPIO_OUT);
    gpio_init(LED_RED);
    gpio_set_dir(LED_RED, GPIO_OUT);

    gpio_init(Botao_A);
    gpio_set_dir(Botao_A, GPIO_IN);
    gpio_pull_up(Botao_A);
}

// Inicialização do joystick
void setup_joystick() {
    adc_init();
    adc_gpio_init(JOYSTICK_X_PIN);
    adc_gpio_init(JOYSTICK_Y_PIN);
    gpio_init(JOYSTICK_PB);
    gpio_set_dir(JOYSTICK_PB, GPIO_IN);
    gpio_pull_up(JOYSTICK_PB);
}

// Atualiza a borda do display com base no estilo atual
void update_display_border() {
    switch (border_style) {
        case 0:
            // Estilo 0: Borda simples
            ssd1306_rect(&ssd, 3, 3, 122, 58, cor, !cor); // Borda externa
            break;
        case 1:
            // Estilo 1: Borda dupla
            ssd1306_rect(&ssd, 3, 3, 122, 58, cor, !cor); // Borda externa
            ssd1306_rect(&ssd, 6, 6, 116, 52, cor, !cor);  // Borda interna
            break;
        case 2:
            // Estilo 2: Borda tracejada (usando retângulos menores)
            ssd1306_rect(&ssd, 3, 3, 122, 58, cor, !cor); // Borda externa
            ssd1306_rect(&ssd, 6, 6, 10, 10, cor, !cor);   // Canto superior esquerdo
            ssd1306_rect(&ssd, 110, 6, 10, 10, cor, !cor); // Canto superior direito
            ssd1306_rect(&ssd, 6, 46, 10, 10, cor, !cor);  // Canto inferior esquerdo
            ssd1306_rect(&ssd, 110, 46, 10, 10, cor, !cor);// Canto inferior direito
            break;
    }
}

// Handler de interrupção para os botões
void gpio_irq_handler(uint gpio, uint32_t events) {
    current_time = to_us_since_boot(get_absolute_time());

    // Debouncing
    if (current_time - last_time > 200000) { // 200 ms de debouncing
        last_time = current_time;

        if (gpio == JOYSTICK_PB) { // Botão do joystick pressionado
            gpio_put(LED_GREEN, !gpio_get(LED_GREEN)); // Alterna o estado do LED verde
            border_style = (border_style + 1) % 3; // Alterna entre os estilos de borda
            update_display_border(); // Atualiza a borda do display
            ssd1306_send_data(&ssd); // Atualiza o display
        } else if (gpio == Botao_A) { // Botão A pressionado
            enableSlices = !enableSlices; // Alterna a habilitação dos slices PWM
            pwm_set_enabled(slice_led_b, enableSlices);
            pwm_set_enabled(slice_led_r, enableSlices);
        }
    }
}