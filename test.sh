#!/bin/bash

URL="http://localhost:8080"

DATA="Hello Server"

for i in {1..15}
do
    curl -X POST $URL -d "$DATA"
done