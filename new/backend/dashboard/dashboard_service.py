# dashboard_service.py
from fastapi import FastAPI
from fastapi.middleware.cors import CORSMiddleware
import requests
import os
from dotenv import load_dotenv

load_dotenv()

app = FastAPI(title="Dashboard Service")

# Permitir que frontend consuma
app.add_middleware(
    CORSMiddleware,
    allow_origins=["*"],
    allow_methods=["*"],
    allow_headers=["*"],
)

import os
import requests
from fastapi import FastAPI
from dotenv import load_dotenv

load_dotenv()

app = FastAPI()

TB_URL = os.getenv("TB_URL")  # ex: https://thingsboard.cloud
DEVICE_ID = os.getenv("TB_DEVICE_ID")  # UUID do device
TB_TOKEN = os.getenv("TB_TOKEN")  # API Token do usuário

import os
import requests
from fastapi import FastAPI
from dotenv import load_dotenv

load_dotenv()

app = FastAPI()

TB_URL = os.getenv("TB_URL")  
DEVICE_ID = os.getenv("TB_DEVICE_ID")  
TB_TOKEN = os.getenv("TB_TOKEN")        

@app.get("/dashboard")
def get_dashboard():
    if DEVICE_ID is None or TB_TOKEN is None:
        return {"error": "DEVICE_ID ou TB_TOKEN não configurados corretamente no .env"}

    url = f"{TB_URL}/api/plugins/telemetry/DEVICE/{DEVICE_ID}/values/timeseries?keys=queda,bpm,acc_x,acc_y,acc_z&limit=20"
    headers = {"X-Authorization": f"Bearer {TB_TOKEN}"}

    try:
        resp = requests.get(url, headers=headers, timeout=5)
        resp.raise_for_status()
        return {"dashboard_data": resp.json()}
    except requests.exceptions.HTTPError as errh:
        return {"error": f"HTTP Error: {errh}"}
    except requests.exceptions.ConnectionError as errc:
        return {"error": f"Connection Error: {errc}"}
    except requests.exceptions.Timeout as errt:
        return {"error": f"Timeout Error: {errt}"}
    except Exception as e:
        return {"error": str(e)}
    
