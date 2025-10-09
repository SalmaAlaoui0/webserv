#!/usr/bin/env python3
import sys

# --- CGI headers ---
print("Content-Type: text/plain")  # required header
print("Status: 200 OK")            # optional, defaults to 200
print("X-Custom-Header: MyHeader") # example of custom header
print()                             # blank line ends headers

# --- Body ---
print("Hello world")

sys.stdout.flush()
