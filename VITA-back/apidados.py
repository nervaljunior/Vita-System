from fastapi import FastAPI, HTTPException
from fastapi.middleware.cors import CORSMiddleware
import uvicorn
import paho.mqtt.client as mqtt

app_mqtt = FastAPI()

app_mqtt.add_middleware(
    CORSMiddleware,
    allow_methods=["*"],
    allow_headers=["*"],
    allow_credentials=True,
    allow_origins=["http://localhost:3000"]
)

mqtt_broker = "broker.mqttdashboard.com"
mqtt_port = 1883
mqtt_topic = "wokwi-weather"

received_data = {}  # Armazenar os dados recebidos via MQTT

def on_message(client, userdata, message):
    payload = message.payload.decode("utf-8")
    received_data[mqtt_topic] = payload
    print(f"Recebido dado MQTT no tópico {mqtt_topic}: {payload}")

mqtt_client = mqtt.Client()
mqtt_client.on_message = on_message
mqtt_client.connect(mqtt_broker, mqtt_port)
mqtt_client.subscribe(mqtt_topic)
mqtt_client.loop_start()

@app_mqtt.get('/get_data')
async def get_data():
    data = {}

    if mqtt_topic in received_data:
        data[mqtt_topic] = received_data[mqtt_topic]

    return {
        "Volume_corrente": data.get(mqtt_topic, 0),
        "Razão_IE": data.get(mqtt_topic, 0),
        "Frequência": data.get(mqtt_topic, 0),
        "Fluxo_médio": data.get(mqtt_topic, 0)
    }

@app_mqtt.post("/receber-dados")
async def receber_dados(payload: str):
    try:
        mqtt_client.publish(mqtt_topic, payload)
        return {"message": "Dados enviados com sucesso para o MQTT."}
    except Exception as e:
        raise HTTPException(status_code=500, detail=str(e))

if __name__ == '__main__':
    uvicorn.run(app_mqtt, host='0.0.0.0', port=7778)
