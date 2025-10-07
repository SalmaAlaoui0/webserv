#!/usr/bin/env python3

# print("Content-Type: text/plain")
# print("Status: 404 OK")  
# print()  # <- obligatoire pour séparer les headers du corps
# print("Hello, CGI works!")

#!/usr/bin/env python3

import os
import cgi
import cgitb
import random
import datetime

# Enable CGI error reporting
cgitb.enable()

# Préparation du contenu HTML
html = []

html.append("<!DOCTYPE html>")
html.append("<html><head><title>CGI Test Script</title>")
html.append("<style>")
html.append("body { font-family: Arial, sans-serif; background: #f4f4f4; padding: 20px; }")
html.append("table { border-collapse: collapse; width: 100%; }")
html.append("th, td { border: 1px solid #ccc; padding: 8px; text-align: left; }")
html.append("th { background-color: #eee; }")
html.append("</style>")
html.append("</head><body>")

html.append("<h1>🚀 Python CGI Test Script</h1>")
html.append(f"<p><strong>Server time:</strong> {datetime.datetime.now()}</p>")

# Query params
# form = cgi.FieldStorage()
# if form:
#     html.append("<h2>Query Parameters</h2><ul>")
#     for key in form.keys():
#         html.append(f"<li>{key} = {form.getvalue(key)}</li>")
#     html.append("</ul>")
# else:
#     html.append("<p><em>No query parameters received.</em></p>")

# Random numbers
numbers = [random.randint(1, 100) for _ in range(10)]
html.append("<h2>Random Numbers</h2>")
html.append("<p>" + ", ".join(map(str, numbers)) + "</p>")

# Environment
html.append("<h2>Environment Variables</h2>")
html.append("<table><tr><th>Variable</th><th>Value</th></tr>")
for key, value in sorted(os.environ.items()):
    html.append(f"<tr><td>{key}</td><td>{value}</td></tr>")
html.append("</table>")

# ASCII art
html.append("<pre>")
html.append(r"""
   (\_/)
   ( •_•)   Bunny approves!
  / >🍪     Have a cookie.
""")
html.append("</pre>")
html.append("</body></html>")

# Convertir en une seule chaîne
body = "\n".join(html)

# ==== Impression des headers CGI ====
print("Content-Type: text/html; charset=utf-8")
print(f"Content-Length: {len(body.encode('utf-8'))}")
print()  # Fin des headers (obligatoire)

# ==== Impression du corps ====
print(body)
