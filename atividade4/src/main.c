/* Relógios (Clock Gating) */
#define SIM_SCGC5   (*((volatile unsigned int*)0x40048038)) // Clock para Portas
#define SIM_SCGC6   (*((volatile unsigned int*)0x4004803C)) // Clock para ADC0

/* Multiplexação de Pinos (Port Control) */
#define PORTB_PCR2  (*((volatile unsigned int*)0x4004A008)) // Pino ADC (A0)
#define PORTB_PCR21 (*((volatile unsigned int*)0x4004A054)) // LED Azul
#define PORTE_PCR26 (*((volatile unsigned int*)0x4004D068)) // LED Verde

/* Controle de GPIO - Porta B (Azul) */
#define GPIOB_PSOR  (*((volatile unsigned int*)0x400FF044)) // Set
#define GPIOB_PCOR  (*((volatile unsigned int*)0x400FF048)) // Clear
#define GPIOB_PDDR  (*((volatile unsigned int*)0x400FF054)) // Direction

/* Controle de GPIO - Porta E (Verde) */
#define GPIOE_PSOR  (*((volatile unsigned int*)0x400FF104)) // Set
#define GPIOE_PCOR  (*((volatile unsigned int*)0x400FF108)) // Clear
#define GPIOE_PDDR  (*((volatile unsigned int*)0x400FF114)) // Direction

/* Conversor Analógico-Digital (ADC0) */
#define ADC0_SC1A   (*((volatile unsigned int*)0x4003B000)) // Status e Controle
#define ADC0_CFG1   (*((volatile unsigned int*)0x4003B008)) // Configuração
#define ADC0_RA     (*((volatile unsigned int*)0x4003B010)) // Resultado

void delayMs(int n) {
    volatile int i;
    volatile int j;
    for (i = 0; i < n; i++) {
        for (j = 0; j < 7000; j++) {}
    }
}

int main(void) {
    // Liga o clock das Portas B (bit 10) e E (bit 13)
    SIM_SCGC5 |= (1 << 10) | (1 << 13);
    // Liga o clock do módulo ADC0 (bit 27)
    SIM_SCGC6 |= (1 << 27);

    // LEDs como GPIO
    PORTB_PCR21 &= ~(0x700); PORTB_PCR21 |= (1 << 8); // Azul
    PORTE_PCR26 &= ~(0x700); PORTE_PCR26 |= (1 << 8); // Verde
    
    // Pino PTB2 como Analógico
    PORTB_PCR2 &= ~(0x700); 

    // Configura os pinos dos LEDs como saída (Output)
    GPIOB_PDDR |= (1 << 21); // Azul
    GPIOE_PDDR |= (1 << 26); // Verde

    // Inicia os LEDs desligados
    GPIOB_PSOR = (1 << 21);
    GPIOE_PSOR = (1 << 26);

    // CFG1: Configura para resolução de 12 bits (bits 3 e 2 = 01)
    ADC0_CFG1 = (1 << 2);

    while (1) {
        ADC0_SC1A = 12;

        // Aguarda a flag COCO ir para 1 no bit 7
        while ((ADC0_SC1A & (1 << 7)) == 0) {
            // Travado aqui enquanto o hardware mede a tensão
        }

        // Ler o registrador RA captura o valor medido e limpa a flag COCO
        unsigned int adc_val = ADC0_RA;

        if (adc_val > 3500) {
            // Tensão próxima de 3.3V: Liga Azul, Desliga Verde
            GPIOB_PCOR = (1 << 21); 
            GPIOE_PSOR = (1 << 26); 
        } 
        else if (adc_val < 500) {
            // Tensão próxima de 0V: Liga Verde, Desliga Azul
            GPIOE_PCOR = (1 << 26); 
            GPIOB_PSOR = (1 << 21); 
        } 
        else {
            // Qualquer outro valor no meio: Desliga ambos
            GPIOB_PSOR = (1 << 21); 
            GPIOE_PSOR = (1 << 26);
        }

        delayMs(50);
    }

    return 0;
}