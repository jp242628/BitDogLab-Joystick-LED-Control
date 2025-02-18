#include <stdio.h>        // Para stdio_init_all
#include "pico/stdlib.h"  // Para stdio_init_all
#include "hardware/i2c.h" // Para o barramento I2C
#include "hardware/adc.h" // Para o controle do joystick
#include "hardware/pwm.h" // Para o controle dos LEDs RGB
#include "inc/ssd1306.h"  // Para o display OLED
#include "pico/bootrom.h" // Para reset_usb_boot

#define I2C_PORT i2c1 // Barramento I2C
#define I2C_SDA 14    // Pinos para o barramento I2C (SDA)
#define I2C_SCL 15    // Pinos para o barramento I2C (SCL)
#define ENDERECO 0x3C // Endereço do display OLED

#define JOYSTICK_BUTTON 22 // Botão do joystick
#define JOYSTICK_X_PIN 26  // GPIO para eixo X horizontal
#define JOYSTICK_Y_PIN 27  // GPIO para eixo Y vertical

#define BOTAO_A 5 // GPIO para o botão A
#define BOTAO_B 6 // GPIO para o botão B

#define LED_VERMELHO 13 // GPIOs para os LEDs RGB (PWM) Vermelho
#define LED_VERDE 11    // GPIOs para os LEDs RGB (PWM) Verde
#define LED_AZUL 12     // GPIOs para os LEDs RGB (PWM) Azul

#define QUADRADO_TAM 8 // Tamanho do quadrado

// Variáveis globais para controle dos LEDs e estados
bool led_verde_estado = false; // Estado do LED verde (ligado/desligado)
bool pwm_ativo = true;         // Estado do PWM (ligado/desligado)
uint8_t borda_estilo = 0;      // 0: sem borda, 1: borda 1, 2: borda 2, 3: borda 3

// Função para debouncing
bool debounce_button(uint gpio)
{
    if (gpio_get(gpio) == 0)
    {                 // Botão pressionado
        sleep_ms(20); // Aguarda para eliminar bouncing
        if (gpio_get(gpio) == 0)
        {
            while (gpio_get(gpio) == 0)
                ; // Aguarda soltar o botão
            return true;
        }
    }
    return false; // Botão não pressionado
}

// IRQ para o botão B (modo boot)
void gpio_irq_handler(uint gpio, uint32_t events)
{
    reset_usb_boot(0, 0);
}

