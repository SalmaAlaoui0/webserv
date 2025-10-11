#!/usr/bin/env bash

# CGI requires a header before any output
echo "Content-Type: text/plain"
echo

# Simulate a very long-running CGI
sleep 1222222222
echo "This should never be reached if timeout works."
