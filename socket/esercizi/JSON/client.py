#!/usr/bin/env python3

import socket
import sys
import json

def makeJSON(name):
	dictionary={"filename":name}
	jsonFormat=json.dumps(dictionary)
	return jsonFormat
	
def makeFile(jsonResponse):
	dictResponse=json.loads(jsonResponse);
	try:
		f=open(dictResponse["filename"],"w")
		f.write(dictResponse["data"])
	finally:
		f.close()
		


HOST=sys.argv[1]
PORT=int(sys.argv[2])
with socket.socket(socket.AF_INET, socket.SOCK_STREAM) as s:
    s.connect((HOST, PORT))
    filename = input('Please enter the file name: ') 
    jsonFormat=makeJSON(filename) 
    s.sendall(jsonFormat.encode('utf-8'))
    data = s.recv(1024)
    s.close()

makeFile(data.decode('utf-8'))
