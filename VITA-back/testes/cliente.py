# -*- coding: utf8 -*-

import socket 

SERVER_ADDRESS='localhost'
SERVER_PORT=8000

socket_cliente= socket.socket(socket.AF_INET,socket.SOCK_STREAM)

socket_cliente.connect((SERVER_ADDRESS,SERVER_PORT))

print(f'conexãoo estabelecida com sucesso')

socket_cliente.close()