#include "hc_sr04.h"
#include <zephyr/kernel.h>
#include <zephyr/irq.h>

#define SIM_SCGC5      (*((volatile uint32_t*)0x40048038))
#define SIM_SCGC6      (*((volatile uint32_t*)0x4004803C))
#define PORTC_PCR1     (*((volatile uint32_t*)0x4004B004)) // Echo (PTC1)
#define PORTC_PCR2     (*((volatile uint32_t*)0x4004B008)) // Trigger (PTC2)

/* Base do FTM0: 0x40038000 */
#define FTM0_SC        (*((volatile uint32_t*)0x40038000)) // Offset 0x00
#define FTM0_CNT       (*((volatile uint32_t*)0x40038004)) // Offset 0x04
#define FTM0_MOD       (*((volatile uint32_t*)0x40038008)) // Offset 0x08

#define FTM0_C0SC      (*((volatile uint32_t*)0x4003800C)) // Offset 0x0C (Echo SC)
#define FTM0_C0V       (*((volatile uint32_t*)0x40038010)) // Offset 0x10 (Echo Val)

#define FTM0_C1SC      (*((volatile uint32_t*)0x40038014)) // Offset 0x14 (Trigger SC)
#define FTM0_C1V       (*((volatile uint32_t*)0x40038018)) // Offset 0x18 (Trigger Val)

#define FTM0_MODE      (*((volatile uint32_t*)0x40038050)) // Offset 0x50

#define FTM0_IRQ_NUM   DT_IRQN(DT_NODELABEL(ftm0))

static volatile uint32_t t_subida = 0;
static volatile uint32_t delta_ticks = 0;
static volatile int esperando_subida = 1; 
static volatile uint32_t isr_counter = 0; 

void ftm0_isr(const void *parameter) {
    uint32_t c0sc = FTM0_C0SC;
    
    // Verifica se a interrupção realmente pertence ao Canal 0 (Echo)
    if (c0sc & (1 << 7)) {
        // Limpa a flag
        FTM0_C0SC = c0sc & ~(1 << 7);
        isr_counter++; 

        if (esperando_subida) {
            t_subida = FTM0_C0V;
            esperando_subida = 0;
        } else {
            uint32_t t_descida = FTM0_C0V;
            
            if (t_descida >= t_subida) {
                delta_ticks = t_descida - t_subida;
            } else {
                delta_ticks = (FTM0_MOD - t_subida) + t_descida;
            }
            esperando_subida = 1; 
        }
    }
}

void ultrassom_init(void) {
    // Ativa os clocks
    SIM_SCGC5 |= (1 << 11); 
    SIM_SCGC6 |= (1 << 24);            

    // Desativa o Timer temporariamente para configuração
    FTM0_SC = 0;

    // Destranca os registradores protegidos da NXP
    FTM0_MODE = (1 << 2) | (1 << 0);

    // Configura os pinos como ALT4 (Módulo FTM0)
    PORTC_PCR1 = (4 << 8); // PTC1 -> FTM0_CH0 (Echo)
    PORTC_PCR2 = (4 << 8); // PTC2 -> FTM0_CH1 (Trigger)

    FTM0_CNT = 0;
    FTM0_MOD = 46875; // Janela estável para 20 Hz (50ms)

    // Canal 1 como Saída PWM nativa (Sem interrupção)
    FTM0_C1SC = (1 << 5) | (1 << 3); 
    FTM0_C1V = 9; // Inicia com pulso de ~10 us

    // Canal 0 como Entrada Input Capture
    FTM0_C0SC = (1 << 6) | (1 << 3) | (1 << 2); 

    // Registro dinâmico de interrupção
    irq_connect_dynamic(FTM0_IRQ_NUM, 0, ftm0_isr, NULL, 0);
    irq_enable(FTM0_IRQ_NUM);

    // Clock do Sistema, Prescaler = 64
    FTM0_SC = (1 << 3) | (6 << 0);
}

float ultrassom_ler_tempo_us(void) {
    return (float)delta_ticks * 1.066666f;
}

uint32_t ultrassom_get_isr_count(void) {
    return isr_counter;
}

float ultrassom_ler_distancia_cm(void) {
    return ultrassom_ler_tempo_us() / 58.0f;
}