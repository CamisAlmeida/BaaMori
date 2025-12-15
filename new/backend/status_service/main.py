from fastapi import FastAPI
from pydantic import BaseModel

app = FastAPI()

class Status(BaseModel):
    device_id: str
    ok: bool

@app.post("/status")
def receive_status(status: Status):
    print("📡 Status:", status)
    return {"alive": True}
