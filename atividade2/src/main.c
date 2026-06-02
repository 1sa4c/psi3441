#include <zephyr/kernel.h>
#include <zephyr/drivers/gpio.h>

static const struct gpio_dt_spec led_vermelho = GPIO_DT_SPEC_GET(DT_ALIAS(led2), gpios);

#define PERIODO_US 20000 
#define PASSOS_BRILHO 100

int main(void) {
    if (!gpio_is_ready_dt(&led_vermelho)) {
        return 0;
    }

    gpio_pin_configure_dt(&led_vermelho, GPIO_OUTPUT_INACTIVE);

    int duty_cycle = 0; // Vai de 0 a 100
    int direcao = 1;

    while (1) {
        /* mesmo nível de brilho 2 vezes para dar tempo (aprox. 40ms) do olho ver */
        for (int i = 0; i < 2; i++) {
            
            if (duty_cycle > 0) {
                gpio_pin_set_dt(&led_vermelho, 1);
                k_usleep(duty_cycle * (PERIODO_US / PASSOS_BRILHO)); 
            }
            
            if (duty_cycle < PASSOS_BRILHO) {
                gpio_pin_set_dt(&led_vermelho, 0);
                k_usleep((PASSOS_BRILHO - duty_cycle) * (PERIODO_US / PASSOS_BRILHO));
            }
        }

        duty_cycle += direcao * 5; // Muda o brilho de 5 em 5 passos

        if (duty_cycle >= PASSOS_BRILHO) {
            duty_cycle = PASSOS_BRILHO;
            direcao = -1;
        } else if (duty_cycle <= 0) {
            duty_cycle = 0;
            direcao = 1;
        }
    }

    return 0;
}