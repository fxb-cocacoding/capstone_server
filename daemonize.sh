#!/bin/bash
TIMEOUT=180
AUTOLOOP_PID=0
IP=""
PORT=""

trap ctrl_c INT

function ctrl_c() {
    echo "[SHUTTING DOWN] "$SECONDS" sec - killing remaining instances..."
    killall capstone_server
    #kill $AUTOLOOP_PID && wait $AUTOLOOP_PID 2>/dev/null
    echo "[FINISHING] DONE"
    exit
}

function looping() {
    while true;
    do
        echo "[REFRESHING] "$SECONDS" sec - restarted capstone_server"
        echo "[ARGUMENTS] ip: "$IP" port: "$PORT" "
        ./build/capstone_server "$IP" "$PORT"
    done
}


if [[ $# -ne 2 ]]; then
  echo "Usage: daemonize.sh <ip> <port>"
  exit 1
fi
echo "[ARGUMENTS] ip: "$1" port: "$2" "
IP=$1
PORT=$2
echo "[STARTING] "$SECONDS" sec - starting capstone_server"

looping &

while true;
do
    sleep $TIMEOUT
    echo "[REFRESHING] "$SECONDS" sec - killed capstone_server"
    killall capstone_server
done

