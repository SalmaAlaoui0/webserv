#!/usr/bin/env python3
import socket, struct
s = socket.socket()
s.connect(("127.0.0.1", 8080))
s.sendall(b"GET / HTTP/1.1\r\nHost: localhost\r\nContent-Length: 0\r\n\r\n")
# activer SO_LINGER 0 pour forcer RST au close
s.setsockopt(socket.SOL_SOCKET, socket.SO_LINGER, struct.pack('ii', 1, 0))
s.close()
