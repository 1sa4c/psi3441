#include <zephyr/kernel.h>
#include <zephyr/device.h>
#include <zephyr/drivers/adc.h>
#include <zephyr/logging/log.h>
#include <stdio.h>

LOG_MODULE_REGISTER(daq_system, LOG_LEVEL_INF);


#define ADC_NODE        DT_ALIAS(my_adc)
#define ADC_RESOLUTION  12
#define ADC_CHANNEL     12
#define ADC_PORT        adc0
#define ADC_REFERENCE   ADC_REF_INTERNAL
#define ADC_GAIN        ADC_GAIN_1

#define QUEUE_SIZE 100
#define SAMPLE_INTERVAL_MS 1
#define ENABLE_FIR_FILTER 1

typedef struct {
    uint32_t timestamp;
    int16_t raw_value;
    int16_t filtered_value;
} data_packet_t;

/* Fila de mensagens para comunicação entre threads */
K_MSGQ_DEFINE(data_queue, sizeof(data_packet_t), QUEUE_SIZE, 4);

/* ------------------------------------------------------------------
 * Filtro Média Móvel de 1024 amostras
 * ------------------------------------------------------------------ */
#define FIR_TAPS 1024
float fir_buffer[FIR_TAPS] = {0.0f};
uint16_t fir_index = 0;

/* Coeficiente estático para simular um Low-Pass (1/1024) */
const float FIR_COEFF = 0.0009765625; 

int16_t apply_fir_filter(int16_t new_sample) {
    /* Converte a amostra inteira para float para gastar mais ciclos */
    fir_buffer[fir_index] = (float)new_sample;
    fir_index = (fir_index + 1) % FIR_TAPS;

    float sum = 0.0f;
    
    // Um loop de 1024 iterações fazendo multiplicação de floats.
    for (int i = 0; i < FIR_TAPS; i++) {
        sum += fir_buffer[i] * FIR_COEFF;
    }
    
    return (int16_t)sum;
}

/* ------------------------------------------------------------------
 * Thread 1: Aquisição de Dados
 * ------------------------------------------------------------------ */
void acquisition_thread(void) {
    const struct device *adc_dev = DEVICE_DT_GET(ADC_NODE);
    if (!device_is_ready(adc_dev)) {
        LOG_ERR("Dispositivo ADC não está pronto!");
        return;
    }

    struct adc_channel_cfg channel_cfg = {
        .gain             = ADC_GAIN,
        .reference        = ADC_REFERENCE,
        .acquisition_time = ADC_ACQ_TIME_DEFAULT,
        .channel_id       = ADC_CHANNEL,
        .differential     = 0
    };
    adc_channel_setup(adc_dev, &channel_cfg);

    int16_t sample_buffer[1];
    struct adc_sequence sequence = {
        .channels    = BIT(ADC_CHANNEL),
        .buffer      = sample_buffer,
        .buffer_size = sizeof(sample_buffer),
        .resolution  = ADC_RESOLUTION,
    };

    LOG_INF("Iniciando Thread de Aquisição...");

    while (1) {
        uint32_t start_time = k_cycle_get_32();
        
        int ret = adc_read(adc_dev, &sequence);
        if (ret < 0) {
            LOG_ERR("Erro na leitura do ADC: %d", ret);
            continue;
        }

        data_packet_t packet;
        packet.timestamp = k_uptime_get_32();
        packet.raw_value = sample_buffer[0];

        if (ENABLE_FIR_FILTER) {
            packet.filtered_value = apply_fir_filter(packet.raw_value);
        } else {
            packet.filtered_value = packet.raw_value;
        }

        if (k_msgq_put(&data_queue, &packet, K_NO_WAIT) != 0) {
            /* Se a fila estiver cheia, perdemos a mensagem. Registra no Log. */
            LOG_WRN("Fila cheia! Mensagem descartada no tempo %u ms", packet.timestamp);
        }

        k_msleep(SAMPLE_INTERVAL_MS); 
    }
}

/* ------------------------------------------------------------------
 * Thread 2: Comunicação
 * ------------------------------------------------------------------ */
void communication_thread(void) {
    data_packet_t packet;
    LOG_INF("Iniciando Thread de Comunicacao...");
    
    // Pequeno delay para o script Python ter tempo de conectar
    k_msleep(2000); 

    while (1) {
        /* Bloqueia até ter dado na fila */
        k_msgq_get(&data_queue, &packet, K_FOREVER);
        
        printk("%u,%d,%d\n", packet.timestamp, packet.raw_value, packet.filtered_value);
    }
}


K_THREAD_DEFINE(acq_tid, 1024, acquisition_thread, NULL, NULL, NULL, 5, 0, 0);
K_THREAD_DEFINE(comm_tid, 1024, communication_thread, NULL, NULL, NULL, 7, 0, 0);