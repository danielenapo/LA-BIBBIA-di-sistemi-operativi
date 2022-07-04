#!/usr/bin/env python3

import socket
import os
import sys
import time

HOST = '127.0.0.1'  # Standard loopback interface address (localhost)
PORT = 8080         # Port to listen on (non-privileged ports are > 1023)
HTDOCS = './htdocs' # Root of Web documents
MIMETYPES=[
    ('.gif', 'image/gif'),
    ('.png', 'image/png'),
    ('.jpg', 'image/jpeg'),
    ('.html', 'text/html'),
]

def parse_request(conn):
    request = ''
    while True:
        request += conn.recv(1024).decode('utf-8')
        if request.find('\r\n\r\n') >0:
            break
    reqline, headers = request.split('\r\n', 1)
    method, url, version = reqline.split(' ', 3)
    return url

def send_dummy_response(conn, url):
    body='<html><body><h1>Requested: %s</h1></body></html>' % url
    resp='HTTP/1.0 200 OK\r\n' + \
        'Connection: close\r\n' + \
        'Content-Type: text/html\r\n' + \
        'Content-Length: %d\r\n\r\n' % len(body) + body
    conn.sendall(resp.encode('utf-8'))
    time.sleep(1)

def send_404(conn, url):
    body='<html><body><h1>%s Not Found!</h1></body></html>' % url
    resp='HTTP/1.0 404 Not Found\r\n' + \
        'Connection: close\r\n' + \
        'Content-Type: text/html\r\n' + \
        'Content-Length: %d\r\n\r\n' % len(body) + body
    conn.sendall(resp.encode('utf-8'))
    time.sleep(1)

def get_mime(fname):
    mime = 'text/plain' # default
    for ext, mimetype in MIMETYPES:
        if fname.endswith(ext):
            mime = mimetype
            break
    return mime

def send_200(conn, fname):
    header = 'HTTP/1.0 200 OK\r\n' + \
        'Connection: close\r\n' + \
        'Content-Length: %d\r\n' % os.path.getsize(fname) + \
        'Content-Type: %s\r\n\r\n' % get_mime(fname)
    conn.sendall(header.encode('utf-8'))
    with open(fname, 'rb') as f:
        l = f.read(1024)
        while (l):
            conn.send(l)
            l = f.read(1024)            
    time.sleep(1)

def send_response(conn, url):
    doc_root = HTDOCS
    fname = doc_root + url
    if not os.path.exists(fname):
        print('file %s not found' % fname)
        send_404(conn, url)
    else:
        print('sending file %s' % fname)
        send_200(conn, fname)

def serve_request(conn):
    #print('Connected by', addr)
    url=parse_request(conn)
    #print(url)
    #send_dummy_response(conn, url)
    send_response(conn, url)
    conn.close()

# main
with socket.socket(socket.AF_INET, socket.SOCK_STREAM) as s:
    s.bind((HOST, PORT))
    s.listen()
    # non-parallel version:
    #conn, addr = s.accept()
    #serve_request(conn)
    # single-request version:
    #while True:
    #    conn, addr = s.accept()
    #    serve_request(conn)
    # fork version:
    while True:
        conn, addr = s.accept()
        child_pid=os.fork()
        if child_pid==0:
            serve_request(conn)
            sys.exit()
