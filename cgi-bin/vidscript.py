#!/usr/bin/env python3
import sys

with open("/home/salaoui/images/directory/3minvid.mp4", "rb") as f:
    data = f.read()

# headers first, as bytes
sys.stdout.buffer.write(b"Content-Type: video/mp4\r\n")
sys.stdout.buffer.write(f"Content-Length: {len(data)}\r\n\r\n".encode())

# then the video data
sys.stdout.buffer.write(data)
sys.stdout.buffer.flush()  # make sure everything is sent


#!/usr/bin/env python3
print("Content-Type: video/mp4")
print()
with open("/home/salaoui/images/directory/3minvid.mp4", "rb") as f:
    data = f.read()
print(f"Content-Length: {len(data)}")

import sys
sys.stdout.buffer.write(data)
