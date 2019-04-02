#!/bin/bash

log=$1
while true; do
	top -H -b -n1 | grep validate11 | wc -l >> $log
	sleep 10
done
