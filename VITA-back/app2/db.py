from motor.motor_asyncio import AsyncIOMotorClient
from app.models import User, ArduinoData

class Database:
    def __init__(self, url: str, database_name: str):
        self.client = AsyncIOMotorClient(url)
        self.db = self.client[database_name]
        self.users = self.db.users
        self.arduino_data = self.db.arduino_data

db = Database("mongodb://localhost:27017", "mydb")

import sqlalchemy as _sql
import sqlalchemy.ext.declarative as _declarative
import sqlalchemy.orm as _orm

DATABASE_URL = "sqlite:///./database.db"

engine = _sql.create_engine(DATABASE_URL, connect_args={"check_same_thread": False})

SessionLocal = _orm.sessionmaker(autocommit=False, autoflush=False, bind=engine)

Base = _declarative.declarative_base()