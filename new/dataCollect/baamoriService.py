#!/home/mi/.venv/bin/python3
import websocket
import csv
import math
import statistics
import datetime
import os
import time

# ---------------- CONFIGURAÇÕES ----------------
WINDOW_SIZE = 50
SAVE_DIR = "/home/mi/Documents/BAAMORI/"
WS_URL = "ws://192.168.4.1:81/"

os.makedirs(SAVE_DIR, exist_ok=True)

# ----------- Pergunta tipo de sessão -----------
tipo = ""
while tipo not in ["q", "n"]:
    tipo = input("Essa sessão é: [q] queda, [n] normal ? ").strip().lower()

label = "queda" if tipo == "q" else "normal"

# ----------- Nome do arquivo -----------
timestamp = datetime.datetime.now().strftime("%Y-%m-%d_%H-%M-%S")
filename = f"{label}_{timestamp}.csv"
filepath = os.path.join(SAVE_DIR, filename)

# ----------- WebSocket -----------
ws = websocket.WebSocket()
ws.settimeout(1)
ws.connect(WS_URL)

# ----------- CSV -----------
file = open(filepath, "w", newline="")
writer = csv.writer(file)

writer.writerow([
    "fall_id",
    "n_samples",
    "min_ax","min_ay","min_az","min_gx","min_gy","min_gz",
    "max_ax","max_ay","max_az","max_gx","max_gy","max_gz",
    "mean_ax","mean_ay","mean_az","mean_gx","mean_gy","mean_gz",
    "std_ax","std_ay","std_az","std_gx","std_gy","std_gz",
    "mag_acc_mean","mag_gyro_mean",
    "timestamp_inicio","timestamp_fim",
    "classe"
])

print(f"Coletando para dataset: {filepath}")
print("Aguardando dados... (Ctrl+C para parar)")

# buffer por fall_id
buffers = {}

# timestamps por fall_id
t_inicio = {}

total_msgs = 0
ultimo_fall_id_recebido = -1


# ------------------- FUNÇÃO: PROCESSA UMA QUEDA -------------------
def processar_buffer(fall_id, classe):
    global buffers, t_inicio

    buf = buffers.get(fall_id, [])
    if len(buf) == 0:
        return

    n = len(buf)

    ax = [x[0] for x in buf]
    ay = [x[1] for x in buf]
    az = [x[2] for x in buf]
    gx = [x[3] for x in buf]
    gy = [x[4] for x in buf]
    gz = [x[5] for x in buf]

    mag_acc = sum(math.sqrt(a*a + b*b + c*c) for a,b,c,_,_,_ in buf) / n
    mag_gyro = sum(math.sqrt(gx*gx + gy*gy + gz*gz) for _,_,_,gx,gy,gz in buf) / n
    t_fim = datetime.datetime.now()

    writer.writerow([
        fall_id,
        n,
        min(ax), min(ay), min(az), min(gx), min(gy), min(gz),
        max(ax), max(ay), max(az), max(gx), max(gy), max(gz),
        statistics.mean(ax), statistics.mean(ay), statistics.mean(az),
        statistics.mean(gx), statistics.mean(gy), statistics.mean(gz),
        statistics.pstdev(ax), statistics.pstdev(ay), statistics.pstdev(az),
        statistics.pstdev(gx), statistics.pstdev(gy), statistics.pstdev(gz),
        mag_acc, mag_gyro,
        t_inicio[fall_id].isoformat(),
        t_fim.isoformat(),
        classe
    ])

    print(f"✔ Queda {fall_id} salva com {n} amostras")

    # limpa buffer
    buffers[fall_id] = []
    t_inicio[fall_id] = t_fim


# ------------------- LOOP PRINCIPAL -------------------
try:
    while True:

        # -------- TENTAR RECEBER DADOS --------
        try:
            msg = ws.recv()
        except websocket.WebSocketTimeoutException:
            continue
        except KeyboardInterrupt:
            break

        total_msgs += 1

        vals = msg.strip().split(",")

        # ESP32 manda 8 valores
        if len(vals) != 8:
            continue

        ax, ay, az, gx, gy, gz = map(float, vals[:6])
        classe = int(vals[6])       # 0 normal, 1 queda
        fall_id = int(vals[7])      # identificador da queda

        # cria buffer do id se ainda não existe
        if fall_id not in buffers:
            buffers[fall_id] = []
            t_inicio[fall_id] = datetime.datetime.now()

        # adiciona dado
        buffers[fall_id].append((ax, ay, az, gx, gy, gz))

        # --------------------- MODO NORMAL ---------------------
        if label == "normal":
            if len(buffers[fall_id]) >= WINDOW_SIZE:
                processar_buffer(fall_id, classe=0)

        # --------------------- MODO QUEDA ---------------------
        else:
            # detecta fim da queda quando fall_id muda
            if ultimo_fall_id_recebido != -1 and fall_id != ultimo_fall_id_recebido:
                # processa a queda anterior
                processar_buffer(ultimo_fall_id_recebido, classe=1)

        ultimo_fall_id_recebido = fall_id


except KeyboardInterrupt:
    print("\nEncerrando...")

# processa última queda pendente
if label == "queda":
    for fid in buffers:
        if len(buffers[fid]) > 0:
            processar_buffer(fid, classe=1)

file.close()
ws.close()

print(f"\nTotal de mensagens recebidas: {total_msgs}")
print(f"Arquivo salvo em:\n  {filepath}")