// Função principal
int main()
{
    stdio_init_all();

    // Inicialização do I2C e do Display
    i2c_init(I2C_PORT, 400 * 1000);            // Inicializa o barramento I2C com 400kHz
    gpio_set_function(I2C_SDA, GPIO_FUNC_I2C); // Define como função I2C
    gpio_set_function(I2C_SCL, GPIO_FUNC_I2C); // Define como função I2C
    gpio_pull_up(I2C_SDA);                     // Habilita pull-up interno
    gpio_pull_up(I2C_SCL);                     // Habilita pull-up interno

    ssd1306_t ssd;                                                // Estrutura para o display
    ssd1306_init(&ssd, WIDTH, HEIGHT, false, ENDERECO, I2C_PORT); // Inicializa o display
    ssd1306_config(&ssd);                                         // Configura o display
    ssd1306_send_data(&ssd);                                      // Envia os dados para o display

    // Inicializa ADC
    adc_init();
    adc_gpio_init(JOYSTICK_X_PIN); // Inicializa os pinos do joystick
    adc_gpio_init(JOYSTICK_Y_PIN); // Inicializa os pinos do joystick

    // Configuração do botão B (modo boot)
    gpio_init(BOTAO_B);                                                                       // Inicializa o botão B
    gpio_set_dir(BOTAO_B, GPIO_IN);                                                           // Define como entrada
    gpio_pull_up(BOTAO_B);                                                                    // Habilita pull-up interno
    gpio_set_irq_enabled_with_callback(BOTAO_B, GPIO_IRQ_EDGE_FALL, true, &gpio_irq_handler); // Habilita IRQ para borda de descida

    // Configuração dos botões A e Joystick
    gpio_init(BOTAO_A);             // Inicializa o botão A
    gpio_set_dir(BOTAO_A, GPIO_IN); // Define como entrada
    gpio_pull_up(BOTAO_A);          // Habilita pull-up interno

    gpio_init(JOYSTICK_BUTTON);             // Inicializa o botão do joystick
    gpio_set_dir(JOYSTICK_BUTTON, GPIO_IN); // Define como entrada
    gpio_pull_up(JOYSTICK_BUTTON);          // Habilita pull-up interno

    // Configuração dos LEDs RGB com PWM
    gpio_set_function(LED_VERMELHO, GPIO_FUNC_PWM); // Define como PWM
    gpio_set_function(LED_AZUL, GPIO_FUNC_PWM);     // Define como PWM
    gpio_set_function(LED_VERDE, GPIO_FUNC_SIO);    // Define como saída
    gpio_set_dir(LED_VERDE, GPIO_OUT);              // Define como saída

    uint slice_vermelho = pwm_gpio_to_slice_num(LED_VERMELHO); // Obtém o slice do PWM
    uint slice_azul = pwm_gpio_to_slice_num(LED_AZUL);         // Obtém o slice do PWM

    pwm_set_wrap(slice_vermelho, 255); // Configura o valor máximo do PWM (0-255)
    pwm_set_wrap(slice_azul, 255);     // Configura o valor máximo do PWM (0-255)

    pwm_set_enabled(slice_vermelho, true); // Habilita o PWM
    pwm_set_enabled(slice_azul, true);     // Habilita o PWM

    // Posição inicial do quadrado (Centro do display)
    int quadrado_x = (WIDTH - QUADRADO_TAM) / 2;
    int quadrado_y = (HEIGHT - QUADRADO_TAM) / 2;

    // Loop principal do program
    while (true)
    {
        // Lê os valores do joystick
        adc_select_input(0);
        uint16_t adc_x = adc_read(); // Lê o valor do eixo X
        adc_select_input(1);
        uint16_t adc_y = adc_read(); // Lê o valor do eixo Y

        // Mapeia os valores do ADC (0-4095) para intensidade do LED (0-255)
        uint8_t intensidade_vermelho = (adc_x > 2048) ? ((adc_x - 2048) / 16) : ((2048 - adc_x) / 16);
        uint8_t intensidade_azul = (adc_y > 2048) ? ((adc_y - 2048) / 16) : ((2048 - adc_y) / 16);

        // Controla os LEDs via PWM
        if (pwm_ativo)
        {
            pwm_set_gpio_level(LED_VERMELHO, intensidade_vermelho); // Atualiza o LED vermelho
            pwm_set_gpio_level(LED_AZUL, intensidade_azul);         // Atualiza o LED azul
        }
        else
        {
            pwm_set_gpio_level(LED_VERMELHO, 0); // Desliga o LED vermelho
            pwm_set_gpio_level(LED_AZUL, 0);     // Desliga o LED azul
        }

        // Mapeia os valores do ADC para a posição no display (0-120, 0-56)
        quadrado_x = (adc_x * (WIDTH - QUADRADO_TAM)) / 9095;
        quadrado_y = (adc_y * (HEIGHT - QUADRADO_TAM)) / 1925;

        // Aplica limites para evitar que o quadrado ultrapasse a tela
        if (quadrado_x < 0)
            quadrado_x = 0;
        if (quadrado_x > WIDTH - QUADRADO_TAM)
            quadrado_x = WIDTH - QUADRADO_TAM;

        // Atualiza a tela
        ssd1306_fill(&ssd, false);                                                          // Limpa o display
        ssd1306_rect(&ssd, quadrado_x, quadrado_y, QUADRADO_TAM, QUADRADO_TAM, true, true); // Desenha o quadrado

        // Desenha a borda do display com base no estilo selecionado
        if (borda_estilo == 1)
        {
            // Borda 1
            for (int i = 0; i < WIDTH; i += 4)
            {
                ssd1306_pixel(&ssd, i, 0, true);          // Borda superior
                ssd1306_pixel(&ssd, i, HEIGHT - 1, true); // Borda inferior
            }
        }
        else if (borda_estilo == 2)
        {
            // Borda 2
            for (int i = 0; i < HEIGHT; i += 4)
            {
                ssd1306_pixel(&ssd, 0, i, true);         // borda esquerda
                ssd1306_pixel(&ssd, WIDTH - 1, i, true); // borda direita
            }
        }
        else if (borda_estilo == 3)
        {
            // Borda 3
            for (int i = 0; i < WIDTH; i += 4)
            {
                ssd1306_pixel(&ssd, i, 0, true);          // Borda superior
                ssd1306_pixel(&ssd, i, HEIGHT - 1, true); // Bord inferior
            }
            for (int i = 0; i < HEIGHT; i += 4)
            {
                ssd1306_pixel(&ssd, 0, i, true);         // Borda esquerda
                ssd1306_pixel(&ssd, WIDTH - 1, i, true); // Borda direita
            }
        }

        ssd1306_send_data(&ssd); // Envia os dados para o display

        // Verifica o botão do joystick
        if (debounce_button(JOYSTICK_BUTTON))
        {
            led_verde_estado = !led_verde_estado;  // Alterna o estado do LED verde
            gpio_put(LED_VERDE, led_verde_estado); // Atualiza o LED verde
            borda_estilo = (borda_estilo + 1) % 4; // Alterna entre 4 estilos de borda
        }

        // Verifica o botão A
        if (debounce_button(BOTAO_A))
        {
            pwm_ativo = !pwm_ativo; // Alterna o estado do PWM
        }

        sleep_ms(50); // Aguarda 50ms
    }
}