# executar com: /home/mi/.venv/bin/python3 /home/mi/Documents/BAAMORI/baamoriService.py
#!/home/mi/.venv/bin/python3
import websocket
import csv
import math
import statistics
import datetime
import os

# CONFIGURAÇÕES 
WINDOW_SIZE = 50
SAVE_DIR = "/home/mi/Documents/BAAMORI/"
WS_URL = "ws://192.168.4.1:81/"
# 

os.makedirs(SAVE_DIR, exist_ok=True)  # garante que o diretório exista

# Pergunta tipo de sessão
tipo = ""
while tipo not in ["q", "n"]:
    tipo = input("Essa sessão é: [q] queda, [n] normal ? ").strip().lower()

label = "queda" if tipo == "q" else "normal"

# Cria nome automaticamente
timestamp = datetime.datetime.now().strftime("%Y-%m-%d_%H-%M-%S")
filename = f"{label}_{timestamp}.csv"
filepath = os.path.join(SAVE_DIR, filename)

# Conecta WebSocket
ws = websocket.WebSocket()
ws.connect(WS_URL)

# Abre arquivo
file = open(filepath, "w", newline="")
writer = csv.writer(file)

# Cabeçalho do CSV
writer.writerow([
    "ax","ay","az","gx","gy","gz",
    "min_ax","min_ay","min_az","min_gx","min_gy","min_gz",
    "max_ax","max_ay","max_az","max_gx","max_gy","max_gz",
    "mean_ax","mean_ay","mean_az","mean_gx","mean_gy","mean_gz",
    "std_ax","std_ay","std_az","std_gx","std_gy","std_gz",
    "mag_acc_mean","mag_gyro_mean",
    "timestamp_inicio","timestamp_fim",
    "classe"
])

buffer = []
print(f"Coletando para dataset: {filepath}\nCtrl+C para parar.")

t_inicio = datetime.datetime.now()

try:
    while True:
        try:
            msg = ws.recv()
        except websocket.WebSocketTimeoutException:
            continue
        except websocket.WebSocketConnectionClosedException:
            print("Conexão perdida, reconectando...")
            ws = websocket.WebSocket()
            ws.connect(WS_URL)
            continue

        vals = msg.strip().split(",")

        if len(vals) == 6:
            ax, ay, az, gx, gy, gz = map(float, vals)
            buffer.append((ax, ay, az, gx, gy, gz))

            # Se o buffer encheu → processa
            if len(buffer) == WINDOW_SIZE:

                ax_list = [b[0] for b in buffer]
                ay_list = [b[1] for b in buffer]
                az_list = [b[2] for b in buffer]
                gx_list = [b[3] for b in buffer]
                gy_list = [b[4] for b in buffer]
                gz_list = [b[5] for b in buffer]

                # magnitudes médias
                mag_acc = sum(
                    math.sqrt(a*a + b*b + c*c) 
                    for a,b,c,_,_,_ in buffer
                ) / WINDOW_SIZE

                mag_gyro = sum(
                    math.sqrt(gx*gx + gy*gy + gz*gz) 
                    for _,_,_,gx,gy,gz in buffer
                ) / WINDOW_SIZE

                t_fim = datetime.datetime.now()

                # salva cada amostra da janela com estatísticas
                for i in range(WINDOW_SIZE):
                    writer.writerow([
                        buffer[i][0], buffer[i][1], buffer[i][2],
                        buffer[i][3], buffer[i][4], buffer[i][5],

                        min(ax_list), min(ay_list), min(az_list),
                        min(gx_list), min(gy_list), min(gz_list),

                        max(ax_list), max(ay_list), max(az_list),
                        max(gx_list), max(gy_list), max(gz_list),

                        statistics.mean(ax_list),
                        statistics.mean(ay_list),
                        statistics.mean(az_list),
                        statistics.mean(gx_list),
                        statistics.mean(gy_list),
                        statistics.mean(gz_list),

                        statistics.pstdev(ax_list),
                        statistics.pstdev(ay_list),
                        statistics.pstdev(az_list),
                        statistics.pstdev(gx_list),
                        statistics.pstdev(gy_list),
                        statistics.pstdev(gz_list),

                        mag_acc,
                        mag_gyro,

                        t_inicio.isoformat(),
                        t_fim.isoformat(),

                        label
                    ])

                buffer.clear()
                t_inicio = t_fim  # próxima janela

except KeyboardInterrupt:
    print("Coleta finalizada.")

file.close()
ws.close()
print(f"\nArquivo salvo em:\n  {filepath}")
