project_vita_system/
├── .git
│
├── Backend/
│   ├── venv/
│   ├── App/
│   |   ├── __init__.py
│   |   ├── main.py
│   |   ├── models.py
│   |   ├── db.py
│   |   ├── routers/
│   |   │   ├── __init__.py
│   |   │   ├── auth.py
│   |   │   ├── dashboard.py
|   │   ├──__pycache__/
│   |   ├── db.py
│   |   ├── static/
│   |   ├── templates
│   ├── run.py
│   ├── requirements.txt
│   ├── .gitignore
│   ├── README.md 
│   ├──__pycache__/
|
|
│
├── Frontend/
│   ├── .next
│   ├── node_modules
│   ├── pages
│   ├── public
│   ├── src
│   ├── styles
│   ├── .gitignore
│   ├── next.config.js
│   ├── packege.json
│   ├── packege-lock.json
│   ├── README.md
|
|
├── documents/
│   ├── architecture.md
│   ├── sistema de login.drawio
│   ├── manual de uso.pdf
│   ├── user_manual.md
|
│
├── Firmware 
│   ├── vita_pneuma
│          ├── vita_pneuma.ino
|
├── .gitignore 
├── LICENSE
├── README.md




flask run 


  +------------------+     +----------------+     +----------------+
  |   Solicitação   | --> |   Controlador  | --> |      Modelo    |
  |   HTTP (Rota)   |     |    (Função)    |     |     (Banco)    |
  +------------------+     +----------------+     +----------------+
                                        |
                                        v
                                  +----------------+
                                  |      Visão     |
                                  |    (Resposta)  |
                                  +----------------+
