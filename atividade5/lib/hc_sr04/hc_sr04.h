#ifndef ULTRASSOM_H
#define ULTRASSOM_H

#include <stdint.h>

void ultrassom_init(void);

float ultrassom_ler_tempo_us(void);

float ultrassom_ler_distancia_cm(void);

uint32_t ultrassom_get_isr_count(void);

#endif