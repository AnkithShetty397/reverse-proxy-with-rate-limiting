# Server Test Suite

This directory contains a comprehensive test script (`test.sh`) for testing the server's rate-limiting and connection-handling capabilities.

## Overview

The `test.sh` script is designed to test the server's behavior according to the configuration specified in `config.hpp`:

- **Port**: `8080`
- **Rate Limit**: 6 requests per 2 seconds (`WINDOW_SIZE = 2`, `REQUEST_LIMIT = 6`)
- **Thread Pool Size**: 4 threads

### Test Objectives

The script covers the following tests:

1. **Basic Connectivity**: Ensures the server is up and responding to a single request.
2. **Rate Limiting**: Sends a burst of requests to trigger the rate-limiting mechanism.
3. **Sustained Requests**: Sends requests spaced over time to check if the rate limit is resetting properly.
4. **Burst Test**: Sends a flood of requests to verify the server's behavior under load.

## Test Breakdown

1. **Basic Connectivity**:
    - A single request is sent to the server to check if it responds with a `200 OK` status code.
    
2. **Rate Limiting**:
    - 8 rapid requests are sent within a short time window. The server should respond to the first 6 and block the next 2 due to the rate limit.

3. **Sustained Requests**:
    - 12 requests are sent, each spaced by 1 second. This test checks if the server accepts requests consistently over time without hitting the rate limit.

4. **Burst Test**:
    - 10 requests are fired rapidly in quick succession. The server should block some of the requests based on the rate-limiting policy.

## Prerequisites

- Ensure that the server is running on `localhost` at port `8080` (or modify the script to match your configuration).
- The server should be compiled and started prior to running the tests.

## Usage
### Running the Test Suite

1. Navigate to the `test/` directory:
   ```bash
   cd test/

2. Make sure the test script has execute permission:
   ```bash
   chmod +x test.sh

3. Run the test script:
   ```bash
   ./test.sh
