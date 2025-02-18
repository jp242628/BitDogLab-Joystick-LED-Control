# BitDogLab-Joystick-LED-Control

Projeto que utiliza um joystick para controlar a intensidade de LEDs RGB via PWM, movimentar um quadrado no display SSD1306 e alternar funcionalidades com botões. Implementa debouncing, I2C e interrupções para um sistema interativo e responsivo.

## Funcionalidades

- Controle da intensidade dos LEDs RGB utilizando PWM.
- Movimentação de um quadrado no display OLED SSD1306 com base na posição do joystick.
- Alternância de estilos de borda no display.
- Controle de estado dos LEDs e PWM através de botões.

## Hardware e Conexões

### Joystick
- **Botão**: GPIO 22
- **Eixo X**: GPIO 26
- **Eixo Y**: GPIO 27

### LEDs RGB
- **Vermelho**: GPIO 13
- **Verde**: GPIO 11
- **Azul**: GPIO 12

### Botões
- **Botão A**: GPIO 5
- **Botão B**: GPIO 6

### Display OLED SSD1306
- **I2C SDA**: GPIO 14
- **I2C SCL**: GPIO 15
- **Endereço I2C**: 0x3C

## Compilação e Execução

Clone o repositório:

```sh
git clone https://github.com/jp242628/BitDogLab-Joystick-LED-Control.git
cd BitDogLab-Joystick-LED-Control
```

Configure o ambiente, compile e carregue o código no Raspberry Pi Pico utilizando o ambiente de desenvolvimento de sua preferência.

## Código Principal

O código principal está localizado no arquivo `JoystickLEDControl.c`. Ele inicializa os periféricos, configura os pinos GPIO, e implementa o loop principal que lê os valores do joystick, controla os LEDs e atualiza o display OLED.