from fastapi.middleware.cors import CORSMiddleware
from pydantic import BaseModel
from jwt import encode as jwt_encode
from fastapi.security import HTTPBearer, HTTPAuthorizationCredentials
from fastapi import Depends, HTTPException ,FastAPI
import mysql.connector
from mysql.connector import errorcode
import paho.mqtt.client as mqtt
import jwt
from jwt.exceptions import DecodeError


app = FastAPI()

app.add_middleware(
    CORSMiddleware,
    allow_origins=["http://localhost:3000"],  
    allow_credentials=True,
    allow_methods=["*"],
    allow_headers=["*"],
)


mqtt_broker = "broker.mqttdashboard.com"
mqtt_port = 1883
mqtt_topic = "wokwi-weather"

received_data = {}

class User(BaseModel):
    username: str = None  
    email: str
    password: str

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


SECRET_KEY = "your-secret-key-goes-here"
security = HTTPBearer()

@app.get("/")
def homepage():
    return {"message": "Welcome to the homepage"}

@app.post("/login")
def login(user: User):
    
    query = "SELECT * FROM users WHERE email = %s AND password = %s"
    cursor.execute(query, (user.email, user.password))
    user_data = cursor.fetchone()

    if user_data:
        
        token = generate_token(user.email)
        user_dict = {
            "username": user_data[1],  
            "email": user_data[2]  
        }
        user_dict["token"] = token
        return user_dict

    return {"message": "Invalid email or password"}

@app.post("/register")
def register(user: User):

    query = "SELECT * FROM users WHERE email = %s"
    cursor.execute(query, (user.email,))
    existing_user = cursor.fetchone()

    if existing_user:
        return {"message": "User already exists"}


    query = "INSERT INTO users (username, email, password) VALUES (%s, %s, %s)"
    cursor.execute(query, (user.username, user.email, user.password))
    connection.commit()


    token = generate_token(user.email)
    user_dict = {
        "username": user.username,
        "email": user.email
    }
    user_dict["token"] = token
    return user_dict

@app.get("/api/user")
def get_user(credentials: HTTPAuthorizationCredentials = Depends(security)):
    token = credentials.credentials
    user_email = decode_token(token)
    user_id = get_user_id_by_email(user_email)
    if user_id is not None:
        user_data = get_user_data_by_id(user_id)
        if user_data:
            return {
                "username": user_data[0],
                "email": user_data[1]
            }
    raise HTTPException(status_code=401, detail="Invalid token")


@app.get("/api/users/{username}")
def get_user_details(username: str):
    query = "SELECT username, email FROM users WHERE username = %s"
    cursor.execute(query, (username,))
    user_data = cursor.fetchone()
    if user_data:
        user_details = {
            "username": user_data[0],
            "email": user_data[1]
        }
        return user_details
    raise HTTPException(status_code=404, detail="User not found")

@app.put("/api/users/{username}")
def update_user_details(username: str, user: User):
    query = "UPDATE users SET password = %s WHERE username = %s"
    cursor.execute(query, (user.password, username))
    connection.commit()
    return {"message": "User details updated successfully"}

@app.delete("/api/users/{username}")
def delete_user(username: str):
    query = "DELETE FROM users WHERE username = %s"
    cursor.execute(query, (username,))
    connection.commit()
    return {"message": f"User {username} deleted successfully"}


def on_message(client, userdata, message):
    payload = message.payload.decode("utf-8")
    received_data[mqtt_topic] = payload
    print(f"Received MQTT data on topic {mqtt_topic}: {payload}")


mqtt_client = mqtt.Client()
mqtt_client.on_message = on_message
mqtt_client.connect(mqtt_broker, mqtt_port)
mqtt_client.subscribe(mqtt_topic)
mqtt_client.loop_start()

@app.get('/get_data')
async def get_mqtt_data():
    return {
        "Mensagem_MQTT": received_data.get(mqtt_topic, "Nenhum dado recebido via MQTT")
    }

@app.post("/save_data")
async def save_mqtt_data(data: MqttData, credentials: HTTPAuthorizationCredentials = Depends(security)):

    token = credentials.credentials

    user_email = decode_token(token)

    user_id = get_user_id_by_email(user_email)

    if user_id is not None:
        query = "INSERT INTO dados (user_id, data) VALUES (%s, %s)"
        cursor.execute(query, (user_id, data.data))
        connection.commit()
        return {"message": "Data received and saved successfully"}
    else:
        raise HTTPException(status_code=401, detail="User not found")

def decode_token(token: str) -> str:
    try:
        payload = jwt.decode(token, 'your-secret-key-goes-here', algorithms=["HS256"])
        return payload['email']
    except DecodeError:
        return None

def get_user_id_by_email(email: str) -> int:
    query = "SELECT id FROM users WHERE email = %s"
    cursor.execute(query, (email,))
    result = cursor.fetchone()
    if result:
        return result[0]
    return None


def get_user_data_by_id(user_id: int):
    
    query = "SELECT username, email FROM users WHERE id = %s"
    cursor.execute(query, (user_id,))
    return cursor.fetchone()


def generate_token(email: str) -> str:
    payload = {"email": email}
    token = jwt_encode(payload, SECRET_KEY, algorithm="HS256")
    return token

if __name__ == "__main__":
    import uvicorn
    uvicorn.run(app, host="0.0.0.0", port=8000)
