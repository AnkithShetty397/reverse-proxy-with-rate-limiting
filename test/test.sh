#!/bin/bash

# Server address and port
URL="http://localhost:8080"
DATA="DoS Attack"

# Test 1: Basic Connectivity Test
echo "Running Basic Connectivity Test..."
response=$(curl -s -o /dev/null -w "%{http_code}" -X POST $URL -d "$DATA")
if [ "$response" -eq 200 ]; then
    echo "Test 1: Passed - Server responded with 200 OK"
else
    echo "Test 1: Failed - Expected 200 OK, got $response"
fi
echo

# Test 2: Rate Limiting Test (6 requests in 2 seconds allowed)
echo "Running Rate Limiting Test..."
for i in {1..8}
do
    echo "Request $i:"
    curl -X POST $URL -d "$DATA" -w "\n" -s &
done

wait
echo "Test 2: Completed (Should see rate limit exceeded after 6 requests)"
echo

# Sleep for a few seconds to allow for the rate limit window to reset
sleep 3

# Test 3: Sustained Requests over Time
echo "Running Sustained Requests Test (Over time)..."
for i in {1..12}
do
    echo "Request $i:"
    curl -X POST $URL -d "$DATA" -w "\n" -s &
    
    # Pause for 1 second between requests (below WINDOW_SIZE to avoid hitting rate limit)
    sleep 1
done

wait
echo "Test 3: Completed (Requests should be spaced out to avoid rate limit)"
echo

# Test 4: Burst Test (Flood of requests exceeding REQUEST_LIMIT in a short period)
echo "Running Burst Test (Exceeding rate limit)..."
for i in {1..10}
do
    echo "Burst Request $i:"
    curl -X POST $URL -d "$DATA" -w "\n" -s &
done

wait
echo "Test 4: Completed (Expect rate limit exceeded after 6 requests within 2 seconds)"
echo
