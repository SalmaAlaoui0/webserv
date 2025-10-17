#!/usr/bin/env python3
import time

# send correct HTTP header with UTF-8 encoding
print("Content-Type: text/plain; charset=utf-8\n")

start = time.time()
while time.time() - start < 7:
    x = 1 + 1

print("✅ Done after ~7 seconds")
