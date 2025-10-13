#!/usr/bin/env python3
import time

# CGI requires a header before output
print("Content-Type: text/plain\n")

# Simulate a very long-running CGI
# print("This should never be reached if timeout works.")
time.sleep(2)

