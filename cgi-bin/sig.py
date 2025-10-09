#!/usr/bin/env python3
import os
import time
import signal

# Affiche un header CGI minimal
print("Content-Type: text/html\n")
print("<html><body><h1>Test Signal CGI</h1>")



# S'envoie un signal SIGINT (Ctrl+C)
os.kill(os.getpid(), signal.SIGINT)

# Cette ligne ne sera jamais atteinte
print("</body></html>")

