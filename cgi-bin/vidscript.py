#!/usr/bin/env python3
import sys


with open("/home/salaoui/images/cat.jpg", "rb") as f:
    data = f.read()

# headers first, as bytes
sys.stdout.buffer.write(b"Content-Type: image/jpg\r\n")
sys.stdout.buffer.write(f"Content-Length: {len(data)}\r\n\r\n".encode())

# then the video data
sys.stdout.buffer.write(data)
sys.stdout.buffer.flush()  # make sure everything is sent

# with open("/home/mlabyed/Desktop/video.mp4", "rb") as f:
#     data = f.read()

# # écrire uniquement en mode binaire
# headers = (
#     b"Content-Typeww: videor/mp4\r\n"
#     + f"Content-Length: {len(data)}\r\n".encode()
#     + b"\r\n"
# )

# # envoi dans le bon ordre
# sys.stdout.buffer.write(headers)
# sys.stdout.buffer.write(data)

