from fastapi import APIRouter, Depends, HTTPException, Request, status
from fastapi.security import OAuth2PasswordBearer, OAuth2PasswordRequestForm
from app.database import fake_users_db

router = APIRouter()
oauth2_scheme = OAuth2PasswordBearer(tokenUrl="token")

@router.post("/login")
async def login(request: Request, form_data: OAuth2PasswordRequestForm = Depends()):
    username = form_data.username
    password = form_data.password
    user = fake_users_db.get(username)

    if user is None or user["password"] != password:
        raise HTTPException(
            status_code=status.HTTP_401_UNAUTHORIZED,
            detail="Credenciais inválidas",
        )

    return {"access_token": username, "token_type": "bearer"}

@router.post("/register")
async def register(request: Request, form_data: OAuth2PasswordRequestForm = Depends()):
    username = form_data.username
    password = form_data.password

    if username in fake_users_db:
        raise HTTPException(
            status_code=status.HTTP_400_BAD_REQUEST,
            detail="Usuário já registrado",
        )

    fake_users_db[username] = {
        "username": username,
        "password": password,
    }

    return {"message": "Usuário registrado com sucesso"}

@router.get("/users")
async def get_users():
    users = [{"username": user["username"]} for user in fake_users_db.values()]
    return users

@router.delete("/users/{username}")
async def delete_user(username: str):
    if username not in fake_users_db:
        raise HTTPException(
            status_code=status.HTTP_404_NOT_FOUND,
            detail="Usuário não encontrado",
        )

    del fake_users_db[username]
    return {"message": f"Usuário {username} excluído com sucesso"}
