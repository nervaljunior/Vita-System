#esses dados aqui viram direto do brokerMQTT
# é a parte de crud e pegar os dados na tem

from fastapi import FastAPI,request,jsonify
from fastapi.middleware.cors import CORSMiddleware
import uvicorn
from fastapi import HTTPException
import paho.mqtt.client as mqtt
import mysql.connector

app= FastAPI()

mydb = mysql.connector.connect(
    host="localhost",
    user="root",
    password="nervs",
    database="VITA"
)

app.add_middleware(
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

@app.get('/get_data')
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

@app.post("/receber-dados")
async def receber_dados(payload: str):
    try:
        mqtt_client.publish(mqtt_topic, payload)
        return {"message": "Dados enviados com sucesso para o MQTT."}
    except Exception as e:
        raise HTTPException(status_code=500, detail=str(e))
    

@app.route('/login', methods=['POST'])
def login():
    try:
        username = request.json['username']
        password = request.json['password']

        mycursor = mydb.cursor()
        sql = "SELECT * FROM users WHERE username = %s AND password = %s"
        val = (username, password)
        mycursor.execute(sql, val)
        result = mycursor.fetchone()

        if result:
            role = result[3]  # Assuming the 'role' column is at index 3 in the result
            if role == 0:  # User
                return jsonify({'message': 'Login successful', 'role': 'User'})
            elif role == 1:  # Administrator
                return jsonify({'message': 'Login successful', 'role': 'Administrator'})
        else:
            return jsonify({'message': 'Invalid username or password'})

    except Exception as e:
        return jsonify({'message': 'Error occurred', 'error': str(e)})

@app.route('/register', methods=['POST'])
def register():
    try:
        username = request.json['username']
        id = request.json['id']
        password = request.json['password']
        role = request.json['role']

        mycursor = mydb.cursor()
        sql = "INSERT INTO users (username, id, password, role) VALUES (%s, %s, %s, %s)"
        val = (username, id, password, role)
        mycursor.execute(sql, val)
        mydb.commit()

        return jsonify({'message': 'User registered successfully'})

    except Exception as e:
        return jsonify({'message': 'Error occurred', 'error': str(e)})


@app.route('/users', methods=['GET'])
def get_users():
    try:
        mycursor = mydb.cursor()
        mycursor.execute("SELECT * FROM users")
        result = mycursor.fetchall()

        users = []
        for row in result:
            user = {
                'id': row[0],
                'username': row[1],
                'role': row[3]
            }
            users.append(user)

        return jsonify(users)

    except Exception as e:
        return jsonify({'message': 'Error occurred', 'error': str(e)})

@app.route('/users/<int:user_id>', methods=['DELETE'])
def delete_user(user_id):
    try:
        mycursor = mydb.cursor()
        sql = "DELETE FROM users WHERE id = %s"
        val = (user_id,)
        mycursor.execute(sql, val)
        mydb.commit()

        if mycursor.rowcount > 0:
            return jsonify({'message': 'User deleted successfully'})
        else:
            return jsonify({'message': 'User not found'})

    except Exception as e:
        return jsonify({'message': 'Error occurred', 'error': str(e)})



if __name__=='__main__':
    uvicorn.run(app, host='0.0.0.0',port=7777)
    #app.run(host='0.0.0.0', port=4000
    
    #uvicorn app:app --host 0.0.0.0 --port 8000
