#define SIM_SCGC5   (*((volatile unsigned int*)0x40048038))
#define PORTE_PCR26 (*((volatile unsigned int*)0x4004D068))
#define GPIOE_PSOR  (*((volatile unsigned int*)0x400FF104)) // Port Set Output Register
#define GPIOE_PCOR  (*((volatile unsigned int*)0x400FF108)) // Port Clear Output Register
#define GPIOE_PDDR  (*((volatile unsigned int*)0x400FF114)) // Port Data Direction Register

void delayMs(int n) {
    volatile int i;
    volatile int j;
    for (i = 0; i < n; i++) {
        for (j = 0; j < 7000; j++) {

        }
    }
}

int main(void) {
    SIM_SCGC5 |= (1 << 13);

    PORTE_PCR26 &= ~(0x700);
    PORTE_PCR26 |= (1 << 8);

    GPIOE_PDDR |= (1 << 26);

    while (1) {

        GPIOE_PCOR = (1 << 26);

        delayMs(1000);

        GPIOE_PSOR = (1 << 26);

        delayMs(1000);
    }

    return 0;
}