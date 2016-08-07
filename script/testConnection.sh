#!/bin/bash

sudo ping 192.168.1.10 -c 1 >/dev/null 2>&1

echo $?
