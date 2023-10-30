from fastapi import APIRouter, Depends, HTTPException, status
from fastapi.security import OAuth2PasswordBearer, OAuth2PasswordRequestForm
from sqlalchemy.orm import Session
from .models import User, get_user, create_user, update_user, delete_user
from .database import get_db
from jose import JWTError, jwt

from datetime import datetime, timedelta
from ..config import SECRET_KEY, ALGORITHM

def create_access_token(data: dict, expires_delta: timedelta = None):
    to_encode = data.copy()
    if expires_delta:
        expire = datetime.utcnow() + expires_delta
    else:
        expire = datetime.utcnow() + timedelta(minutes=15)
    to_encode.update({"exp": expire})
    encoded_jwt = jwt.encode(to_encode, SECRET_KEY, algorithm=ALGORITHM)
    return encoded_jwt

router = APIRouter()
oauth2_scheme = OAuth2PasswordBearer(tokenUrl="token")

@router.post("/login")
async def login(form_data: OAuth2PasswordRequestForm = Depends(), db: Session = Depends(get_db)):
    username = form_data.username
    password = form_data.password
    user = get_user(db, username)

    if user is None or not user.verify_password(password):
        raise HTTPException(
            status_code=status.HTTP_401_UNAUTHORIZED,
            detail="Credenciais inválidas",
        )

    # Gere um token de acesso JWT e retorne como resposta
    access_token = create_access_token(data={"sub": username})
    return {"access_token": access_token, "token_type": "bearer"}

@router.post("/register")
async def register(form_data: OAuth2PasswordRequestForm = Depends(), db: Session = Depends(get_db)):
    username = form_data.username
    password = form_data.password

    user = get_user(db, username)
    if user:
        raise HTTPException(
            status_code=status.HTTP_400_BAD_REQUEST,
            detail="Usuário já registrado",
        )

    user = create_user(db, username, password)
    return {"message": "Usuário registrado com sucesso"}

@router.get("/users")
async def get_users(db: Session = Depends(get_db)):
    users = db.query(User).all()
    user_list = [{"username": user.username} for user in users]
    return user_list

@router.get("/users/{username}")
async def get_user_details(username: str, db: Session = Depends(get_db)):
    user = get_user(db, username)
    if user is None:
        raise HTTPException(
            status_code=status.HTTP_404_NOT_FOUND,
            detail="Usuário não encontrado",
        )
    return {"username": user.username}

@router.put("/users/{username}")
async def update_user_details(username: str, form_data: OAuth2PasswordRequestForm = Depends(), db: Session = Depends(get_db)):
    user = get_user(db, username)
    if user is None:
        raise HTTPException(
            status_code=status.HTTP_404_NOT_FOUND,
            detail="Usuário não encontrado",
        )

    new_password = form_data.password
    user = update_user(db, username, new_password)
    return {"message": "Dados do usuário atualizados com sucesso"}

@router.delete("/users/{username}")
async def delete_user(username: str, db: Session = Depends(get_db)):
    user = get_user(db, username)
    if user is None:
        raise HTTPException(
            status_code=status.HTTP_404_NOT_FOUND,
            detail="Usuário não encontrado",
        )

    delete_user(db, username)
    return {"message": f"Usuário {username} excluído com sucesso"}
