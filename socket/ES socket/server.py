#!/usr/bin/env python3

import socket
import sys
import time
import os

HOST = '127.0.0.1'  # Standard loopback interface address (localhost)
PORT = 2525        # Port to listen on (non-privileged ports are > 1023)

with socket.socket(socket.AF_INET, socket.SOCK_STREAM) as s:
    s.bind((HOST, PORT))
    s.listen(1)
    while True:
        conn, addr = s.accept()
        child_pid=os.fork()
        if child_pid==0:
            conn.sendall(('Welcome from %s'%socket.gethostname()).encode('utf-8') )
			# socket must be closed by client! sleep for 1 second to wait for the client
            time.sleep(1)
            sys.exit()
