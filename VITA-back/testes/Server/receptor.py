import serial
import datetime
from datetime import datetime
from flask import Flask, request, jsonify
import requests
import json



class Antena:
    def __init__(self, port, baudrate, bytesize, parity, stopbits):
        self.port = port
        self.baudrate = baudrate
        self.bytesize = bytesize
        self.parity = parity
        self.stopbits = stopbits
        self.serial = None
        self.buffer = []
        # self.api = PostagemAPI(url_api)
    
    def open(self):
        try:
            self.serial = serial.Serial(self.port, self.baudrate, self.bytesize, self.parity, self.stopbits)
            print(f"Porta serial {self.port} aberta com sucesso!")
        except serial.SerialException as e:
            print(f"Erro ao abrir a porta serial {self.port}: {e}")
    
    def close(self):
        if self.serial is not None and self.serial.is_open:
            self.serial.close()
            print(f"Porta serial {self.port} fechada.")
    
    def read_pacote(self):
        if self.serial is not None and self.serial.is_open:

            self.serial.timeout = 0.1

            #while True:
            try:
                pacote = self.serial.read(11)
                print(pacote)
                pacote_bruto = pacote.decode('utf-8')
            except:
                pacote_bruto = "+00000000"

            print(f"Posição lida na antena {self.port}: {pacote}")
            return pacote_bruto
        else:
            print("A porta serial não está aberta.")

    def conver_posicao(self, pacote_bruto):
        

        for i in range(len(pacote_bruto)-1):
            
            if pacote_bruto[i] == '+':
                elevacao = int(pacote_bruto[i+4]+pacote_bruto[i+3]+pacote_bruto[i+2]+pacote_bruto[i+1])
                azimute = int(pacote_bruto[i+8]+pacote_bruto[i+7]+pacote_bruto[i+6]+pacote_bruto[i+5])
        
                conver_elevacao = elevacao*0.043945862
                conver_azimute = azimute*0.043945862

                posicao_convertida = [conver_elevacao, conver_azimute]
                hora_atual = datetime.now()
                hora_atual = hora_atual.strftime('%H:%M:%S')
                
                posicao_convertida.append(hora_atual)

                return posicao_convertida

    def obter_buffer(self):
        return self.buffer
    
    def atualizar_buffer(self):
        pacote_bruto = self.read_pacote()
        posicao_convertida = self.conver_posicao(pacote_bruto)


        if posicao_convertida != None:
            print("ta aqui")
            self.buffer.append(posicao_convertida)
        else:
            self.buffer.append([0,0,0])


class ProgramaAntena:
    def __init__(self):
        self.antenas = []
        self.buffer_posicoes = []
        
    
    def adicionar_antena(self, antena):
        self.antenas.append(antena)
    
    def iniciar_leitura(self):

        #while True:
            for antena in self.antenas:
                antena.open()

                antena.atualizar_buffer()
                pacote = antena.read_pacote()
                posicao = antena.conver_posicao(pacote)
                antena.close()
                self.buffer_posicoes.append(posicao)

            # for antena in self.antenas:
            #     print(f"Antena {antena.port}: {antena.obter_buffer()}")
    
     
    def exibir_buffer_posicoes(self):
        print("Buffer de Posições:")
        print(self.buffer_posicoes)
        # for pacote in self.buffer_posicoes:
        #     print(pacote)

class ListToJsonConverter:
    def __init__(self, data):
        self.data = data


    def to_json(self):
        dados = []
        azimute = []
        
        for i in range(len(self.data)):
            azimute.append(self.data[i][0])
        
        
        elevacao= []
        for i in range(len(self.data)):
            elevacao.append(self.data[i][1])

        temp = []
        for i in range(len(self.data)):
            temp.append(self.data[i][2])

            
        return ([azimute,elevacao,temp])
    
class PostagemAPI:
    def __init__(self, url):
        self.url = url

    def postar_dados(self, dados):
        print("ta chegando aqui")
        print(type(dados))
        headers = {'Content-Type': 'application/json'}
        response =  requests.post(self.url, dados, headers=headers)
        print(response)
        if response.status_code == 200:
            print("Dados postados com sucesso!")
        else:
            print("Falha ao postar os dados. Código de status:", response.status_code)



programa = ProgramaAntena()

# Adicionar antenas
antena1 = Antena("COM11", 4800,7,'E', 2)
antena2 = Antena("COM21", 4800,7,'E', 2)
programa.adicionar_antena(antena1)
programa.adicionar_antena(antena2)




url = 'http://localhost:5000/buffer'
# Iniciar leitura das posições

while True:
    programa.iniciar_leitura()
    print("está chegando aqi")
    # Exibir o buffer de posições
    #programa.exibir_buffer_posicoes()
    buffer1 = antena1.obter_buffer()
    buffer2 = antena2.obter_buffer()

    converter = ListToJsonConverter(buffer1)
    converter2 = ListToJsonConverter(buffer2)
    json_data = converter.to_json()
    print(json_data)
    json_data2 = converter2.to_json()

    data2 = {
        "antena1": json_data,
        "antena2": json_data2
    }

    json_convert_again= json.dumps(data2)
    print(data2)
    post = PostagemAPI(url)
    post.postar_dados(json_convert_again)
    #post.postar_dados(json_data2)
    
    #print(buffer1)

    # response = requests.post(url, json = buffer1)

    # if response.status_code == 200:
    #     print('Dados enviados com sucesso')
    # else:
    #     print('Erro ao enviar os dados:', response.status_code)

             