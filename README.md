# C-Socket Chat System

![Project Status](https://img.shields.io/badge/status-active-brightgreen)  
![License](https://img.shields.io/badge/license-MIT-blue.svg)

## 📖 Sobre o Projeto

O **C-Socket Chat System** é um sistema de chat de múltiplos canais baseado em sockets implementado em C, que permite a comunicação entre vários clientes conectados a um servidor. Este sistema oferece uma interface interativa no terminal, onde os usuários podem trocar mensagens em tempo real em canais diferentes, cada um com seu histórico de mensagens. Além disso, o projeto demonstra como trabalhar com **multithreading**, **sockets** e **sincronização de threads** com o uso de **mutexes**.

Este sistema foi criado para ser avaliado como Trabalho 1 da matéria SSC0641 - Redes de Computadores.

## 🎯 Funcionalidades

- **Chat de múltiplos canais**: até 5 canais diferentes onde os usuários podem se conectar e trocar mensagens.
- **Multithreading**: cada cliente é tratado em uma thread separada, garantindo que múltiplos clientes possam se comunicar simultaneamente.
- **Troca de canal**: um cliente pode mudar de canal durante a execução, digitando `SAIR` e selecionando um novo canal.
- **Controle de nomes de usuários**: verifica se o nome de usuário já está em uso e solicita outro nome se necessário.
- **Logs do servidor**: as interações do servidor, incluindo conexões e mensagens trocadas, são registradas em um arquivo de log.

## 🛠️ Estrutura do Projeto

├── bin/                   # Diretório de binários (servidor e cliente)  
├── build/                 # Diretório dos arquivos compilados  
├── include/               # Arquivos de cabeçalho (.h)  
│   ├── globals.h  
│   ├── logger.h  
│   └── my_socket.h  
├── logs/                  # Diretório de logs do servidor  
├── src/                   # Código-fonte do sistema  
│   ├── client.c           # Código-fonte do cliente  
│   ├── globals.c          # Variáveis globais e constantes  
│   ├── logger.c           # Funções de logging  
│   ├── my_socket.c        # Funções de gerenciamento de socket e comunicação  
│   └── server.c           # Código-fonte do servidor  
├── Makefile               # Arquivo Make para automatizar a compilação  
└── README.md              # Arquivo de documentação do projeto

## 🚀 Como executar o projeto

### Pré-requisitos

Antes de iniciar, você precisa ter o **GCC** (GNU Compiler Collection) instalado em seu sistema para compilar o código.

### Instruções de Instalação

1. **Utilize o S.O Ubuntu 20.04 ou o WSL no Windows.**

2. **Clone o repositório**:
    ```bash
    git clone https://github.com/seu-usuario/c-socket-chat.git  
    cd c-socket-chat
    ```

3. **Compilação**:

   Compile o servidor e o cliente usando o **Makefile**:
    ```bash
    make
    ```

   Isso criará os executáveis no diretório `bin/` para o cliente e servidor.

4. **Executando o servidor**:

   No terminal, execute o servidor com o seguinte comando:
    ```bash
    cd bin/
    ./server
    ```

   O servidor será iniciado na porta **8080** (ou na porta definida em `globals.h`).

5. **Executando o cliente**:

   Em outro terminal, execute o cliente para se conectar ao servidor:

    ```bash
    cd bin/
    ./client
    ```

   Você será solicitado a inserir seu nome de usuário e escolher um canal de chat.

### Limpeza do projeto

Para remover os binários e arquivos de build, use o comando:

```bash
make clean
```

## 📋 Funcionalidades e Interações

### Fluxo de uso do cliente

1. **Introdução**: Ao iniciar o cliente, você verá uma introdução com uma tela de carregamento.
2. **Nome de usuário**: O cliente solicitará um nome de usuário.
3. **Seleção de canal**: Escolha um canal (de 1 a 5) para se conectar.
4. **Envio de mensagens**: Após escolher um canal, você poderá começar a enviar mensagens.
5. **Troca de canal**: Para trocar de canal, digite `SAIR` e escolha um novo canal.

### Logs

O servidor mantém um log de todas as interações em um arquivo de log armazenado no diretório `logs/`. O log inclui conexões, mensagens recebidas e erros.

### Troca de canal

Se o usuário digitar exatamente "SAIR", ele será desconectado do canal atual e terá a opção de selecionar outro canal. O histórico de mensagens será mostrado ao entrar no novo canal.

### Erros tratados

- **Nome de usuário duplicado**: Se o nome já estiver em uso, o cliente é notificado e solicitado a fornecer outro nome.
- **Canal inválido**: Se o usuário escolher um número de canal inválido, ele será solicitado a escolher um valor correto entre 1 e 5.

## 📚 Estrutura dos Arquivos

### 1. **server.c**

Gerencia as conexões dos clientes e manipula a lógica de comunicação entre os canais. Usa **multithreading** para suportar múltiplos clientes simultâneos.

### 2. **client.c**

Interface do cliente que permite enviar e receber mensagens. O cliente se conecta ao servidor e pode trocar de canal, visualizando o histórico de mensagens do novo canal escolhido.

### 3. **my_socket.c / my_socket.h**

Implementa todas as funções de manipulação de sockets, como criar conexões, aceitar conexões e enviar/receber mensagens.

### 4. **logger.c / logger.h**

Gerencia o sistema de logs do servidor, registrando eventos importantes, como conexões e mensagens trocadas.

### 5. **globals.c / globals.h**

Define variáveis globais, como arrays de clientes conectados, canais e buffers.

