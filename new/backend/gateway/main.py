from fastapi import FastAPI
from pydantic import BaseModel
from typing import Dict, Any
import requests

app = FastAPI(title="API Gateway")

class FallPayload(BaseModel):
    ts: int
    values: Dict[str, Any]


FALL_SERVICE_URL = "http://localhost:8001/falls"  

@app.post("/falls")
async def receive_fall(payload: FallPayload):
    print("Gateway recebeu")

    r = requests.post(FALL_SERVICE_URL, json=payload.dict(), timeout=5)
    print("Repassado, status:", r.status_code)

    return {"status": "ok"}


# URLs dos microserviços
DASHBOARD_SERVICE_URL = "http://localhost:8003/dashboard"

@app.get("/dashboard")
def gateway_dashboard():
    try:
        resp = requests.get(DASHBOARD_SERVICE_URL, timeout=5)
        resp.raise_for_status()
        return resp.json()
    except Exception as e:
        return {"error": str(e)}

