#!/bin/sh

echo "Ok to kill nautilus? (Ctrl-C to abort)"
read input

pid=$(ps ax  | grep nautilus |
		grep -v grep |
		grep -v kill-nautilus.sh |
		awk '{ print $1 }')
if [ $pid ]; then
	kill $pid
fi

echo "and start again?"
read input

nautilus &
