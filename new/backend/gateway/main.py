from fastapi import FastAPI
from pydantic import BaseModel
from typing import Dict, Any

app = FastAPI()

class FallPayload(BaseModel):
    ts: int
    values: Dict[str, Any]

@app.post("/falls")
async def receive_fall(payload: FallPayload):
    print("=== QUEDA RECEBIDA ===")
    print("Timestamp:", payload.ts)
    print("Dados:", payload.values)
    return {"status": "ok"}
