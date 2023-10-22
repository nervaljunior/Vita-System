from flask import Flask, request, jsonify
from flask_cors import CORS
import mysql.connector

app = Flask(__name__)
CORS(app)  # Adiciona o middleware Flask-CORS

mydb = mysql.connector.connect(
    host="localhost",
    user="root",
    password="jbneto9!",
    database="siris"
)

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

if __name__ == '__main__':
    app.run()
