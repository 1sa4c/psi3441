#include <zephyr/kernel.h>
#include "hc_sr04.h"

int main(void) {
    k_msleep(2000); 
    
    ultrassom_init();

    while (1) {
        float distancia = ultrassom_ler_distancia_cm();
        
        // if (distancia > 400.0f || distancia <= 0.0f) {
        //     printk("Fora de alcance...\n");
        // } else {
            printk("Distancia: %.2f cm\n", distancia);
        //}
        
        k_msleep(250); 
    }

    return 0;
}