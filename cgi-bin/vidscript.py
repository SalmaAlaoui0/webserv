#!/usr/bin/env python3
print("Content-Type: video/mp4")
print()
with open("/home/salaoui/goinfre/1sec.mp4", "rb") as f:
    data = f.read()
print(f"Content-Length: {len(data)}")

import sys
sys.stdout.buffer.write(data)
