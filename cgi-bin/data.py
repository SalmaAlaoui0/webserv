#!/usr/bin/env python3
import os
import datetime
import requests
import getpass

# Username (like whoami)
user = getpass.getuser()

# Date
today = datetime.datetime.now().strftime("%A, %B %d %Y")

# IP address (using requests instead of curl)
try:
    ip = requests.get("https://ifconfig.me", timeout=5).text.strip()
except Exception:
    ip = "Could not fetch IP"

print(f"Good morning, {user}!")
print(f"Today is {today}.")
print(f"Here's your IP address: {ip}")
