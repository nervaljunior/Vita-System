from datetime import datetime
from sqlalchemy import create_engine
from sqlalchemy.orm import sessionmaker, Session
from sqlalchemy.ext.declarative import declarative_base


DB_PATH = "backend/utils/basedados.db"

#https://fastapi.tiangolo.com/tutorial/sql-databases/
def criar_engine(DB_PATH):
    DB_URL = f"sqlite:///{DB_PATH}"
    engine = create_engine(DB_URL, connect_args={"check_same_thread": False})
    print(f"[{datetime.now()}] [SERVIDOR] DB CONECTADO: ", DB_URL)
    return engine


def criar_sessao(engine):
    session_maker = sessionmaker(autocommit=False, autoflush=True, bind=engine)
    session = session_maker()
    return session

Base = declarative_base()
