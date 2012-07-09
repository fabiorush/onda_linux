#!/bin/sh

ARG=$1

(
  while true
  do
    true
  done
) > /dev/null 2>&1 < /dev/null &

PID=$!

export ARG PID

(
  sleep $ARG
  kill $PID
) > /dev/null 2>&1 < /dev/null &
