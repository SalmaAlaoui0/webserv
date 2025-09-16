#!/bin/bash

# --- CGI Headers ---
echo "Content-Type: text/html"
echo ""  # End of headers

# --- HTML Start ---
echo "<!DOCTYPE html>"
echo "<html><head><title>Long Bash CGI Test</title>"
echo "<style>"
echo "body { font-family: monospace; background: #f0f0f0; padding: 10px; }"
echo "table { border-collapse: collapse; width: 100%; }"
echo "th, td { border: 1px solid #ccc; padding: 4px; }"
echo "th { background-color: #ddd; }"
echo "</style>"
echo "</head><body>"
echo "<h1>🚀 Long Bash CGI Test Script</h1>"

# --- Simulate slow output in chunks ---
for i in {1..50}; do
    echo "<p>Chunk #$i: Random number: $RANDOM</p>"
    sleep 0.1  # slow down output to test server reading chunks
done

# --- Generate a large table ---
echo "<h2>Environment Variables Table</h2>"
echo "<table>"
echo "<tr><th>Variable</th><th>Value</th></tr>"
for var in $(printenv | sort); do
    key=$(echo "$var" | cut -d= -f1)
    val=$(echo "$var" | cut -d= -f2-)
    echo "<tr><td>$key</td><td>$val</td></tr>"
done
echo "</table>"

# --- ASCII Art ---
echo "<pre>"
echo "   (\_/)"
echo "   ( •_•)  Bunny approves!"
echo "  / >🍪    Have a cookie."
echo "</pre>"

# --- Footer ---
echo "<p>Script finished at $(date)</p>"
echo "</body></html>"
