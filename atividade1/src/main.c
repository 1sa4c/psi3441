#include <zephyr/kernel.h>
#include <zephyr/drivers/gpio.h>

#define SLEEP_TIME_MS  1000

/* 1. Mapeamento dos pinos usando a Device Tree da FRDM-K64F */
#define LED_GREEN_NODE DT_ALIAS(led0)
#define LED_BLUE_NODE  DT_ALIAS(led1)
#define LED_RED_NODE   DT_ALIAS(led2)

/* Inicializa as estruturas dos pinos */
static const struct gpio_dt_spec led_verde    = GPIO_DT_SPEC_GET(LED_GREEN_NODE, gpios);
static const struct gpio_dt_spec led_azul     = GPIO_DT_SPEC_GET(LED_BLUE_NODE, gpios);
static const struct gpio_dt_spec led_vermelho = GPIO_DT_SPEC_GET(LED_RED_NODE, gpios);

/* 2. Definição dos estados da nossa máquina de estados */
typedef enum {
    ESTADO_VERMELHO,
    ESTADO_VERDE,
    ESTADO_AZUL
} estado_semaforo_t;

int main(void) 
{
    /* Verifica se todos os controladores GPIO estão prontos */
    if (!gpio_is_ready_dt(&led_vermelho) || 
        !gpio_is_ready_dt(&led_verde) || 
        !gpio_is_ready_dt(&led_azul)) {
        return 0; // Erro de hardware
    }

    /* Configura todos os 3 pinos como saídas e já inicia apagados (INACTIVE) */
    gpio_pin_configure_dt(&led_vermelho, GPIO_OUTPUT_INACTIVE);
    gpio_pin_configure_dt(&led_verde, GPIO_OUTPUT_INACTIVE);
    gpio_pin_configure_dt(&led_azul, GPIO_OUTPUT_INACTIVE);

    /* Inicializa a máquina de estados começando pelo vermelho */
    estado_semaforo_t estado_atual = ESTADO_VERMELHO;

    while (1) {
        /* 3. Máquina de Estados usando switch/case */
        switch (estado_atual) {
            
            case ESTADO_VERMELHO:
                /* Liga vermelho, garante que verde e azul estejam desligados */
                gpio_pin_set_dt(&led_vermelho, 1);
                gpio_pin_set_dt(&led_verde, 0);
                gpio_pin_set_dt(&led_azul, 0);

                /* Espera 2 segundos no vermelho e transita para o verde */
                k_msleep(SLEEP_TIME_MS * 2);
                estado_atual = ESTADO_VERDE;
                break;

            case ESTADO_VERDE:
                /* Liga verde, garante que vermelho e azul estejam desligados */
                gpio_pin_set_dt(&led_vermelho, 0);
                gpio_pin_set_dt(&led_verde, 1);
                gpio_pin_set_dt(&led_azul, 0);

                /* Espera 2 segundos no verde e transita para o azul (atenção) */
                k_msleep(SLEEP_TIME_MS * 2);
                estado_atual = ESTADO_AZUL;
                break;

            case ESTADO_AZUL:
                /* Liga azul, garante que vermelho e verde estejam desligados */
                gpio_pin_set_dt(&led_vermelho, 0);
                gpio_pin_set_dt(&led_verde, 0);
                gpio_pin_set_dt(&led_azul, 1);

                /* Espera 1 segundo na atenção e volta para o vermelho */
                k_msleep(SLEEP_TIME_MS);
                estado_atual = ESTADO_VERMELHO;
                break;
        }
    }

    return 0;
}