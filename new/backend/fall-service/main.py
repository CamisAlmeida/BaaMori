from fastapi import FastAPI
from pydantic import BaseModel
from typing import Dict
from datetime import datetime
import requests
import time
import os
from dotenv import load_dotenv

load_dotenv()  

app = FastAPI()
TELEGRAM_TOKEN = os.getenv("TELEGRAM_TOKEN")
CHAT_ID = os.getenv("TELEGRAM_CHAT_ID")


def send_telegram(msg: str):
    url = f"https://api.telegram.org/bot{TELEGRAM_TOKEN}/sendMessage"
    requests.post(
        url,
        json={"chat_id": CHAT_ID, "text": msg},
        timeout=10
    )

# Models
class FallValues(BaseModel):
    device_id: str
    queda: int
    fall_id: int
    count: int

    acc_min_x: float
    acc_min_y: float
    acc_min_z: float
    acc_max_x: float
    acc_max_y: float
    acc_max_z: float
    acc_mean_x: float
    acc_mean_y: float
    acc_mean_z: float

    gyro_min_x: float
    gyro_min_y: float
    gyro_min_z: float
    gyro_max_x: float
    gyro_max_y: float
    gyro_max_z: float
    gyro_mean_x: float
    gyro_mean_y: float
    gyro_mean_z: float


class FallEvent(BaseModel):
    ts: int
    values: FallValues


@app.post("/falls")
def receive_fall(event: FallEvent):
    print("QUEDA RECEBIDA")
    print("Device:", event.values.device_id)
    print("Fall ID:", event.values.fall_id)
    print("Queda:", event.values.queda)

    if event.values.queda == 1:
        ts_ms = event.ts
        dt = datetime.fromtimestamp(ts_ms / 1000)
        formatted_time = dt.strftime("%d/%m/%Y %H:%M:%S")

        msg = (
            "*QUEDA DETECTADA*\n\n"
            f"Dispositivo: {event.values.device_id}\n"
            f"Fall ID: {event.values.fall_id}\n"
            f"Timestamp: {formatted_time}"
        )
        send_telegram(msg)

    return {"status": "ok"}