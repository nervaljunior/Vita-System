from fastapi import APIRouter, HTTPException, Request
from app.database import mqtt_broker, mqtt_port, mqtt_topic
import paho.mqtt.client as mqtt
import random

router = APIRouter()

received_data = {}

def on_message(client, userdata, message):
    payload = message.payload.decode("utf-8")
    received_data[mqtt_topic] = payload
    print(f"Recebido dado MQTT no tópico {mqtt_topic}: {payload}")

mqtt_client = mqtt.Client()
mqtt_client.on_message = on_message
mqtt_client.connect(mqtt_broker, mqtt_port)
mqtt_client.subscribe(mqtt_topic)
mqtt_client.loop_start()

@router.get('/get_random_data')
async def get_random_data():
    volume_corrente = random.uniform(0, 100)
    razao_ie = random.uniform(0, 10)
    frequencia = random.uniform(0, 50)
    fluxo_medio = random.uniform(0, 30)

    return {
        "Volume_corrente": volume_corrente,
        "Razão_IE": razao_ie,
        "Frequência": frequencia,
        "Fluxo_médio": fluxo_medio
    }

@router.get('/get_mqtt_data')
async def get_mqtt_data():
    data = {}
    if mqtt_topic in received_data:
        data[mqtt_topic] = received_data[mqtt_topic]

    return {
        "Mensagem_MQTT": data.get(mqtt_topic, "Nenhum dado recebido via MQTT")
    }
