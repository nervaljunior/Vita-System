import socket

SERVER_ADDRESS='0.0.0.0'
SERVER_PORT=8000

socket_servidor= socket.socket(socket.AF_INET,socket.SOCK_STREAM)

socket_servidor.bind((SERVER_ADDRESS,SERVER_PORT))


socket_servidor.listen()

print(f'servidor ouvindo em {SERVER_ADDRESS}:')

socket_cliente, cliente_addr=socket_servidor.accept()

print(f'cliente conectado com sucesso. {cliente_addr[0]}:{cliente_addr[1]}')


socket_cliente.close()