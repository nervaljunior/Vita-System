from fastapi import FastAPI, HTTPException
from fastapi.middleware.cors import CORSMiddleware
import uvicorn
import mysql.connector
from fastapi import Request, Depends

app_users = FastAPI()

mydb = mysql.connector.connect(
    host="localhost",
    user="root",
    password="nervs",
    database="VITA"
)

app_users.add_middleware(
    CORSMiddleware,
    allow_methods=["*"],
    allow_headers=["*"],
    allow_credentials=True,
    allow_origins=["http://localhost:3000"]
)

@app_users.post("/login")
async def login(request: Request):
    data = await request.json()
    username = data['username']
    password = data['password']

    try:
        mycursor = mydb.cursor()
        sql = "SELECT * FROM users WHERE username = %s AND password = %s"
        val = (username, password)
        mycursor.execute(sql, val)
        result = mycursor.fetchone()

        if result:
            role = result[3]  # Assuming the 'role' column is at index 3 in the result
            if role == 0:  # User
                return {"message": "Login successful", "role": "User"}
            elif role == 1:  # Administrator
                return {"message": "Login successful", "role": "Administrator"}
        else:
            raise HTTPException(status_code=401, detail="Invalid username or password")

    except Exception as e:
        raise HTTPException(status_code=500, detail="Error occurred")

@app_users.post("/register")
async def register(request: Request):
    data = await request.json()
    username = data['username']
    id = data['id']
    password = data['password']
    role = data['role']

    try:
        mycursor = mydb.cursor()
        sql = "INSERT INTO users (username, id, password, role) VALUES (%s, %s, %s, %s)"
        val = (username, id, password, role)
        mycursor.execute(sql, val)
        mydb.commit()

        return {"message": "User registered successfully"}

    except Exception as e:
        raise HTTPException(status_code=500, detail="Error occurred")

@app_users.get("/users")
async def get_users():
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

        return users
    except:
        pass

@app_users.delete("/users/{user_id}")
async def delete_user(user_id: int):
    try:
        mycursor = mydb.cursor()
        sql = "DELETE FROM users WHERE id = %s"
        val = (user_id,)
        mycursor.execute(sql, val)
        mydb.commit()

        if mycursor.rowcount > 0:
            return {"message": "User deleted successfully"}
        else:
            raise HTTPException(status_code=404, detail="User not found")

    except Exception as e:
        raise HTTPException(status_code=500, detail="Error occurred")

if __name__ == '__main__':
    uvicorn.run(app_users, host='0.0.0.0', port=7777)
