# dashboard_service.py
from fastapi import FastAPI, Query
from fastapi.middleware.cors import CORSMiddleware
from fastapi.staticfiles import StaticFiles
from fastapi.responses import FileResponse

import requests
import os
import time
from dotenv import load_dotenv

# -------------------------------------------------
load_dotenv()

app = FastAPI(title="Dashboard Service")

# Permitir que frontend consuma
app.add_middleware(
    CORSMiddleware,
    allow_origins=["*"],
    allow_methods=["*"],
    allow_headers=["*"],
)

# Servir arquivos estáticos
app.mount("/static", StaticFiles(directory="static"), name="static")


TB_URL = os.getenv("TB_URL")
DEVICE_ID = os.getenv("TB_DEVICE_ID")
TB_TOKEN_JWT = os.getenv("TB_TOKEN_JWT")

@app.get("/")
def dashboard_page():
    return FileResponse("static/index.html")


@app.get("/dashboard")
def get_dashboard():
    if not DEVICE_ID or not TB_TOKEN_JWT:
        return {"error": "DEVICE_ID ou TB_TOKEN não configurados"}

    keys = (
        "queda,"
        "acc_min_x,acc_min_y,acc_min_z,"
        "acc_max_x,acc_max_y,acc_max_z,"
        "acc_mean_x,acc_mean_y,acc_mean_z,"
        "gyro_min_x,gyro_min_y,gyro_min_z,"
        "gyro_max_x,gyro_max_y,gyro_max_z,"
        "gyro_mean_x,gyro_mean_y,gyro_mean_z"
    )

    url = (
        f"{TB_URL}/api/plugins/telemetry/DEVICE/{DEVICE_ID}/values/timeseries"
        f"?keys={keys}&limit=20"
    )

    headers = {"X-Authorization": f"Bearer {TB_TOKEN_JWT}"}

    resp = requests.get(url, headers=headers, timeout=5)
    resp.raise_for_status()

    return {"dashboard_data": resp.json()}


@app.get("/dashboard/history")
def get_dashboard_history(limit: int = Query(20, ge=1, le=500)):
    if not DEVICE_ID or not TB_TOKEN_JWT:
        return {"error": "DEVICE_ID ou TB_TOKEN não configurados"}

    keys = (
        "queda,"
        "acc_min_x,acc_min_y,acc_min_z,"
        "acc_max_x,acc_max_y,acc_max_z,"
        "acc_mean_x,acc_mean_y,acc_mean_z,"
        "gyro_min_x,gyro_min_y,gyro_min_z,"
        "gyro_max_x,gyro_max_y,gyro_max_z,"
        "gyro_mean_x,gyro_mean_y,gyro_mean_z"
    )

    end_ts = int(time.time() * 1000)
    start_ts = 0

    url = (
        f"{TB_URL}/api/plugins/telemetry/DEVICE/{DEVICE_ID}/values/timeseries"
        f"?keys={keys}"
        f"&startTs={start_ts}"
        f"&endTs={end_ts}"
        f"&limit={limit}"
        f"&orderBy=DESC"
    )

    headers = {"X-Authorization": f"Bearer {TB_TOKEN_JWT}"}

    resp = requests.get(url, headers=headers, timeout=5)
    resp.raise_for_status()

    return {"dashboard_data": resp.json()}
