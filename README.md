# Vita-System
sistema de controle e plataforma de recebimento de dados do VITA


# Projeto VITA - Monitoramento e Controle de Dispositivo Médico

![VITA Logo](logo.png)(sem logo// AINDA)

O Projeto VITA é uma iniciativa que visa desenvolver um sistema embarcado inovador para o monitoramento e controle de dispositivos médicos, utilizando tecnologias de Internet das Coisas (IoT) e integração de hardware e software. O objetivo central é permitir um acompanhamento eficiente de dispositivos médicos, fornecendo uma interface gráfica amigável para visualização em tempo real, além de comunicação segura com um banco de dados para análise posterior.

## Características Principais

- Monitoramento em Tempo Real: Através da comunicação bidirecional entre o dispositivo médico e a interface web, os dados são apresentados em tempo real, possibilitando um acompanhamento preciso.

- Controle Remoto: O sistema permite o controle remoto do dispositivo médico por meio da interface gráfica, possibilitando ajustes em parâmetros e configurações.

- Armazenamento de Dados: Os dados coletados são armazenados de forma segura em um banco de dados ou arquivo CSV, permitindo análises futuras e a geração de relatórios.

- Tecnologias de Ponta: Utilização de protocolos MQTT e HTTP para a comunicação entre o hardware e a interface web, demonstrando o uso de tecnologias avançadas.

## Como Utilizar

1. **Instalação:**
   - Clone este repositório em sua máquina local.
   - Instale as dependências necessárias listadas no arquivo `requirements.txt`.

2. **Configuração:**
   - Siga as instruções na documentação do projeto para configurar o Access Point no Arduino Wi-Fi R2.

3. **Execução:**
   - Execute o servidor da interface gráfica executando o comando `python app.py`.
   - Acesse a interface web no navegador utilizando o endereço `http://localhost:3000`.

4. **Utilização:**
   - Visualize os dados em tempo real na interface gráfica.
   - Faça ajustes e controle o dispositivo médico remotamente.

## Contribuição

Contribuições são bem-vindas! Se você encontrou algum bug ou tem alguma sugestão de melhoria, por favor, abra uma issue ou envie um pull request.

## Créditos

Este projeto foi desenvolvido por Nerval Junior e faz parte de um trabalho acadêmico/pesquisa. Para mais informações, consulte os documentos e referências mencionados.

## Licença

Este projeto é licenciado sob a [Licença MIT](LICENSE).


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

