import json
import socket
from time import sleep

HOST = '127.0.0.1'
PORT = 5559

h = "HTTP/1.1 200 OK\r\nAccess-Control-Allow-Origin: *\r\nConnection: keep-alive\r\nRequest Method: POST\r\nKeep-Alive: timeout=5\r\nContent-Type: json/application; charset=utf-8\r\n\r\n"


sock = socket.socket(socket.AF_INET, socket.SOCK_STREAM)

sock.bind((HOST, PORT))
sock.listen()

print("Server started")

data1 = {"id": 77, "name": "billy"}
data2 = {"id": 44, "name": "berry"}
data3 = {"id": 11, "name": "smoker"}

switch = 'a'
while (switch != 'q'):
    #Receive connection
    conn, addr = sock.accept()

    #Receive 1 byte from client
    # indicating what to send back
    data = conn.recv(1)
    
    print("code", chr(data[0]))
    d = data3
    #Process Data
    if chr(data[0]) == 'a':
        d = data1
    elif chr(data[0]) == 'b':
        d = data2
    elif chr(data[0]) == 'q':
        switch = 'q'

    #Send back jSon dump
    jData = json.dumps(d)
    conn.send(bytes(jData, encoding="utf-8"))

    print('DATA SENT')
    
print("Closing server")
sock.close()