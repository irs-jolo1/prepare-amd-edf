import adi

from fastapi import FastAPI, WebSocket, Query, HTTPException
from fastapi.middleware.cors import CORSMiddleware
import uvicorn
from pydantic import BaseModel
import asyncio
import random
from starlette.websockets import WebSocketDisconnect

from irsutils.adc import ADC

app = FastAPI()

# Allow all CORS
app.add_middleware(
    CORSMiddleware,
    allow_origins=["*"],
    allow_credentials=False,
    allow_methods=["GET", "POST"],
    allow_headers=["*"],
)

class Config(BaseModel):
    buffer_size: int
    sample_rate: int

class Device(BaseModel):
    uri: str
    device_name: str

adc = None

@app.get('/config')
def get_setup(uri: str, device_name: str):
    global adc
    try:
        adc = ADC(uri, device_name)
        config = adc.adc.get_config()
        return { "status": "ok", "config": config  }
    except Exception as e:
        raise HTTPException(status_code=503, detail=f"Could not connect to IIO backend or IIO device: {e}")


@app.post('/config')
def post_config(config: Config):
    print("config:", config)
    try:
        #  adc = adi.ad7606(uri=, device_name="ad7606c18")
        #  data = request.get_json()
        #  adc.channel[0].scale = 1
        return {"status": "ok" }
    except Exception as e:
        return {"status": "error", "message": f"Could not not write config: {e}"}

@app.websocket("/ws")
async def websocket_endpoint(ws: WebSocket, sample_rate: int, buffer_size: int):

    print(f"Starting websocket stream with sample rate: {sample_rate}, and buffer size {buffer_size}")

    print("Websocket client connected")
    await ws.accept()
    if adc is None:
        print("Server not ready (IIO device not initialized)")
        await ws.close()
        return

    try:
        while True:
            data = adc.read(buffer_size=buffer_size, sample_rate=sample_rate)
            await ws.send_json({"status": "ok", "voltages": data})
            #  await asyncio.sleep(0.5) # make variable
    except WebSocketDisconnect:
        print("Client disconnected")
    except Exception as e:
        print("Unexpected error:", e)
    finally:
        try:
            await ws.close()
        except RuntimeError:
            pass

def run_server(args):
    uvicorn.run(
        "irsutils.server:app",
        host="0.0.0.0",
        port=8082,
        reload=False
    )
