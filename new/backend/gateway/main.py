from fastapi import FastAPI
from pydantic import BaseModel
from typing import Dict, Any
import requests

app = FastAPI()

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
