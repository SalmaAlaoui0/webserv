#!/usr/bin/env python3

import os
import cgi
import cgitb
import random
import datetime

# Enable nice error messages in browser
cgitb.enable()

print("Content-Type: text/html")
print()  # end of headers

print("<!DOCTYPE html>")
print("<html><head><title>CGI Test Script</title>")
print("<style>")
print("body { font-family: Arial, sans-serif; background: #f4f4f4; padding: 20px; }")
print("table { border-collapse: collapse; width: 100%; }")
print("th, td { border: 1px solid #ccc; padding: 8px; text-align: left; }")
print("th { background-color: #eee; }")
print("</style>")
print("</head><body>")

print("<h1>🚀 Python CGI Test Script</h1>")

# Show current server time
print(f"<p><strong>Server time:</strong> {datetime.datetime.now()}</p>")

# Parse query parameters
form = cgi.FieldStorage()
if form:
    print("<h2>Query Parameters</h2>")
    print("<ul>")
    for key in form.keys():
        print(f"<li>{key} = {form.getvalue(key)}</li>")
    print("</ul>")
else:
    print("<p><em>No query parameters received.</em></p>")

# Show some random numbers
print("<h2>Random Numbers</h2>")
numbers = [random.randint(1, 100) for _ in range(10)]
print("<p>" + ", ".join(map(str, numbers)) + "</p>")

# Dump environment variables
print("<h2>Environment Variables</h2>")
print("<table>")
print("<tr><th>Variable</th><th>Value</th></tr>")
for key, value in sorted(os.environ.items()):
    print(f"<tr><td>{key}</td><td>{value}</td></tr>")
print("</table>")

# ASCII art for fun
print("<pre>")
print(r"""
   (\_/)
   ( •_•)   Bunny approves!
  / >🍪     Have a cookie.
""")
print("</pre>")

print("</body></html>")
