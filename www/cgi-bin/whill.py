#!/usr/bin/env python3
import time
import sys

print("Content-Type: text/plain\n")
print("Starting infinite loop...")

# Flush so server can read partial output
sys.stdout.flush()

# Infinite loop
while True:
    print("Still running...", flush=True)
    time.sleep(1)