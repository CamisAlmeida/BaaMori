from fastapi import FastAPI
from pydantic import BaseModel
from typing import Dict

app = FastAPI()

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
    print("QUEDA RECEBIDA!!!!")
    print("Timestamp:", event.ts)
    print("Device:", event.values.device_id)
    print("Fall ID:", event.values.fall_id)
    return {"status": "ok"}
