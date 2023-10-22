from app.models import ArduinoData
from app.db import db
from fastapi import FastAPI
from fastapi.middleware.cors import CORSMiddleware
import random

app = FastAPI()


@app.get('/get_data')
async def get_data():
    # Gere valores aleatórios para cada propriedade
    volume_corrente = random.uniform(0, 100)  # Gere um valor entre 0 e 100
    razao_ie = random.uniform(0, 10)  # Gere um valor entre 0 e 10
    frequencia = random.uniform(0, 50)  # Gere um valor entre 0 e 50
    fluxo_medio = random.uniform(0, 30)  # Gere um valor entre 0 e 30

    return {
        "Volume_corrente": volume_corrente,
        "Razão_IE": razao_ie,
        "Frequência": frequencia,
        "Fluxo_médio": fluxo_medio
    }
