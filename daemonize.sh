#!/bin/bash
TIMEOUT=23
AUTOLOOP_PID=0

trap ctrl_c INT

function ctrl_c() {
    echo "[SHUTTING DOWN] "$SECONDS" sec - killing remaining instances..."
    kill $AUTOLOOP_PID && wait $AUTOLOOP_PID 2>/dev/null
    echo "[FINISHING] DONE"
    exit
}

echo "[STARTING] "$SECONDS" sec - starting capstone_server"
while true;
do
    echo "[REFRESHING] "$SECONDS" sec - restarted capstone_server"
    ./build/capstone_server &
    AUTOLOOP_PID=$!
    sleep $TIMEOUT
    kill $AUTOLOOP_PID && wait $AUTOLOOP_PID #2>/dev/null
done
