#!/bin/bash
socat -u -b32768 UNIX-LISTEN:/tmp/unix.sock /dev/null &
sleep 1
socat -u -b32768 "SYSTEM:dd if=/dev/zero bs=1M count=10240" UNIX:/tmp/unix.sock
sleep 1 

socat -u -b32768 UNIX-LISTEN:/tmp/unix.sock /dev/null &
sleep 1
socat -u -b32768 "SYSTEM:dd if=/dev/zero bs=8192 count=1024000" UNIX:/tmp/unix.sock

