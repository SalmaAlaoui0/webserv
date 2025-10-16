#!/usr/bin/env python3
import sys

print("Content-Type: text/plain\n")

# Generate ~1 MB of "A"s
sys.stdout.write("A" * (1004 * 1024))
