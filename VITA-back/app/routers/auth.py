from fastapi import APIRouter, Depends, HTTPException
from fastapi.security import OAuth2PasswordBearer
from app.models import User, ArduinoData
from app.db import db
from datetime import datetime
from typing import List

router = APIRouter()

# Autenticação de usuário
oauth2_scheme = OAuth2PasswordBearer(tokenUrl="token")

# Rota para registro de usuário
@router.post("/register", response_model=User)
async def register(user: User):
    try:
        # Verifique se o usuário já existe
        existing_user = await db.users.find_one({"username": user.username})
        if existing_user:
            raise HTTPException(status_code=400, detail="Username already registered")

        # Insira o novo usuário no banco de dados
        user_data = user.dict()
        user_data["password"] = user.get_password_hash(user_data["password"])
        result = await db.users.insert_one(user_data)

        return {**user.dict(), "id": result.inserted_id}

    except Exception as e:
        raise HTTPException(status_code=500, detail=str(e))

# Rota para fazer login
@router.post("/login", response_model=User)
async def login(user: User):
    try:
        stored_user = await db.users.find_one({"username": user.username})
        if not stored_user:
            raise HTTPException(status_code=400, detail="User not found")
        
        if not user.verify_password(user.password, stored_user["password"]):
            raise HTTPException(status_code=400, detail="Incorrect password")
        
        return user

    except Exception as e:
        raise HTTPException(status_code=500, detail=str(e))

# Rota para obter dados do Arduino
@router.get("/arduino-data", response_model=List[ArduinoData])
async def get_arduino_data():
    try:
        arduino_data = await db.arduino_data.find().to_list(1000)
        return arduino_data

    except Exception as e:
        raise HTTPException(status_code=500, detail=str(e))
