#!/usr/bin/env python3

import socket
import sys
import time

#funzione per svolgere il riconoscimento della classe, calcolo netid e broadcast
#per indirizzi A,B,C
#e trova eventuali errori nella stringa di input
def calculate(ip):
	ips=ip.split('.')
	#controlli su ogni byte validità valori
	if len(ips)!=4:
		return("error")
	for ip in ips:
		if int(ip)<0 or int(ip)>255:
			return("error")
	#riconoscimento classe
	c=int(ips[0])
	netid=""
	broadcast=""
	if c<=127:
		classe="A"
		netid=ips[0]+".0.0.0"
		broadcast=ips[0]+".255.255.255"
	elif c<=191:
		classe="B"
		netid=ips[0]+"."+ips[1]+".0.0"
		broadcast=ips[0]+"."+ips[1]+".255.255"
	elif c<=223:
		classe="C"
		netid=ips[0]+"."+ips[1]+"."+ips[2]+".0"
		broadcast=ips[0]+"."+ips[1]+"."+ips[2]+".255"
	elif c<=239:
		classe="D"
	elif c<=255:
		classe="E"
	return classe+" "+netid+" "+broadcast	



HOST = '127.0.0.1'  # Standard loopback interface address (localhost)
PORT = 1024       # Port to listen on (non-privileged ports are > 1023)

with socket.socket(socket.AF_INET, socket.SOCK_STREAM) as s:
	s.bind((HOST, PORT))
	s.listen(1)
	conn, addr = s.accept()
	data = conn.recv(1024)
	try:
		sendstring = calculate(data.decode('utf-8'))
	except:
		sendstring = "error"
	conn.sendall(sendstring.encode('utf-8'))
	# socket must be closed by client! sleep for 1 second to wait for the client
	time.sleep(1)
	# otherwise socket goes to TIME_WAIT!
