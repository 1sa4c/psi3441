#include <zephyr.h>
#include <device.h>
#include <drivers/gpio.h>

#define LED_PORT       "GPIO_1"
#define RGB_RED          18     // Pino LED vermelho
#define RGB_GREEN        19     // Pino LED verde
#define RGB_BLUE         13     // Pino LED azul (serve como amarelo)
#define SLEEP_TIME_MS  1000

typedef enum {
    ESTADO_VERMELHO,
    ESTADO_VERDE,
    ESTADO_AZUL
} estado_semaforo_t;

int main(void) {
    const struct device *port = device_get_binding(LED_PORT);

    if (port == NULL) {
        return -1;
    }

    gpio_pin_configure(port, RGB_RED, GPIO_OUTPUT_ACTIVE);
    gpio_pin_configure(port, RGB_GREEN, GPIO_OUTPUT_ACTIVE);
    gpio_pin_configure(port, RGB_BLUE, GPIO_OUTPUT_ACTIVE);

    estado_semaforo_t estado_atual = ESTADO_VERMELHO;

    while (1) {
        switch (estado_atual) {
            
            case ESTADO_VERMELHO:
                gpio_pin_set(port, RGB_RED, 1);
                gpio_pin_set(port, RGB_GREEN, 0);
                gpio_pin_set(port, RGB_BLUE, 0);

                k_msleep(SLEEP_TIME_MS * 2);
                estado_atual = ESTADO_VERDE;
                break;

            case ESTADO_VERDE:
                gpio_pin_set(port, RGB_RED, 0);
                gpio_pin_set(port, RGB_GREEN, 1);
                gpio_pin_set(port, RGB_BLUE, 0);

                k_msleep(SLEEP_TIME_MS * 2);
                estado_atual = ESTADO_AZUL;
                break;

            case ESTADO_AZUL:
                gpio_pin_set(port, RGB_RED, 0);
                gpio_pin_set(port, RGB_GREEN, 0);
                gpio_pin_set(port, RGB_BLUE, 1);

                k_msleep(SLEEP_TIME_MS);
                estado_atual = ESTADO_VERMELHO;
                break;
        }
    }

    return 0;
}