#!/usr/bin/env python3

body = "===HELLO WORLD==="

# Send headers
print("Content-Type: text/plain")
print(f"Content-Length: 5")
print()  # End of headers

# Send exactly body length
print(body, end="")  # no extra newline!
