#!/bin/sh

ps -W | grep -ie \\\\explorer.exe$ -ie \\\\taskmgr.exe$ > /tmp/killtasks.txt

echo
cat /tmp/killtasks.txt
echo
echo "Okay to kill?  If not, ^C"
read answer

pids=$(cat /tmp/killtasks.txt | sed -n "s/^[^0-9]*\([0-9][0-9]*\).*$/\1/p")
for p in $pids
do
	echo taskkill //f //pid $p
	taskkill //f //pid $p
done

echo "start again?"
read i

/c/WINDOWS/explorer &
sleep 2
/c/WINDOWS/system32/taskmgr &
