#!/usr/bin/env python3

import socket
import sys
import time
import json
import os

def test(data):
    name=json.loads(data)
    try:
        f=open(name["filename"],"r")
        retDict={"filename":name["filename"], "filesize": os.fstat(f.fileno()).st_size, "data":f.read()}
        return (json.dumps(retDict))
    except:
        print("ERROR on opening file")
    finally:
        f.close()


HOST = '127.0.0.1'  # Standard loopback interface address (localhost)
PORT = 2525       # Port to listen on (non-privileged ports are > 1023)
with socket.socket(socket.AF_INET, socket.SOCK_STREAM) as s:
    s.bind((HOST, PORT))
    s.listen(1)
    conn, addr = s.accept()
    #print('Connected by', addr)
    data = conn.recv(1024).decode('utf-8')
    print('Here is the message: %s'% data)
    jsonResponse=(test(data))
    conn.sendall(jsonResponse.encode('utf-8'))
    # socket must be closed by client! sleep for 1 second to wait for the client
    time.sleep(1)
    # otherwise socket goes to TIME_WAIT!
