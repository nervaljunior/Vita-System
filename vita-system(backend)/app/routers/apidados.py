from fastapi import APIRouter, HTTPException, Request
import paho.mqtt.client as mqtt
from sqlalchemy.orm import Session
from .database import get_db
from .models import Dados, User

mqtt_broker = "broker.mqttdashboard.com"
mqtt_port = 1883
mqtt_topic = "wokwi-weather"

router = APIRouter()

received_data = {}

def on_message(client, userdata, message, db: Session = None):
    payload = message.payload.decode("utf-8")
#    received_data[mqtt_topic] = payload
    print(f"Recebido dado MQTT no tópico {mqtt_topic}: {payload}")

    if db and Request.state.user:
        data = Dados(user_id=Request.state.user.id, data=payload)
        db.add(data)
        db.commit()

mqtt_client = mqtt.Client()
mqtt_client.on_message = on_message
mqtt_client.connect(mqtt_broker, mqtt_port)
mqtt_client.subscribe(mqtt_topic)
mqtt_client.loop_start()

@router.get('/get_mqtt_data')
async def get_mqtt_data():
    data = {}
    if mqtt_topic in received_data:
        data[mqtt_topic] = received_data[mqtt_topic]

    return {
        "Mensagem_MQTT": data.get(mqtt_topic, "Nenhum dado recebido via MQTT")
    }
