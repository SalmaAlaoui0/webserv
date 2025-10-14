#!/usr/bin/env bash

# ────────────────────────────────
# Webserv Tester 🧪
# ────────────────────────────────
# Make sure your server is running on 127.0.0.1:8080
# Usage:
#   ./tester.sh
# ────────────────────────────────

HOST="127.0.0.1"
PORT="8090"
URL="http://${HOST}:${PORT}"

GREEN="\033[1;32m"
RED="\033[1;31m"
YELLOW="\033[1;33m"
NC="\033[0m"

function test_case() {
    local description=$1
    local cmd=$2
    local expected=$3

    echo -e "\n${YELLOW}▶ ${description}${NC}"
    response=$(eval "$cmd" 2>/dev/null)
    code=$(echo "$response" | head -n 1 | awk '{print $2}')
    if [[ "$code" == "$expected" ]]; then
        echo -e "${GREEN}✔ Passed (HTTP $code)${NC}"
    else
        echo -e "${RED}✘ Failed${NC} (expected HTTP $expected, got ${code:-No Response})"
        echo "----- Raw response -----"
        echo "$response"
        echo "-------------------------"
    fi
}

# ────────────────────────────────
# 1️⃣ Basic GET
# ────────────────────────────────
test_case "GET / (root)" \
    "curl -s -D - ${URL}/ -o /dev/null" \
    "200"

# ────────────────────────────────
# 2️⃣ Non-existent page
# ────────────────────────────────
test_case "GET /doesnotexist" \
    "curl -s -D - ${URL}/doesnotexist -o /dev/null" \
    "404"

# ────────────────────────────────
# 3️⃣ POST with body
# ────────────────────────────────
test_case "POST /post with small body" \
    "curl -s -D - -X POST -d 'name=test' ${URL}/post -o /dev/null" \
    "200"

# ────────────────────────────────
# 4️⃣ Wrong method
# ────────────────────────────────
test_case "PATCH / (method not allowed)" \
    "curl -s -D - -X PATCH ${URL}/ -o /dev/null" \
    "405"

# ────────────────────────────────
# 5️⃣ Large body (should trigger 413 if size limit is low)
# ────────────────────────────────
big_body=$(head -c 200000 < /dev/zero | tr '\0' 'A')  # 200 KB body
test_case "POST /upload with big body" \
    "curl -s -D - -X POST -d \"$big_body\" ${URL}/upload -o /dev/null" \
    "413"

# ────────────────────────────────
# 6️⃣ CGI test (if cgi-bin/script.py exists)
# ────────────────────────────────
if [[ -x "./cgi-bin/script.py" ]]; then
    test_case "CGI script execution" \
        "curl -s -D - ${URL}/cgi-bin/script.py -o /dev/null" \
        "200"
else
    echo -e "${YELLOW}⚠ CGI test skipped (no ./cgi-bin/script.py)${NC}"
fi

# ────────────────────────────────
# 7️⃣ Directory listing (if autoindex enabled)
# ────────────────────────────────
test_case "GET /directory/ (autoindex)" \
    "curl -s -D - ${URL}/directory/ -o /dev/null" \
    "200"

# ────────────────────────────────
# 8️⃣ Keep-alive & multiple requests
# ────────────────────────────────
echo -e "\n${YELLOW}▶ Testing persistent connections${NC}"
curl -s -D - --keepalive-time 5 ${URL}/ -o /dev/null >/dev/null
curl -s -D - --keepalive-time 5 ${URL}/ -o /dev/null >/dev/null
echo -e "${GREEN}✔ Keep-alive requests sent${NC}"

# ────────────────────────────────
# End
# ────────────────────────────────
echo -e "\n${GREEN}✅ Testing complete${NC}"