from fastapi import FastAPI
from pydantic import BaseModel

app = FastAPI()

class FallEvent(BaseModel):
    device_id: str
    fall_id: int
    detected: bool
    count: int

@app.post("/falls")
def receive_fall(event: FallEvent):
    print("Queda:", event)
    return {"status": "ok"}
