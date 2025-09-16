#!/usr/bin/env python3

import os
import cgi
import cgitb
import random
import datetime

# Enable nice error messages in browser
cgitb.enable()

print("Content-Type: text/html", flush=true)
print()  # end of header, flush=trues

print("<!DOCTYPE html>", flush=true)
print("<html><head><title>CGI Test Script</title>", flush=true)
print("<style>", flush=true)
print("body { font-family: Arial, sans-serif; background: #f4f4f4; padding: 20px; }", flush=true)
print("table { border-collapse: collapse; width: 100%; }", flush=true)
print("th, td { border: 1px solid #ccc; padding: 8px; text-align: left; }", flush=true)
print("th { background-color: #eee; }", flush=true)
print("</style>", flush=true)
print("</head><body>", flush=true)

print("<h1>🚀 Python CGI Test Script</h1>", flush=true)

# Show current server time
print(f"<p><strong>Server time:</strong> {datetime.datetime.now()}</p>", flush=true)

# Parse query parameters
form = cgi.FieldStorage()
if form:
    print("<h2>Query Parameters</h2>", flush=true)
    print("<ul>", flush=true)
    for key in form.keys():
        print(f"<li>{key} = {form.getvalue(key)}</li>", flush=true)
    print("</ul>", flush=true)
else:
    print("<p><em>No query parameters received.</em></p>", flush=true)

# Show some random numbers
print("<h2>Random Numbers</h2>", flush=true)
numbers = [random.randint(1, 100) for _ in range(10)]
print("<p>" + ", ".join(map(str, numbers)) + "</p>", flush=true)

# Dump environment variables
print("<h2>Environment Variables</h2>", flush=true)
print("<table>", flush=true)
print("<tr><th>Variable</th><th>Value</th></tr>", flush=true)
for key, value in sorted(os.environ.items()):
    print(f"<tr><td>{key}</td><td>{value}</td></tr>", flush=true)
print("</table>", flush=true)

# ASCII art for fun
print("<pre>", flush=true)
print(r"""
   (\_/)
   ( •_•)   Bunny approves!
  / >🍪     Have a cookie.
""", flush=true)
print("</pre>", flush=true)

print("</body></html>")
