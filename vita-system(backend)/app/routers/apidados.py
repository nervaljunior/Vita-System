from fastapi import APIRouter
import paho.mqtt.client as mqtt
from pydantic import BaseModel
import mysql.connector
from mysql.connector import errorcode

router = APIRouter()


mqtt_broker = "broker.mqttdashboard.com"
mqtt_port = 1883
mqtt_topic = "wokwi-weather"

received_data = {}


class MqttData(BaseModel):
    data: str

try:
    connection = mysql.connector.connect(
        host="localhost",
        user="root",
        password="",
        database="vita",
        port=3307
    )
except mysql.connector.Error as err:
    if err.errno == errorcode.ER_ACCESS_DENIED_ERROR:
        print("Error: Access denied. Check your credentials.")
    elif err.errno == errorcode.ER_BAD_DB_ERROR:
        print("Error: The specified database does not exist.")
    else:
        print("Error:", err)
    exit(1)

cursor = connection.cursor()


def on_message(client, userdata, message):
    payload = message.payload.decode("utf-8")
    received_data[mqtt_topic] = payload
    print(f"Received MQTT data on topic {mqtt_topic}: {payload}")


mqtt_client = mqtt.Client()
mqtt_client.on_message = on_message
mqtt_client.connect(mqtt_broker, mqtt_port)
mqtt_client.subscribe(mqtt_topic)
mqtt_client.loop_start()

@router.get('/get_data')
async def get_mqtt_data():
    return {
        "Mensagem_MQTT": received_data.get(mqtt_topic, "Nenhum dado recebido via MQTT")
    }

@router.post("/save_data")
async def save_mqtt_data(data: MqttData):
    query = "INSERT INTO dados (data) VALUES (%s)"
    cursor.execute(query, (data.data,))
    connection.commit()
    
    return {"message": "Data received and saved successfully"}
