from sqlalchemy.orm import Session
from .database import Base
from sqlalchemy import Column, Integer, String, ForeignKey
from sqlalchemy.orm import relationship

class User(Base):
    __tablename__ = "users"

    id = Column(Integer, primary_key=True, index=True)
    username = Column(String, unique=True, index=True)
    password = Column(String)

    def verify_password(self, plain_password: str):
        
        return self.password == plain_password

def get_user(db: Session, username: str):
    return db.query(User).filter(User.username == username).first()

def create_user(db: Session, username: str, password: str):
    user = User(username=username, password=password)
    db.add(user)
    db.commit()
    db.refresh(user)
    return user

def update_user(db: Session, username: str, new_password: str):
    user = get_user(db, username)
    user.password = new_password
    db.commit()
    db.refresh(user)
    return user

def delete_user(db: Session, username: str):
    user = get_user(db, username)
    db.delete(user)
    db.commit()




class Dados(Base):
    __tablename__ = "dados"

    id = Column(Integer, primary_key=True, index=True)
    user_id = Column(Integer, ForeignKey("users.id"), index=True)
    data = Column(String)  # Armazenar JSON como uma string

    # Relacionamento com o usuário
    user = relationship("User", back_populates="dados")
