#!/usr/bin/env python3
import sys

# --- CGI Headers ---
print("Status: 200")
print("Content-Type: text/plain")
print()  # <--- Blank line to separate headers from body

# --- CGI Body ---
print("This is the CGI response body.")
print("You can parse the headers above!")