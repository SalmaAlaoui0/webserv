#!/bin/bash
echo "Good morning, $(whoami)!"
echo "Today is $(date '+%A, %B %d %Y')."
echo "Here's your IP address: $(curl -s ifconfig.me)"
