import serial
import matplotlib.pyplot as plt
import matplotlib.animation as animation
import collections

log_file = open("erros_zephyr.txt", "w")

# Configurações
SERIAL_PORT = '/dev/ttyACM0'  # Altere para sua porta (ex: /dev/ttyACM0)
BAUD_RATE = 230400    # Padrão do console do Zephyr
MAX_POINTS = 200      # Número de pontos mostrados na tela por vez

# Estruturas de dados (Deques são rápidos para operações de "janela deslizante")
timestamps = collections.deque(maxlen=MAX_POINTS)
raw_data = collections.deque(maxlen=MAX_POINTS)
filtered_data = collections.deque(maxlen=MAX_POINTS)

# Configuração do Gráfico
fig, ax = plt.subplots()
line_raw, = ax.plot([], [], label='Dado Bruto (Raw)', color='red', alpha=0.5)
line_filt, = ax.plot([], [], label='Dado Filtrado (FIR)', color='blue', linewidth=2)
ax.set_title('Aquisição de Dados em Tempo Real - FRDM-K64F')
ax.set_xlabel('Tempo (ms)')
ax.set_ylabel('Valor ADC (0-4095)')
ax.set_ylim(0, 4100)
ax.legend()

try:
    ser = serial.Serial(SERIAL_PORT, BAUD_RATE, timeout=0.1)
    print(f"Conectado a {SERIAL_PORT}")
except Exception as e:
    print(f"Erro ao abrir porta serial: {e}")
    exit()


def update_plot(frame):
    lines_read = 0
    
    # Válvula de escape: se o buffer do PC estiver gigantesco (muito atrasado)
    # nós limpamos tudo para evitar que o gráfico mostre o passado distante e congele.
    if ser.in_waiting > 4000:
        ser.reset_input_buffer()
        
    # Lê no máximo 200 linhas por frame
    while ser.in_waiting > 0 and lines_read < 200:
        try:
            # 1. Lê os bytes crus primeiro
            raw_bytes = ser.readline()

            # 2. Tenta decodificar para texto
            line = raw_bytes.decode('utf-8').strip()
            parts = line.split(',')

            # Processa apenas se for um pacote CSV perfeito
            if len(parts) == 3:
                ts = int(parts[0])
                raw = int(parts[1])
                filt = int(parts[2])

                timestamps.append(ts)
                raw_data.append(raw)
                filtered_data.append(filt)

            elif len(line) > 0:
                log_file.write("[LOG ZEPHYR] " + line + "\n")
                log_file.flush() # Força a gravação no disco imediatamente

        except UnicodeDecodeError:
            # 3. Se não for texto válido, salva o lixo bruto no arquivo!
            log_file.write(f"[ERRO SERIAL] Lixo recebido: {raw_bytes}\n")
            log_file.flush()
            pass
        except ValueError:
            pass

    if len(timestamps) > 0:
        line_raw.set_data(timestamps, raw_data)
        line_filt.set_data(timestamps, filtered_data)

        # Ajusta o eixo X dinamicamente
        ax.set_xlim(min(timestamps), max(timestamps) + 10)

    return line_raw, line_filt

# Atualiza o gráfico a cada 50ms
ani = animation.FuncAnimation(fig, update_plot, interval=50, cache_frame_data=False)

plt.tight_layout()
plt.show()

ser.close()
log_file.close()
