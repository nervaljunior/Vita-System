# config.py
from pydantic import BaseSettings

class Settings(BaseSettings):
    database_url: str = "mysql://root@localhost:3307/paradigmas"

settings = Settings()
